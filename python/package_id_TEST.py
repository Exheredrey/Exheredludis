#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

#
# Copyright (c) 2007 Piotr Jaroszyński
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

import os

repo_path = os.path.join(os.getcwd(), "package_id_TEST_dir/testrepo")
irepo_path = os.path.join(os.getcwd(), "package_id_TEST_dir/installed")

from paludis import *
import unittest

Log.instance.log_level = LogLevel.WARNING

class TestCase_01_PackageID(unittest.TestCase):
    def setUp(self):
        self.e = NoConfigEnvironment(repo_path, "/var/empty")
        self.ie = NoConfigEnvironment(irepo_path)
        self.pid = iter(self.e.package_database.fetch_repository("testrepo").package_ids("foo/bar")).next()
        if os.environ.get("PALUDIS_ENABLE_VIRTUALS_REPOSITORY") == "yes":
            self.vpid = iter(self.e.package_database.fetch_repository("virtuals").package_ids("virtual/bar")).next()
        self.ipid = iter(self.ie.package_database.fetch_repository("installed").package_ids("cat-one/pkg-one")).next()
        self.mpid = iter(self.e.package_database.fetch_repository("testrepo").package_ids("cat/masked")).next()

    def test_01_get(self):
        pass

    def test_02_name(self):
        self.assertEquals(self.pid.name, QualifiedPackageName("foo/bar"))
        self.assertEquals(self.ipid.name, QualifiedPackageName("cat-one/pkg-one"))

    def test_03_version(self):
        self.assertEquals(self.pid.version, VersionSpec("1.0"))
        self.assertEquals(self.ipid.version, VersionSpec("1"))

    def test_04_slot(self):
        self.assertEquals(str(self.ipid.slot_key().value()), "test_slot")
        self.assertEquals(str(self.pid.slot_key().value()), "0")

    def test_05_repository(self):
        self.assertEquals(str(self.pid.repository.name), "testrepo")
        self.assertEquals(str(self.ipid.repository.name), "installed")

    def test_07_canonical_form(self):
        # Load the metadata
        self.pid.slot_key().value
        self.ipid.slot_key().value

        self.assertEquals(self.pid.canonical_form(PackageIDCanonicalForm.FULL), "foo/bar-1.0:0::testrepo")
        self.assertEquals(self.pid.canonical_form(PackageIDCanonicalForm.VERSION), "1.0")
        self.assertEquals(self.pid.canonical_form(PackageIDCanonicalForm.NO_VERSION), "foo/bar:0::testrepo")

        self.assertEquals(self.ipid.canonical_form(PackageIDCanonicalForm.FULL),
                "cat-one/pkg-one-1:test_slot::installed")
        self.assertEquals(self.ipid.canonical_form(PackageIDCanonicalForm.VERSION), "1")
        self.assertEquals(self.ipid.canonical_form(PackageIDCanonicalForm.NO_VERSION),
                "cat-one/pkg-one:test_slot::installed")

    def test_08_str(self):
        # Load the metadata
        self.pid.slot_key().value
        self.ipid.slot_key().value

        self.assertEquals(str(self.pid), "foo/bar-1.0:0::testrepo")
        self.assertEquals(str(self.ipid), "cat-one/pkg-one-1:test_slot::installed")

    def test_09_find_metadata(self):
        self.assert_(isinstance(self.pid.find_metadata("DEPEND"), MetadataDependencySpecTreeKey))

    def test_10_perform_action(self):
        self.pid.perform_action(PretendAction())
        self.assertRaises(UnsupportedActionError, self.pid.perform_action, ConfigAction())
        self.assertRaises(UnsupportedActionError, self.ipid.perform_action, PretendAction())

    def test_11_supports_action(self):
        self.assert_(self.pid.supports_action(SupportsFetchActionTest()))
        self.assert_(self.pid.supports_action(SupportsInstallActionTest()))
        self.assert_(self.pid.supports_action(SupportsFetchActionTest()))
        self.assert_(not self.pid.supports_action(SupportsUninstallActionTest()))
        self.assert_(not self.pid.supports_action(SupportsInstalledActionTest()))
        self.assert_(self.pid.supports_action(SupportsPretendActionTest()))
        self.assert_(not self.pid.supports_action(SupportsConfigActionTest()))

        self.assert_(not self.ipid.supports_action(SupportsFetchActionTest()))
        self.assert_(not self.ipid.supports_action(SupportsInstallActionTest()))
        self.assert_(not self.ipid.supports_action(SupportsFetchActionTest()))
        self.assert_(self.ipid.supports_action(SupportsUninstallActionTest()))
        self.assert_(self.ipid.supports_action(SupportsInstalledActionTest()))
        self.assert_(not self.ipid.supports_action(SupportsPretendActionTest()))
        self.assert_(self.ipid.supports_action(SupportsConfigActionTest()))

    def test_12_masked(self):
        self.assert_(not self.pid.masked)
        self.assert_(self.mpid.masked)

    def test_13_masks(self):
        mask = iter(self.mpid.masks).next()
        self.assert_(isinstance(mask, UnacceptedMask))

    def test_14_virtual_for_key(self):
        if os.environ.get("PALUDIS_ENABLE_VIRTUALS_REPOSITORY") == "yes":
            self.assertEquals(self.vpid.virtual_for_key().value(), self.pid)
            self.assertEquals(self.pid.virtual_for_key(), None)
            self.assertEquals(self.ipid.virtual_for_key(), None)

    def test_15_provide_key(self):
        self.assert_(isinstance(self.pid.provide_key(), MetadataProvideSpecTreeKey))
        self.assertEquals(self.ipid.provide_key(), None)

    def test_16_contains_key(self):
        self.assertEquals(self.pid.contains_key(), None)
        self.assertEquals(self.ipid.contains_key(), None)

    def test_17_contained_in_key(self):
        self.assertEquals(self.pid.contained_in_key(), None)
        self.assertEquals(self.ipid.contained_in_key(), None)

    def test_18_build_dependencies_key(self):
        self.assert_(isinstance(self.pid.build_dependencies_key(), MetadataDependencySpecTreeKey))
        self.assertEquals(self.ipid.build_dependencies_key(), None)

    def test_19_run_dependencies_key(self):
        self.assert_(isinstance(self.pid.run_dependencies_key(), MetadataDependencySpecTreeKey))
        self.assertEquals(self.ipid.run_dependencies_key(), None)

    def test_20_post_dependencies_key(self):
        self.assert_(isinstance(self.pid.post_dependencies_key(), MetadataDependencySpecTreeKey))
        self.assertEquals(self.ipid.post_dependencies_key(), None)

    def test_21_suggested_dependencies_key(self):
        self.assertEquals(self.pid.suggested_dependencies_key(), None)
        self.assertEquals(self.ipid.suggested_dependencies_key(), None)

    def test_22_fetches_key(self):
        self.assert_(isinstance(self.pid.fetches_key(), MetadataFetchableURISpecTreeKey))
        self.assertEquals(self.ipid.fetches_key(), None)

    def test_23_homepage_key(self):
        self.assert_(isinstance(self.pid.homepage_key(), MetadataSimpleURISpecTreeKey))
        self.assertEquals(self.ipid.homepage_key(), None)

    def test_24_short_description_key(self):
        self.assertEquals(self.pid.short_description_key().value(), "Test package")
        self.assertEquals(self.ipid.short_description_key().value(), "a description")

    def test_25_long_description_key(self):
        self.assertEquals(self.pid.long_description_key(), None)
        self.assertEquals(self.ipid.long_description_key(), None)

    def test_26_contents_key(self):
        self.assertEquals(self.pid.contents_key(), None)
        self.assert_(isinstance(self.ipid.contents_key(), MetadataContentsKey))

    def test_27_installed_time_key(self):
        self.assertEquals(self.pid.installed_time_key(), None)
        self.assert_(isinstance(self.ipid.installed_time_key(), MetadataTimeKey))

    def test_28_from_repositories_key(self):
        self.assertEquals(self.pid.from_repositories_key(), None)
        self.assertEquals(iter(self.ipid.from_repositories_key().value()).next(), "origin_test")

    def test_30_fs_location_key(self):
        self.assert_(isinstance(self.ipid.fs_location_key(), MetadataFSEntryKey))
        self.assert_(isinstance(self.ipid.fs_location_key(), MetadataFSEntryKey))


if __name__ == "__main__":
    unittest.main()
