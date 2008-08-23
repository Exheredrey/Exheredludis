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

os.environ["PALUDIS_HOME"] = os.path.join(os.getcwd(), "mask_TEST_dir/home")

from paludis import *
from additional_tests import *

import unittest

Log.instance.log_level = LogLevel.WARNING

class TestCase_01_Masks(unittest.TestCase):
    def setUp(self):
        self.e = EnvironmentFactory.instance.create("")

    def test_01_user_mask(self):
        q = Selection.RequireExactlyOne(Generator.Matches(
            parse_user_package_dep_spec("=masked/user-1.0", self.e, [])))
        pid = iter(self.e[q]).next()
        m = iter(pid.masks).next()

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, UserMask))

        self.assertEquals(m.key(), "U")
        self.assertEquals(m.description(), "user")

    def test_02_unaccepted_mask(self):
        q = Selection.RequireExactlyOne(Generator.Matches(
            parse_user_package_dep_spec("=masked/unaccepted-1.0", self.e, [])))
        pid = iter(self.e[q]).next()
        m = iter(pid.masks).next()

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, UnacceptedMask))

        self.assertEquals(m.key(), "K")
        self.assertEquals(m.description(), "keyword")
        self.assert_(isinstance(m.unaccepted_key(), MetadataKeywordNameIterableKey))

    def test_03_repository_mask(self):
        q = Selection.RequireExactlyOne(Generator.Matches(
            parse_user_package_dep_spec("=masked/repo-1.0", self.e, [])))
        pid = iter(self.e[q]).next()
        m = iter(pid.masks).next()

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, RepositoryMask))

        self.assertEquals(m.key(), "R")
        self.assertEquals(m.description(), "repository")

        package_mask_path = os.path.realpath(os.path.join(os.getcwd(),
            "mask_TEST_dir/testrepo/profiles/package.mask"))
        self.assertEquals(m.mask_key().value().mask_file, package_mask_path)
        self.assert_(isinstance(m.mask_key().value().comment, StringIterable))

    def test_04_unsupported_mask(self):
        q = Selection.RequireExactlyOne(Generator.Matches(
            parse_user_package_dep_spec("=masked/unsupported-1.0", self.e, [])))
        pid = iter(self.e[q]).next()
        m = iter(pid.masks).next()

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, UnsupportedMask))

        self.assertEquals(m.key(), "E")
        self.assertEquals(m.description(), "eapi")
        self.assertEquals(m.explanation(), "Unsupported EAPI 'unsupported'")

    def test_05_association_mask(self):
        if os.environ.get("PALUDIS_ENABLE_VIRTUALS_REPOSITORY") == "yes":
            q = Selection.RequireExactlyOne(Generator.Matches(
                parse_user_package_dep_spec("=virtual/association-1.0", self.e, [])))
            pid = iter(self.e[q]).next()
            m = iter(pid.masks).next()

            self.assert_(isinstance(m, Mask))
            self.assert_(isinstance(m, AssociationMask))

            self.assertEquals(m.key(), "A")
            self.assertEquals(m.description(), "by association")
            self.assertEquals(m.associated_package().name, "masked/repo")
        elif os.environ.get("PALUDIS_ENABLE_VIRTUALS_REPOSITORY") != "no":
            raise "oops"

class TestCase_02_Masks_subclassing(unittest.TestCase):
    def test_01_user_mask(self):
        class TestUserMask(UserMask):
            def key(self):
                return "T"

            def description(self):
                return "test"

        test_user_mask(TestUserMask())

    def test_02_unaccepted_mask(self):
        class TestUnacceptedMask(UnacceptedMask):
            def key(self):
                return "T"

            def description(self):
                return "test"

            def unaccepted_key(self):
                return MetadataStringKey("raw", "human", MetadataKeyType.NORMAL)

        test_unaccepted_mask(TestUnacceptedMask())

    def test_03_repository_mask(self):
        class TestRepositoryMask(RepositoryMask):
            def key(self):
                return "T"

            def description(self):
                return "test"

            def mask_key(self):
                return MetadataStringKey("raw", "human", MetadataKeyType.NORMAL)

        test_repository_mask(TestRepositoryMask())

    def test_04_unsupported_mask(self):
        class TestUnsupportedMask(UnsupportedMask):
            def key(self):
                return "T"

            def description(self):
                return "test"

            def explanation(self):
                return "test"

        test_unsupported_mask(TestUnsupportedMask())

    def test_05_association_mask(self):
        class TestAssociationMask(AssociationMask):
            def key(self):
                return "T"

            def description(self):
                return "test"

            def associated_package(self):
                e = EnvironmentFactory.instance.create("")
                q = Selection.RequireExactlyOne(Generator.Matches(
                    parse_user_package_dep_spec("=masked/user-1.0", e, [])))
                pid = iter(e[q]).next()
                return pid

        test_association_mask(TestAssociationMask())


if __name__ == "__main__":
    unittest.main()
