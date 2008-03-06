/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <paludis/stripper.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/kc.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>

using namespace test;
using namespace paludis;

namespace
{
    struct TestStripper :
        Stripper
    {
        virtual void on_enter_dir(const FSEntry &)
        {
        }

        virtual void on_leave_dir(const FSEntry &)
        {
        }


        virtual void on_strip(const FSEntry &)
        {
        }

        virtual void on_split(const FSEntry &, const FSEntry &)
        {
        }

        virtual void on_unknown(const FSEntry &)
        {
        }

        TestStripper(const StripperOptions & o) :
            Stripper(o)
        {
        }
    };
}

namespace test_cases
{
    struct StripperTest : TestCase
    {
        StripperTest() : TestCase("stripper") { }

        void run()
        {
            TestStripper s(StripperOptions::named_create()
                    (k::image_dir(), FSEntry("stripper_TEST_dir/image").realpath())
                    (k::debug_dir(), FSEntry("stripper_TEST_dir/image").realpath() / "usr" / "lib" / "debug")
                    (k::debug_build(), iado_split)
                    );
            s.strip();

            TEST_CHECK(FSEntry("stripper_TEST_dir/image/usr/lib/debug/usr/bin/stripper_TEST_binary.debug").is_regular_file());
        }

        bool repeatable() const
        {
            return false;
        }
    } test_stripper;
}

