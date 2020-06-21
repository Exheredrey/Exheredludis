#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

ENV["PALUDIS_HOME"] = Dir.getwd().to_s + "/environment_TEST_dir/home";

require 'test/unit'
require 'Paludis'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis
    class TestCase_Environment < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                x = Environment.new()
            end
        end
    end

    class TestCase_EnvironmentAcceptLicense < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def pid
            env[Selection::RequireExactlyOne.new(Generator::Matches.new(
                Paludis::parse_user_package_dep_spec('=foo/bar-1.0::testrepo', env, []), nil, []))].first
        end

        def test_accept_license
            assert env.accept_license('GPL-2', pid)
            assert !env.accept_license('Failure', pid)

            pid2 = env[Selection::RequireExactlyOne.new(Generator::Matches.new(
                Paludis::parse_user_package_dep_spec('=foo/baz-1.0::testrepo', env, []), nil, []))].first
            assert env.accept_license('GPL-2', pid2)
            assert env.accept_license('Failure', pid2)
        end

        def test_accept_license_bad
            assert_raise TypeError do
                env.accept_keywords('license','a string')
            end
        end
    end

    class TestCase_EnvironmentAcceptKeywords < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def pid
            env[Selection::RequireExactlyOne.new(Generator::Matches.new(
                Paludis::parse_user_package_dep_spec('=foo/bar-1.0::testrepo', env, []), nil, []))].first
        end

        def test_accept_keywords
            assert env.accept_keywords(['test'], pid)
            assert !env.accept_keywords(['test2'], pid)
            assert env.accept_keywords(['test','testtest'], pid)
            assert env.accept_keywords(['test2','testtest'], pid)
            assert !env.accept_keywords(['test2','test3'], pid)
        end

        def test_accept_keywords_bad
            assert_raise TypeError do
                env.accept_keywords('test',pid)
            end

            assert_raise TypeError do
                env.accept_keywords([],'a string')
            end
        end
    end

    class TestCase_EnvironmentPackageSet < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def test_package_set
            assert_kind_of DepSpec, env.set('everything')
        end

        def test_package_set_error
            assert_raise SetNameError do
                env.set('broken#')
            end
        end
    end

    class TestCase_EnvironmentDistribution < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def test_distribution
            assert_kind_of String, env.distribution
            assert_equal "gentoo", env.distribution
        end
    end

    class TestCase_EnvironmentMirrors < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def test_respond_and_return
            assert_respond_to env, :mirrors
            assert_kind_of Array, env.mirrors('')
        end


        def test_mirrors_star
            star_mirrors = env.mirrors('*')
            assert_equal 2, star_mirrors.length
            assert star_mirrors.include?('http://a')
            assert star_mirrors.include?('http://b')
        end

        def test_named_mirror
            assert_equal ['http://c'], env.mirrors('testmirror')
        end

        def test_empty_mirror
            assert env.mirrors('missingmirror').empty?
        end
    end

    class TestCase_EnvironmentQuery < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def pda
            Paludis::parse_user_package_dep_spec('=foo/bar-1.0', env, [])
        end

        def pda2
            Paludis::parse_user_package_dep_spec('foo/bar', env, [])
        end

        def test_arg_count
            assert_raise ArgumentError do
                env[1, 2];
            end
        end

        def test_environment_query
            a = env[Selection::AllVersionsSorted.new(Generator::Matches.new(pda, nil, []))]
            assert_kind_of Array, a
            assert_equal 1, a.length
            pid = a.first
            assert_kind_of PackageID, pid
            assert_equal 'foo/bar', pid.name
            assert_equal '1.0', pid.version.to_s
            assert_equal 'testrepo', pid.repository_name

            a = env[Selection::AllVersionsSorted.new(Generator::Matches.new(pda, nil, []) | Filter::SupportsAction.new(InstallAction))]
            assert_kind_of Array, a
            assert_equal 1, a.length
            pid = a.first
            assert_kind_of PackageID, pid
            assert_equal 'foo/bar', pid.name
            assert_equal '1.0', pid.version.to_s
            assert_equal 'testrepo', pid.repository_name

            a = env[Selection::AllVersionsSorted.new(Generator::Matches.new(pda, nil, []))]
            assert_kind_of Array, a
            assert_equal 1, a.length
            pid = a.first
            assert_kind_of PackageID, pid
            assert_equal 'foo/bar', pid.name
            assert_equal '1.0', pid.version.to_s
            assert_equal 'testrepo', pid.repository_name

            a = env[Selection::AllVersionsSorted.new(Generator::Matches.new(pda2, nil, []) | Filter::SupportsAction.new(InstallAction))]
            assert_kind_of Array, a
            assert_equal 2, a.length
            pid = a.shift
            assert_kind_of PackageID, pid
            assert_equal 'foo/bar', pid.name
            assert_equal '1.0', pid.version.to_s
            assert_equal 'testrepo', pid.repository_name
            pid2 = a.shift
            assert_kind_of PackageID, pid2
            assert_equal pid.name, pid2.name
            assert_equal '2.0', pid2.version.to_s
            assert_equal pid.repository_name, pid2.repository_name

            a = env[Selection::AllVersionsSorted.new(Generator::Package.new('foo/bar'))]
            assert_kind_of Array, a
            assert_equal 2, a.length
            pid = a.shift
            assert_kind_of PackageID, pid
            assert_equal 'foo/bar', pid.name
            assert_equal '1.0', pid.version.to_s
            assert_equal 'testrepo', pid.repository_name
            pid2 = a.shift
            assert_kind_of PackageID, pid2
            assert_equal pid.name, pid2.name
            assert_equal '2.0', pid2.version.to_s
            assert_equal pid.repository_name, pid2.repository_name


            a = env[Selection::AllVersionsUnsorted.new(Generator::Matches.new(Paludis::parse_user_package_dep_spec(
                '>=foo/bar-27', env, []), nil, []))]
            assert a.empty?

            a = env[Selection::AllVersionsUnsorted.new(Generator::Matches.new(pda2, nil, []) | Filter::SupportsAction.new(ConfigAction))]
            assert a.empty?
        end
    end

    class TestCase_EnvironmentMetadataKeys < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def test_format_key
            assert_respond_to env, :format_key
            assert_not_nil env.format_key
            assert_kind_of MetadataStringKey, env.format_key
            assert_equal 'paludis', env.format_key.parse_value
        end

        def test_config_location_key
            assert_respond_to env, :config_location_key
            assert_not_nil env.config_location_key
            assert_kind_of MetadataFSPathKey, env.config_location_key
            assert_equal Dir.getwd().to_s + "/environment_TEST_dir/home/.paludis", env.config_location_key.parse_value
        end
    end

    class TestCase_TestEnvironment < Test::Unit::TestCase
        def test_create
            x = TestEnvironment.new()
        end
    end

    class TestCase_EnvironmentFetchUniqueQualifiedPackageName < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def test_environment_fetch_unique_qualified_package_name
            assert_equal "foo/bar", env.fetch_unique_qualified_package_name("bar")
            assert_equal "foo/bar", env.fetch_unique_qualified_package_name("bar", Filter::SupportsAction.new(InstallAction))
        end

        def test_error
            assert_raise NoSuchPackageError do
                env.fetch_unique_qualified_package_name('foobarbaz')
            end
            assert_raise NoSuchPackageError do
                env.fetch_unique_qualified_package_name('bar', Filter::SupportsAction.new(ConfigAction))
            end
        end

        def test_bad
            assert_raise ArgumentError do
                env.fetch_unique_qualified_package_name
            end
            assert_raise ArgumentError do
                env.fetch_unique_qualified_package_name(1, 2, 3)
            end
            assert_raise TypeError do
                env.fetch_unique_qualified_package_name([])
            end
            assert_raise TypeError do
                env.fetch_unique_qualified_package_name('bar', 123)
            end
        end
    end

    class TestCase_EnvironmentRepositories < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def test_repositories
            assert_equal 1, env.repositories.length

            a = env.repositories.find_all do | repo |
                repo.name == "testrepo"
            end
            assert_equal 1, a.length

            a = env.repositories.find_all do | repo |
                repo.name == "foorepo"
            end
            assert a.empty?

            assert_equal nil, env.repositories {|repo| assert_kind_of Repository, repo}
        end

        def test_fetch_repository
            assert_equal "testrepo", env.fetch_repository("testrepo").name

            assert_raise Paludis::NoSuchRepositoryError do
                env.fetch_repository("barrepo")
            end
        end

        def test_has_repository_named?
            assert env.has_repository_named?('testrepo')
            assert ! env.has_repository_named?('foobarbaz')
        end
    end
end

