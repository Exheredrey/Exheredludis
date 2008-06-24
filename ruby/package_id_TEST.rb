#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2007, 2008 Richard Brown
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
#

require 'test/unit'
require 'Paludis'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis
    module TestStuff
        def env
            unless @env
                @env = NoConfigEnvironment.new("package_id_TEST_dir/testrepo/", '/var/empty')
            end
            @env
        end

        def env_vdb
            unless @env_vdb
                @env_vdb = NoConfigEnvironment.new("package_id_TEST_dir/installed/")
            end
            @env_vdb
        end

        def pid_testrepo
            env.package_database.fetch_repository("testrepo").package_ids("foo/bar").first
        end

        def pid_bad
            env.package_database.fetch_repository("testrepo").package_ids("bad/pkg").first
        end

        def pid_installed
            env_vdb.package_database.fetch_repository("installed").package_ids("cat-one/pkg-one").first
        end
    end

    class TestCase_MetadataKey < Test::Unit::TestCase
        include TestStuff

        def classes
            [MetadataStringKey, MetadataContentsKey, MetadataTimeKey, MetadataUseFlagNameSetKey, MetadataKeywordNameSetKey,
                MetadataIUseFlagSetKey, MetadataStringSetKey]
        end

        def all_classes
            classes.unshift MetadataKey
        end

        def mk
            pid_testrepo.short_description_key
        end

        def test_no_create
            all_classes.each do |x|
                assert_raise NoMethodError do
                    x.new
                end
            end
        end

        def test_raw_name
            assert_respond_to mk, :raw_name
            assert_kind_of String, mk.raw_name
            assert_equal 'DESCRIPTION', mk.raw_name
        end

        def test_human_name
            assert_respond_to mk, :human_name
            assert_kind_of String, mk.human_name
            assert_equal 'Description', mk.human_name
        end

        def test_type
            assert_respond_to mk, :type
            assert_kind_of Fixnum, mk.type
        end

        def test_value
            assert_respond_to mk, :value
            assert_kind_of String, mk.value
        end
    end


    class TestCase_PackageID < Test::Unit::TestCase
        include TestStuff
        def test_no_create
            assert_raise NoMethodError do
                p = PackageID.new
            end
        end

        def test_members
            { :name => QualifiedPackageName, :version => VersionSpec, :slot => String, :repository_name => String,
                :keywords_key => MetadataKeywordNameSetKey, :iuse_key => MetadataIUseFlagSetKey,
                :short_description_key => MetadataStringKey, :long_description_key => MetadataStringKey,
                :contents_key => MetadataContentsKey, :installed_time_key => MetadataTimeKey,
                :source_origin_key => MetadataStringKey, :binary_origin_key => MetadataStringKey,
                :masks => Array
            }.each_pair do | method, type |

                assert_respond_to pid_testrepo, method
                x = pid_testrepo.send(method)
                if x
                    assert_kind_of type, x
                else
                    assert_nil x
                end
            end
        end

        def test_canonical_form
            assert_equal 'foo/bar-1.0::testrepo', pid_testrepo.canonical_form(PackageIDCanonicalForm::Full)
            assert_equal '1.0', pid_testrepo.canonical_form(PackageIDCanonicalForm::Version)
            assert_equal 'foo/bar::testrepo', pid_testrepo.canonical_form(PackageIDCanonicalForm::NoVersion)
        end

        def test_stringify
            assert_equal 'foo/bar-1.0::testrepo', "#{pid_testrepo}"
        end

        def test_supports_action_test
            assert pid_testrepo.supports_action(SupportsActionTest.new(FetchAction))
            assert_raise TypeError do
                pid_testrepo.supports_action(1)
            end
            assert_raise TypeError do
                pid_testrepo.supports_action(pid_testrepo)
            end
        end

        def test_=
            assert_equal pid_testrepo, pid_testrepo
        end

        def test_subscript
            assert_equal pid_testrepo["DESCRIPTION"].value, "Test package"
            assert_nil pid_testrepo["PRESCRIPTION"]
        end

        def test_each_metadata
            keys = { "DESCRIPTION" => 1, "IUSE" => 1, "INHERITED" => 1, "KEYWORDS" => 1, "EAPI" => 1, "repository_mask" => 1,
                "profile_mask" => 1, "DEPEND" => 1, "RDEPEND" => 1, "PDEPEND" => 1, "LICENSE" => 1, "PROVIDE" => 1,
                "RESTRICT" => 1, "SRC_URI" => 1, "HOMEPAGE" => 1, "EBUILD" => 1 }
            pid_testrepo.each_metadata do | key |
                assert keys.has_key?(key.raw_name)
                keys.delete key.raw_name
            end
            assert keys.empty?
        end

        def test_masked?
            assert pid_testrepo.masked?
            assert !pid_installed.masked?
        end

        def test_breaks_portage
            assert pid_testrepo.breaks_portage.empty?
        end

        def test_masks
            masks = pid_testrepo.masks
            assert_equal 1, masks.length
            mask = masks.first
            assert_kind_of RepositoryMask, mask
            assert_equal "repository_mask", mask.mask_key.raw_name
        end

        def test_hash
            a = pid_testrepo
            b = pid_testrepo
            assert_equal a.hash, b.hash
        end

        def test_eql
            a = pid_testrepo
            b = pid_testrepo
            assert a.eql?(b)
        end
    end

    class TestCase_ERepo < Test::Unit::TestCase
        include TestStuff
        def test_name
            assert_kind_of QualifiedPackageName, pid_testrepo.name
            assert_equal QualifiedPackageName.new('foo/bar'), pid_testrepo.name
        end

        def test_version
            assert_kind_of VersionSpec, pid_testrepo.version
            assert_equal VersionSpec.new('1.0'), pid_testrepo.version
        end

        def test_repository_name
            assert_kind_of String, pid_testrepo.repository_name
            assert_equal 'testrepo', pid_testrepo.repository_name
        end

        def test_slot
            assert_kind_of String, pid_testrepo.slot
            assert_equal '0', pid_testrepo.slot
        end

        def test_short_description
            assert_kind_of MetadataStringKey, pid_testrepo.short_description_key
            assert_equal 'Test package', pid_testrepo.short_description_key.value
        end

        def test_long_description
            #only gems has this atm
            assert_nil pid_testrepo.long_description_key
        end

        def test_contents_key
            assert_nil pid_testrepo.contents_key
        end

        def test_installed_time_key
            assert_nil pid_testrepo.installed_time_key
        end

        def test_source_origin_key
            assert_nil pid_testrepo.source_origin_key
        end

        def test_binary_origin_key
            assert_nil pid_testrepo.binary_origin_key
        end

        def test_keywords_key
            assert_kind_of MetadataKeywordNameSetKey, pid_testrepo.keywords_key
            assert_kind_of Array, pid_testrepo.keywords_key.value
            assert_equal ['test'], pid_testrepo.keywords_key.value
        end

        def test_use_key
            assert_nil pid_testrepo["USE"]
        end

        def test_iuse_key
            assert_kind_of MetadataIUseFlagSetKey, pid_testrepo.iuse_key
            assert_kind_of Array, pid_testrepo.iuse_key.value
            assert_equal ['testflag'], pid_testrepo.iuse_key.value
        end

        def test_repository_mask_info_keys
            assert_kind_of MetadataRepositoryMaskInfoKey, pid_testrepo["repository_mask"]
            assert_kind_of RepositoryMaskInfo, pid_testrepo["repository_mask"].value
            assert_equal "package_id_TEST_dir/testrepo/profiles/package.mask", pid_testrepo["repository_mask"].value.mask_file
            assert_equal ["this is", "a test"], pid_testrepo["repository_mask"].value.comment
            assert_nil pid_testrepo["profile_mask"].value
        end

        def test_build_dependencies_key
            assert_kind_of MetadataDependencySpecTreeKey, pid_testrepo.build_dependencies_key
            assert_kind_of AllDepSpec, pid_testrepo.build_dependencies_key.value
        end

        def test_homepage_key
            assert_kind_of MetadataSimpleURISpecTreeKey, pid_testrepo.homepage_key
            assert_kind_of AllDepSpec, pid_testrepo.homepage_key.value
        end

        def test_fetches_key
            assert_kind_of MetadataFetchableURISpecTreeKey, pid_testrepo.fetches_key
            assert_kind_of AllDepSpec, pid_testrepo.fetches_key.value
            assert_respond_to pid_testrepo.fetches_key, :initial_label
            assert_kind_of URILabel, pid_testrepo.fetches_key.initial_label
            assert_kind_of URIMirrorsThenListedLabel, pid_testrepo.fetches_key.initial_label
            assert_equal "default", pid_testrepo.fetches_key.initial_label.text
        end
    end

    class TestCase_VDBRepo < Test::Unit::TestCase
        include TestStuff

        def test_name
            assert_kind_of QualifiedPackageName, pid_installed.name
            assert_equal QualifiedPackageName.new('cat-one/pkg-one'), pid_installed.name
        end

        def test_version
            assert_kind_of VersionSpec, pid_installed.version
            assert_equal VersionSpec.new('1'), pid_installed.version
        end

        def test_repository
            assert_kind_of String, pid_installed.repository_name
            assert_equal 'installed', pid_installed.repository_name
        end

        def test_slot
            assert_kind_of String, pid_installed.slot
            assert_equal 'test_slot', pid_installed.slot
        end

        def test_short_description
            assert_kind_of MetadataStringKey, pid_installed.short_description_key
            assert_equal 'a description', pid_installed.short_description_key.value
        end

        def test_long_description
            assert_nil pid_installed.long_description_key
        end

        def test_contents_key
            assert_kind_of MetadataContentsKey, pid_installed.contents_key
            assert_kind_of Contents, pid_installed.contents_key.value
        end

        def test_installed_time_key
            assert_kind_of MetadataTimeKey, pid_installed.installed_time_key
            assert_kind_of Time, pid_installed.installed_time_key.value
        end

        def test_source_origin_key
            assert_kind_of MetadataStringKey, pid_installed.source_origin_key
            assert_equal 'origin_test', pid_installed.source_origin_key.value
        end

        def test_binary_origin_key
            assert_nil pid_installed.binary_origin_key
        end

        def test_keywords_key
            assert_nil pid_installed.keywords_key
        end

        def test_use_key
            assert_kind_of MetadataUseFlagNameSetKey, pid_installed["USE"]
            assert_kind_of Array, pid_installed["USE"].value
            assert_equal ['test', 'test_use'], pid_installed["USE"].value
        end

        def test_iuse_key
            assert_kind_of MetadataIUseFlagSetKey, pid_installed.iuse_key
            assert_kind_of Array, pid_installed.iuse_key.value
            assert_equal ['test', 'test_iuse'], pid_installed.iuse_key.value
        end

        def test_inherited_key
            assert_kind_of MetadataStringSetKey, pid_installed["INHERITED"]
            assert_kind_of Array, pid_installed["INHERITED"].value
            assert_equal ['test_inherited'], pid_installed["INHERITED"].value
        end
    end

    class TestCase_BadKeys < Test::Unit::TestCase
        include TestStuff

        def test_keywords_key
            assert_raise NameError do
                pid_bad.keywords_key.value
            end
        end

        def test_iuse_key
            assert_raise NameError do
                pid_bad.iuse_key.value
            end
        end

        def test_build_dependenciess_key
            assert_raise RuntimeError do
                pid_bad.build_dependencies_key.value
            end
        end
    end
end


