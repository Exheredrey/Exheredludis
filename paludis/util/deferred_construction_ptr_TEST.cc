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

#include <paludis/util/deferred_construction_ptr.hh>
#include <paludis/util/active_object_ptr.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <test/test_concepts.hh>

using namespace paludis;
using namespace test;

namespace
{
    int * make_ten()
    {
        return new int(10);
    }

    std::tr1::shared_ptr<int> make_ten_shared()
    {
        return make_shared_ptr(new int(10));
    }

    std::string * make_monkey()
    {
        return new std::string("monkey");
    }

    std::tr1::shared_ptr<std::string> make_chimp_shared()
    {
        return make_shared_ptr(new std::string("chimp"));
    }
}

namespace test_cases
{
    typedef DeferredConstructionPtr<int *> DeferredIntPtr;
    TESTCASE_SEMIREGULAR(DeferredIntPtr, DeferredIntPtr(make_ten));

    typedef DeferredConstructionPtr<std::tr1::shared_ptr<int> > DeferredSharedIntPtr;
    TESTCASE_SEMIREGULAR(DeferredSharedIntPtr, DeferredSharedIntPtr(make_ten_shared));

    struct TestDereference : TestCase
    {
        TestDereference() : TestCase("dereference") { }

        void run()
        {
            DeferredConstructionPtr<std::string *> p(make_monkey);
            TEST_CHECK_EQUAL(p->length(), 6u);

            DeferredConstructionPtr<std::tr1::shared_ptr<std::string> > q(make_chimp_shared);
            TEST_CHECK_EQUAL(q->length(), 5u);
        }
    } test_dereference;

    struct TestDeferredActive : TestCase
    {
        TestDeferredActive() : TestCase("dereferred active") { }

        void run()
        {
            ActiveObjectPtr<DeferredConstructionPtr<std::string *> > p((
                    DeferredConstructionPtr<std::string *>(make_monkey)));
            TEST_CHECK_EQUAL(p->length(), 6u);

            ActiveObjectPtr<DeferredConstructionPtr<std::tr1::shared_ptr<std::string> > > q((
                    DeferredConstructionPtr<std::tr1::shared_ptr<std::string> >(make_chimp_shared)));
            TEST_CHECK_EQUAL(q->length(), 5u);
        }
    } test_dereferred_active;

    struct TestConstructOnlyOnce : TestCase
    {
        TestConstructOnlyOnce() : TestCase("construct only once") { }

        struct Flag
        {
            bool value;
            Flag() : value(false) { }
        };

        static std::tr1::shared_ptr<Flag> make_flag()
        {
            return make_shared_ptr(new Flag);
        }

        void run()
        {
            DeferredConstructionPtr<std::tr1::shared_ptr<Flag> > f(make_flag);
            f->value = true;
            TEST_CHECK(f->value);
        }
    } test_construct_only_once;

    struct TestConstructOnlyOnceActive : TestCase
    {
        TestConstructOnlyOnceActive() : TestCase("construct only once active") { }

        struct Flag
        {
            bool value;
            Flag() : value(false) { }
        };

        static std::tr1::shared_ptr<Flag> make_flag()
        {
            return make_shared_ptr(new Flag);
        }

        void run()
        {
            ActiveObjectPtr<DeferredConstructionPtr<std::tr1::shared_ptr<Flag> > > f((
                    DeferredConstructionPtr<std::tr1::shared_ptr<Flag> >(make_flag)));
            f->value = true;
            TEST_CHECK(f->value);
        }
    } test_construct_only_once_active;
}

