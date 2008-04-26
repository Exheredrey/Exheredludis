/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/clone-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/version_requirements.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct DepSpecAsTest : TestCase
    {
        DepSpecAsTest() : TestCase("dep spec as") { }

        void run()
        {
            std::tr1::shared_ptr<PackageDepSpec> x(new PackageDepSpec(make_package_dep_spec()));
            TEST_CHECK(0 == x->as_conditional_dep_spec());
        }
    } test_dep_spec_as;

    struct FetchableURIDepSpecTest : TestCase
    {
        FetchableURIDepSpecTest() : TestCase("fetchable uri dep spec") { }

        void run()
        {
            FetchableURIDepSpec a("foo");
            TEST_CHECK_EQUAL(a.original_url(), "foo");
            TEST_CHECK_EQUAL(a.renamed_url_suffix(), "");
            TEST_CHECK_EQUAL(a.filename(), "foo");

            FetchableURIDepSpec b("fnord -> bar");
            TEST_CHECK_EQUAL(b.original_url(), "fnord");
            TEST_CHECK_EQUAL(b.renamed_url_suffix(), "bar");
            TEST_CHECK_EQUAL(b.filename(), "bar");

            FetchableURIDepSpec c("http://example.com/download/baz");
            TEST_CHECK_EQUAL(c.filename(), "baz");
        }
    } test_fetchable_uri_dep_spec;

    struct DepSpecCloneTest : TestCase
    {
        DepSpecCloneTest() : TestCase("dep spec clone") { }

        void run()
        {
            PackageDepSpec a(parse_user_package_dep_spec("cat/pkg:1::repo[=1|>3.2][foo]", UserPackageDepSpecOptions()));

            std::tr1::shared_ptr<PackageDepSpec> b(std::tr1::static_pointer_cast<PackageDepSpec>(a.clone()));
            TEST_CHECK_STRINGIFY_EQUAL(a, *b);

            std::tr1::shared_ptr<PackageDepSpec> c(std::tr1::static_pointer_cast<PackageDepSpec>(a.clone()));
            TEST_CHECK_STRINGIFY_EQUAL(a, *c);

            BlockDepSpec d(c);
            std::tr1::shared_ptr<BlockDepSpec> e(std::tr1::static_pointer_cast<BlockDepSpec>(d.clone()));
            TEST_CHECK_STRINGIFY_EQUAL(*(d.blocked_spec()), *(e->blocked_spec()));
        }
    } test_dep_spec_clone;
}

