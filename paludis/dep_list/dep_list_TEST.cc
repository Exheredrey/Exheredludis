/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "dep_list_TEST.hh"
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/stringify_formatter-impl.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/dep_list/override_functions.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    /**
     * Convenience sub base class used by the numbered DepList tests.
     *
     */
    template <int i_>
    struct DepListTestCase : DepListTestCaseBase
    {
        /**
         * Constructor.
         */
        DepListTestCase() :
            DepListTestCaseBase("dep list " + stringify(i_))
        {
        }
    };

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase1 : DepListTestCase<1>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_1;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase2 : DepListTestCase<2>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_2;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase3 : DepListTestCase<3>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("cat/three");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_3;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase4 : DepListTestCase<4>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two cat/three");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_4;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase5 : DepListTestCase<5>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two cat/three");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("cat/three");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_5;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase6 : DepListTestCase<6>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two cat/three");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1")->build_dependencies_key()->set_from_string("cat/two");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_6;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase7 : DepListTestCase<7>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two cat/three");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("cat/four");
            repo->add_version("cat", "three", "1")->build_dependencies_key()->set_from_string("cat/four");
            repo->add_version("cat", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/four-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_7;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase8 : DepListTestCase<8>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two cat/three");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("cat/four cat/three");
            repo->add_version("cat", "three", "1")->build_dependencies_key()->set_from_string("cat/four");
            repo->add_version("cat", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/four-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_8;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase9 : DepListTestCase<9>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two cat/three");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("cat/four");
            repo->add_version("cat", "three", "1")->build_dependencies_key()->set_from_string("cat/four cat/two");
            repo->add_version("cat", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/four-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_9;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase10 : DepListTestCase<10>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_10;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase11 : DepListTestCase<11>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two cat/three");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1")->build_dependencies_key()->set_from_string("|| ( cat/two cat/four )");
            repo->add_version("cat", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_11;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase12 : DepListTestCase<12>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( ( cat/two cat/three ) cat/four )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
            repo->add_version("cat", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_12;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase13 : DepListTestCase<13>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two cat/three cat/four");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
            repo->add_version("cat", "four", "1")->build_dependencies_key()->set_from_string("|| ( ( cat/two cat/three ) cat/five )");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/four-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_13;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase14 : DepListTestCase<14>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( cat/two cat/three )");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_14;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase15 : DepListTestCase<15>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("cat/four");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_15;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase16 : DepListTestCase<16>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two:slot2");
            repo->add_version("cat", "two", "1.1")->set_slot(SlotName("slot1"));
            repo->add_version("cat", "two", "1.2")->set_slot(SlotName("slot2"));
            repo->add_version("cat", "two", "1.3")->set_slot(SlotName("slot3"));
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1.2:slot2::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_16;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase17 : DepListTestCase<17>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("<cat/two-1.2-r2:slot2");
            repo->add_version("cat", "two", "1.1")->set_slot(SlotName("slot1"));
            repo->add_version("cat", "two", "1.2")->set_slot(SlotName("slot2"));
            repo->add_version("cat", "two", "1.2-r1")->set_slot(SlotName("slot2"));
            repo->add_version("cat", "two", "1.2-r2")->set_slot(SlotName("slot2"));
            repo->add_version("cat", "two", "1.3")->set_slot(SlotName("slot3"));
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1.2-r1:slot2::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_17;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase18 : DepListTestCase<18>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( )");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_18;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase19 : DepListTestCase<19>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("enabled? ( cat/two )");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_19;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase20 : DepListTestCase<20>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("!enabled? ( cat/two )");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_20;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase21 : DepListTestCase<21>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("disabled? ( cat/two )");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_21;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase22 : DepListTestCase<22>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("!disabled? ( cat/two )");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_22;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase23 : DepListTestCase<23>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( enabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_23;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase24 : DepListTestCase<24>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( !enabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_24;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase25 : DepListTestCase<25>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( disabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_25;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase26 : DepListTestCase<26>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( !disabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_26;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase27 : DepListTestCase<27>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/three || ( enabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_27;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase28 : DepListTestCase<28>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/three || ( !enabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_28;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase29 : DepListTestCase<29>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/three || ( disabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_29;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase30 : DepListTestCase<30>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/three || ( !disabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_30;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase31 : DepListTestCase<31>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/three || ( enabled? ( cat/three ) cat/two )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_31;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase32 : DepListTestCase<32>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/three || ( !enabled? ( cat/three ) cat/two )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_32;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase33 : DepListTestCase<33>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/three || ( disabled? ( cat/three ) cat/two )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_33;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase34 : DepListTestCase<34>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/three || ( !disabled? ( cat/three ) cat/two )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_34;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase35 : DepListTestCase<35>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( enabled1? ( cat/two ) enabled2? ( cat/three ) )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_35;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase36 : DepListTestCase<36>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( !enabled1? ( cat/two ) enabled2? ( cat/three ) )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_36;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase37 : DepListTestCase<37>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( !enabled1? ( cat/two ) !enabled2? ( cat/three ) )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_37;

    struct DepListTestCase38 : DepListTestCase<38>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->keywords_key()->set_from_string("test");
            repo->add_version("cat", "one", "2")->keywords_key()->set_from_string("~test");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_38;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase42 : DepListTestCase<42>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("cat/one");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_42;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase43 : DepListTestCase<43>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1")->post_dependencies_key()->set_from_string("cat/one");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_43;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase44 : DepListTestCase<44>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("( cat/two cat/two )");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_44;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase45 : DepListTestCase<45>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("( cat/two[enabled] )");
            repo->add_version("cat", "two", "1")->iuse_key()->set_from_string("ebuild", iuse_pm_permissive);
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_45;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase46 : DepListTestCase<46>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("( cat/two[-disabled] )");
            repo->add_version("cat", "two", "1")->iuse_key()->set_from_string("ebuild", iuse_pm_permissive);
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_46;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase47 : DepListTestCase<47>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("( cat/two[disabled] )");
            repo->add_version("cat", "two", "1")->iuse_key()->set_from_string("ebuild", iuse_pm_permissive);
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            TEST_CHECK_THROWS(d.add(PackageDepSpec(merge_target, pds_pm_permissive),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_47;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase48 : DepListTestCase<48>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("( cat/two[-enabled] )");
            repo->add_version("cat", "two", "1")->iuse_key()->set_from_string("ebuild", iuse_pm_permissive);
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            TEST_CHECK_THROWS(d.add(PackageDepSpec(merge_target, pds_pm_permissive),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_48;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase49 : DepListTestCase<49>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("( cat/two cat/two[enabled] )");
            repo->add_version("cat", "two", "1")->iuse_key()->set_from_string("ebuild", iuse_pm_permissive);
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_49;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase50 : DepListTestCase<50>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("( cat/two cat/two[-disabled] )");
            repo->add_version("cat", "two", "1")->iuse_key()->set_from_string("ebuild", iuse_pm_permissive);
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_50;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase51 : DepListTestCase<51>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("( cat/two cat/two[disabled] )");
            repo->add_version("cat", "two", "1")->iuse_key()->set_from_string("ebuild", iuse_pm_permissive);
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            TEST_CHECK_THROWS(d.add(PackageDepSpec(merge_target, pds_pm_permissive),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_51;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase52 : DepListTestCase<52>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("( cat/two cat/two[-enabled] )");
            repo->add_version("cat", "two", "1")->iuse_key()->set_from_string("ebuild", iuse_pm_permissive);
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            TEST_CHECK_THROWS(d.add(PackageDepSpec(merge_target, pds_pm_permissive),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_52;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase53 : DepListTestCase<53>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->post_dependencies_key()->set_from_string("cat/two");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
        }
    } test_dep_list_53;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase54 : DepListTestCase<54>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->post_dependencies_key()->set_from_string("cat/two");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("cat/three");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
        }
    } test_dep_list_54;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase55 : DepListTestCase<55>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->post_dependencies_key()->set_from_string("cat/two");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("cat/one");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
        }
    } test_dep_list_55;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase56 : DepListTestCase<56>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two || ( cat/three virtual/four )");
            repo->add_version("cat", "two", "1")->provide_key()->set_from_string("virtual/four");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("virtual/four-1::virtuals (virtual for cat/two-1:0::repo)");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_56;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase57 : DepListTestCase<57>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->post_dependencies_key()->set_from_string("cat/two");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("cat/three");
            repo->add_version("cat", "three", "1")->build_dependencies_key()->set_from_string("cat/one");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
        }
    } test_dep_list_57;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase58 : DepListTestCase<58>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->provide_key()->set_from_string("virtual/two");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("virtual/two-1::virtuals (virtual for cat/one-1:0::repo)");
        }
    } test_dep_list_58;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase59 : DepListTestCase<59>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( cat/two >=cat/three-2 )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "2");
            installed_repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-2:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_59;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase60 : DepListTestCase<60>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( cat/two >=cat/three-3 )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "2");
            installed_repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_60;

    struct DepListTestCase61 : DepListTestCase<61>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->provide_key()->set_from_string("virtual/foo");
            tr1::shared_ptr<FakePackageID> m(repo->add_version("cat", "two", "2"));
            m->provide_key()->set_from_string("virtual/foo");
            m->build_dependencies_key()->set_from_string("cat/one");
        }

        void populate_expected()
        {
            merge_target = "cat/two";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("virtual/foo-1::virtuals (virtual for cat/one-1:0::repo)");
            expected.push_back("cat/two-2:0::repo");
            expected.push_back("virtual/foo-2::virtuals (virtual for cat/two-2:0::repo)");
        }
    } test_dep_list_61;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase62 : DepListTestCase<62>
    {
        void set_options(DepListOptions & opts)
        {
            using namespace tr1::placeholders;
            opts.override_masks.reset(new DepListOverrideMasksFunctions);
            opts.override_masks->push_back(tr1::bind(&override_tilde_keywords, &env, _1, _2));
        }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1")->keywords_key()->set_from_string("~test");
            repo->add_version("cat", "three", "2")->keywords_key()->set_from_string("~test");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo(M)");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_62;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase63 : DepListTestCase<63>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( =cat/two-1 =cat/two-2 )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "two", "2");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-2:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_63;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase64 : DepListTestCase<64>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( =cat/two-1 =cat/two-2 )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "two", "2");
            installed_repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-2:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_64;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase65 : DepListTestCase<65>
    {
        void set_options(DepListOptions & opts)
        {
            using namespace tr1::placeholders;
            opts.override_masks.reset(new DepListOverrideMasksFunctions);
            opts.override_masks->push_back(tr1::bind(&override_tilde_keywords, &env, _1, _2));
        }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("|| ( =cat/two-2 =cat/two-1 )");
            repo->add_version("cat", "two", "1")->keywords_key()->set_from_string("~test");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo(M)");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_65;

    /**
     * \test Test DepList transactional add behaviour.
     *
     */
    struct DepListTestCaseTransactionalAdd : TestCase
    {
        DepListTestCaseTransactionalAdd() : TestCase("dep list transactional add") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            tr1::shared_ptr<FakeInstalledRepository> destination_repo(new FakeInstalledRepository(&env,
                        RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, destination_repo);

            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two cat/three");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("cat/four");
            repo->add_version("cat", "three", "1")->build_dependencies_key()->set_from_string("cat/four cat/two");
            repo->add_version("cat", "four", "1");
            repo->add_version("cat", "five", "1")->build_dependencies_key()->set_from_string("cat/six cat/seven");
            repo->add_version("cat", "six", "1");
            repo->add_version("cat", "seven", "1")->build_dependencies_key()->set_from_string("cat/doesnotexist");

            DepList d(&env, DepListOptions());
            d.add(PackageDepSpec("cat/one", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "cat/four-1:0::repo cat/two-1:0::repo cat/three-1:0::repo cat/one-1:0::repo");

            TEST_CHECK_THROWS(d.add(PackageDepSpec("cat/five", pds_pm_permissive),
                        env.default_destinations()), DepListError);

            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "cat/four-1:0::repo cat/two-1:0::repo cat/three-1:0::repo cat/one-1:0::repo");
        }
    } test_dep_list_transactional_add;

    /**
     * \test Test DepList transactional add behaviour on PDEPENDs.
     *
     */
    struct DepListTestCaseTransactionalAddPost : TestCase
    {
        DepListTestCaseTransactionalAddPost() : TestCase("dep list transactional add post") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            tr1::shared_ptr<FakeInstalledRepository> destination_repo(new FakeInstalledRepository(&env,
                        RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, destination_repo);

            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two cat/three");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("cat/four");
            repo->add_version("cat", "three", "1")->build_dependencies_key()->set_from_string("cat/four cat/two");
            repo->add_version("cat", "four", "1");
            repo->add_version("cat", "five", "1")->build_dependencies_key()->set_from_string("cat/six cat/seven");
            repo->add_version("cat", "six", "1");
            repo->add_version("cat", "seven", "1")->post_dependencies_key()->set_from_string("cat/doesnotexist");

            DepList d(&env, DepListOptions());
            d.add(PackageDepSpec("cat/one", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "cat/four-1:0::repo cat/two-1:0::repo cat/three-1:0::repo cat/one-1:0::repo");

            TEST_CHECK_THROWS(d.add(PackageDepSpec("cat/five", pds_pm_permissive),
                        env.default_destinations()), DepListError);

            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "cat/four-1:0::repo cat/two-1:0::repo cat/three-1:0::repo cat/one-1:0::repo");
        }
    } test_dep_list_transactional_add_post;

    /**
     * \test Test DepList transactional forced downgrade of installed package behaviour.
     *
     */
    struct DepListTestCaseForcedDowngradeOfInstalled : TestCase
    {
        DepListTestCaseForcedDowngradeOfInstalled() : TestCase("dep list forced downgrade of installed") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "one", "1");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "one", "2");

            DepList d(&env, DepListOptions());
            d.add(PackageDepSpec("cat/one", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "), "cat/one-1:0::repo");
        }
    } test_dep_list_forced_downgrade_of_installed;

    /**
     * \test Test DepList fall back never.
     */
    struct DepListTestCaseFallBackNever : TestCase
    {
        DepListTestCaseFallBackNever() : TestCase("dep list fall back never") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "two", "2");

            DepList d(&env, DepListOptions());
            d.options()->fall_back = dl_fall_back_never;
            TEST_CHECK_THROWS(d.add(PackageDepSpec("cat/one", pds_pm_permissive),
                        env.default_destinations()), DepListError);
        }
    } test_dep_list_fall_back_never;

    /**
     * \test Test DepList fall back as needed.
     */
    struct DepListTestCaseFallBackAsNeeded : TestCase
    {
        DepListTestCaseFallBackAsNeeded() : TestCase("dep list fall back as needed") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "two", "2");

            DepList d(&env, DepListOptions());
            d.options()->fall_back = dl_fall_back_as_needed;
            d.add(PackageDepSpec("cat/one", pds_pm_permissive), env.default_destinations());
            d.add(PackageDepSpec("cat/two", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "), "cat/two-2:0::installed_repo cat/one-1:0::repo");
        }
    } test_dep_list_fall_back_as_needed;

    /**
     * \test Test DepList fall back as needed.
     */
    struct DepListTestCaseFallBackAsNeededNotTargets : TestCase
    {
        DepListTestCaseFallBackAsNeededNotTargets() : TestCase("dep list fall back as needed not targets") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "two", "2");
            installed_repo->add_version("cat", "three", "3");

            DepList d1(&env, DepListOptions());
            d1.options()->fall_back = dl_fall_back_as_needed_except_targets;
            d1.add(PackageDepSpec("cat/one", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d1.begin(), d1.end(), " "), "cat/two-2:0::installed_repo cat/one-1:0::repo");
            TEST_CHECK_THROWS(d1.add(PackageDepSpec("cat/three", pds_pm_permissive),
                        env.default_destinations()), DepListError);

            DepList d2(&env, DepListOptions());
            d2.options()->fall_back = dl_fall_back_as_needed_except_targets;
            TEST_CHECK_THROWS(d2.add(PackageDepSpec("cat/two", pds_pm_permissive),
                        env.default_destinations()), DepListError);

            DepList d3(&env, DepListOptions());
            d3.options()->fall_back = dl_fall_back_as_needed_except_targets;
            tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > t3(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                        tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
            t3->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("cat/one", pds_pm_permissive)))));
            t3->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("cat/two", pds_pm_permissive)))));
            TEST_CHECK_THROWS(d3.add(*t3, env.default_destinations()), DepListError);

            DepList d4(&env, DepListOptions());
            tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > t4(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                    tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
            t4->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("cat/one", pds_pm_permissive)))));
            t4->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("cat/three", pds_pm_permissive)))));
            TEST_CHECK_THROWS(d4.add(*t4, env.default_destinations()), DepListError);
        }
    } test_dep_list_fall_back_as_needed_not_targets;

    /**
     * \test Test DepList upgrade as needed.
     */
    struct DepListTestCaseUpgradeAsNeeded : TestCase
    {
        DepListTestCaseUpgradeAsNeeded() : TestCase("dep list upgrade as needed") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");
            repo->add_version("cat", "two", "2");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "two", "0");

            DepList d1(&env, DepListOptions());
            d1.options()->upgrade = dl_upgrade_as_needed;
            d1.add(PackageDepSpec("cat/one", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d1.begin(), d1.end(), " "), "cat/two-0:0::installed_repo cat/one-1:0::repo");

            DepList d2(&env, DepListOptions());
            d2.options()->upgrade = dl_upgrade_as_needed;

            tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > t2(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                        tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
            t2->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("cat/one", pds_pm_permissive)))));
            t2->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("cat/two", pds_pm_permissive)))));
            d2.add(*t2, env.default_destinations());
            TEST_CHECK_EQUAL(join(d2.begin(), d2.end(), " "), "cat/two-2:0::repo cat/one-1:0::repo");
        }
    } test_dep_list_upgrade_as_needed;

    /**
     * \test Test DepList reinstall scm.
     */
    struct DepListTestCaseReinstallSCM : TestCase
    {
        DepListTestCaseReinstallSCM() : TestCase("dep list reinstall scm") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "zero", "1")->build_dependencies_key()->set_from_string(
                "( cat/one cat/two cat/three-live cat/four-cvs cat/five-svn cat/six-darcs )");
            repo->add_version("cat", "one", "scm");
            repo->add_version("cat", "two", "2");
            repo->add_version("cat", "three-live", "0");
            repo->add_version("cat", "four-cvs", "0");
            repo->add_version("cat", "five-svn", "0");
            repo->add_version("cat", "six-darcs", "0");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "one", "scm");
            installed_repo->add_version("cat", "two", "2");
            installed_repo->add_version("cat", "three-live", "0");
            installed_repo->add_version("cat", "four-cvs", "0");
            installed_repo->add_version("cat", "five-svn", "0");
            installed_repo->add_version("cat", "six-darcs", "0");

            DepList d1(&env, DepListOptions());
            d1.options()->reinstall_scm = dl_reinstall_scm_always;
            d1.add(PackageDepSpec("cat/zero", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d1.begin(), d1.end(), " "), "cat/one-scm:0::repo cat/two-2:0::installed_repo "
                    "cat/three-live-0:0::repo cat/four-cvs-0:0::repo cat/five-svn-0:0::repo cat/six-darcs-0:0::repo "
                    "cat/zero-1:0::repo");
        }
    } test_dep_list_upgrade_reinstall_scm;

    /**
     * \test Test DepList dependency tags.
     */
    struct DepListTestCaseDependencyTags : TestCase
    {
        DepListTestCaseDependencyTags() : TestCase("dep list dependency tags") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/three");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("enabled? ( || ( ( <cat/three-1 cat/three:0 =cat/four-1 ) cat/five ) )");
            repo->add_version("cat", "three", "0.9");
            repo->add_version("cat", "four", "1");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);

            DepList d1(&env, DepListOptions());
            d1.options()->dependency_tags = true;
            PackageDepSpec with_target_tag("cat/one", pds_pm_permissive);
            with_target_tag.set_tag(tr1::shared_ptr<const DepTag>(new TargetDepTag));
            d1.add(with_target_tag, env.default_destinations());
            PackageDepSpec with_set_tag("cat/two", pds_pm_permissive);
            with_set_tag.set_tag(tr1::shared_ptr<const DepTag>(new GeneralSetDepTag(SetName("set"), "test")));
            d1.add(with_set_tag, env.default_destinations());

            TEST_CHECK_EQUAL(join(d1.begin(), d1.end(), " "), "cat/three-0.9:0::repo cat/one-1:0::repo "
                    "cat/four-1:0::repo cat/two-1:0::repo");

            // tags for cat/three
            DepList::Iterator it(d1.begin());
            tr1::shared_ptr<DepListEntryTags> tags(it->tags);
            TEST_CHECK_EQUAL(tags->size(), 3U);
            bool cat_three_has_tag_from_cat_one(false);
            bool cat_three_has_first_tag_from_cat_two(false);
            bool cat_three_has_second_tag_from_cat_two(false);

            for (DepListEntryTags::ConstIterator tag_it(tags->begin()),
                     tag_it_end(tags->end()); tag_it_end != tag_it; ++tag_it)
            {
                if ("dependency" != tag_it->tag->category())
                    continue;
                tr1::shared_ptr<const DependencyDepTag> tag(
                    tr1::static_pointer_cast<const DependencyDepTag>(tag_it->tag));

                if ("cat/one-1:0::repo" == tag->short_text())
                {
                    cat_three_has_tag_from_cat_one = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*tag->dependency(), "cat/three");
                    StringifyFormatter ff;
                    erepository::DepSpecPrettyPrinter pretty(0, tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
                    tag->conditions()->accept(pretty);
                    TEST_CHECK_STRINGIFY_EQUAL(pretty, "cat/three");
                }

                else if ("cat/two-1:0::repo" == tag->short_text())
                {
                    if ("<cat/three-1" == stringify(*tag->dependency()))
                    {
                        cat_three_has_first_tag_from_cat_two = true;
                        StringifyFormatter ff;
                        erepository::DepSpecPrettyPrinter pretty(0, tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
                        tag->conditions()->accept(pretty);
                        TEST_CHECK_STRINGIFY_EQUAL(pretty, "enabled? ( || ( <cat/three-1 ) )");
                    }
                    else if ("cat/three:0" == stringify(*tag->dependency()))
                    {
                        cat_three_has_second_tag_from_cat_two = true;
                        StringifyFormatter ff;
                        erepository::DepSpecPrettyPrinter pretty(0, tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
                        tag->conditions()->accept(pretty);
                        TEST_CHECK_STRINGIFY_EQUAL(pretty, "enabled? ( || ( cat/three:0 ) )");
                    }
                }
            }

            TEST_CHECK(cat_three_has_tag_from_cat_one);
            TEST_CHECK(cat_three_has_first_tag_from_cat_two);
            TEST_CHECK(cat_three_has_second_tag_from_cat_two);

            // tags for cat/one
            ++it;
            tags = it->tags;
            TEST_CHECK_EQUAL(tags->size(), 1U);
            TEST_CHECK_EQUAL(tags->begin()->tag->category(), "target");

            // tags for cat/four
            ++it;
            tags = it->tags;
            TEST_CHECK_EQUAL(tags->size(), 1U);
            TEST_CHECK_EQUAL(tags->begin()->tag->category(), "dependency");
            tr1::shared_ptr<const DependencyDepTag> deptag(
                tr1::static_pointer_cast<const DependencyDepTag>(tags->begin()->tag));
            TEST_CHECK_EQUAL(deptag->short_text(), "cat/two-1:0::repo");
            TEST_CHECK_STRINGIFY_EQUAL(*deptag->dependency(), "=cat/four-1");
            StringifyFormatter ff;
            erepository::DepSpecPrettyPrinter pretty(0, tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
            deptag->conditions()->accept(pretty);
            TEST_CHECK_STRINGIFY_EQUAL(pretty, "enabled? ( || ( =cat/four-1 ) )");

            // tags for cat/two
            ++it;
            tags = it->tags;
            TEST_CHECK_EQUAL(tags->size(), 1U);
            TEST_CHECK_EQUAL(tags->begin()->tag->category(), "general");
            TEST_CHECK_EQUAL(tags->begin()->tag->short_text(), "set");
        }
    } test_dep_list_dependency_tags;
}

