/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/util/save.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct SaveTest : TestCase
    {
        SaveTest() : TestCase("save") { }

        void run()
        {
            std::string s("one");
            TEST_CHECK_EQUAL(s, "one");
            {
                Save<std::string> save_s(&s);
                TEST_CHECK_EQUAL(s, "one");
                s = "two";
                TEST_CHECK_EQUAL(s, "two");
            }
            TEST_CHECK_EQUAL(s, "one");
            {
                Save<std::string> save_s(&s, "three");
                TEST_CHECK_EQUAL(s, "three");
                {
                    Save<std::string> save_s_2(&s, "four");
                    TEST_CHECK_EQUAL(s, "four");
                }
                TEST_CHECK_EQUAL(s, "three");
            }
            TEST_CHECK_EQUAL(s, "one");
        }
    } test_save;

    struct RunOnDestructionTest : TestCase
    {
        RunOnDestructionTest() : TestCase("run on destruction") { }

        void run()
        {
            std::string s("one");
            TEST_CHECK_EQUAL(s, "one");
            {
                RunOnDestruction save_s(std::tr1::bind(&std::string::clear, &s));
                TEST_CHECK_EQUAL(s, "one");
            }
            TEST_CHECK_EQUAL(s, "");
        }
    } test_run_on_destruction;
}

