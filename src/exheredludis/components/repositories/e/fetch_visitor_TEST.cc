/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/fetch_visitor.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>

#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/stringify.hh>

#include <paludis/standard_output_manager.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>

#include <iterator>

#include <gtest/gtest.h>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    const std::shared_ptr<const MirrorsSequence>
    get_mirrors_fn(const std::string & m)
    {
        const std::shared_ptr<MirrorsSequence> result(std::make_shared<MirrorsSequence>());
        if (m == "repo")
            result->push_back("http://fake-repo/fake-repo/");
        if (m == "example")
            result->push_back("http://fake-example/fake-example/");
        return result;
    }
}

TEST(FetchVisitor, Works)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    ASSERT_TRUE(FSPath("fetch_visitor_TEST_dir/in/input1").stat().exists());
    ASSERT_TRUE(! FSPath("fetch_visitor_TEST_dir/out/input1").stat().exists());

    const std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string("exheres-0"));
    FetchVisitor v(&env, *env[selection::RequireExactlyOne(
                generator::Matches(parse_user_package_dep_spec("=cat/pkg-1",
                        &env, { }), nullptr, { }))]->begin(),
            *eapi, FSPath("fetch_visitor_TEST_dir/out"),
            false, false, "test", std::make_shared<URIListedThenMirrorsLabel>("listed-then-mirrors"), false,
            std::make_shared<StandardOutputManager>(), get_mirrors_fn);
    parse_fetchable_uri("file:///" + stringify(FSPath("fetch_visitor_TEST_dir/in/input1").realpath()), &env, *eapi, false)->top()->accept(v);

    ASSERT_TRUE(FSPath("fetch_visitor_TEST_dir/out/input1").stat().is_regular_file());
    SafeIFStream f(FSPath("fetch_visitor_TEST_dir/out/input1"));
    ASSERT_TRUE(bool(f));
    std::string s((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    EXPECT_EQ("contents of one\n", s);
}

