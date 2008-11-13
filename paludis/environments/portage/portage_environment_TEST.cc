/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include "portage_environment.hh"
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <paludis/util/join.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/options.hh>
#include <paludis/package_id.hh>
#include <paludis/package_database.hh>
#include <paludis/dep_spec.hh>
#include <paludis/name.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>

using namespace paludis;
using namespace test;

namespace
{
    class TestPortageEnvironment :
        public PortageEnvironment
    {
        public:
            using PortageEnvironment::accept_keywords;

            TestPortageEnvironment(const std::string & e) :
                PortageEnvironment(e)
            {
            }
    };

    bool accept_keyword(const TestPortageEnvironment & env,
            const KeywordName & k, const PackageID & e)
    {
        std::tr1::shared_ptr<KeywordNameSet> kk(new KeywordNameSet);
        kk->insert(k);
        return env.accept_keywords(kk, e);
    }

    bool get_use(const std::string & f, const Environment &, const std::tr1::shared_ptr<const PackageID> & id)
    {
        const std::tr1::shared_ptr<const ChoiceValue> v(id->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix(f)));
        if (! v)
            return false;
        return v->enabled();
    }
}

namespace test_cases
{
    struct QueryUseTest : TestCase
    {
        QueryUseTest() : TestCase("query_use") { }

        void run()
        {
            PortageEnvironment env(stringify(FSEntry("portage_environment_TEST_dir/query_use").realpath()));

            const std::tr1::shared_ptr<const PackageID> idx(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-x-1",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

            const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

            TEST_CHECK(get_use("one", env, idx));
            TEST_CHECK(get_use("two", env, idx));
            TEST_CHECK(! get_use("three", env, idx));
            TEST_CHECK(! get_use("four", env, idx));
            TEST_CHECK(! get_use("five", env, idx));
            TEST_CHECK(! get_use("six", env, idx));

            TEST_CHECK(! get_use("one", env, id1));
            TEST_CHECK(get_use("two", env, id1));
            TEST_CHECK(! get_use("three", env, id1));
            TEST_CHECK(get_use("four", env, id1));
            TEST_CHECK(! get_use("five", env, id1));
            TEST_CHECK(! get_use("six", env, id1));
        }
    } test_query_use;

    struct KnownUseNamesTest : TestCase
    {
        KnownUseNamesTest() : TestCase("known_use_expand_names") { }

        void run()
        {
            PortageEnvironment env(stringify(FSEntry("portage_environment_TEST_dir/known_use_expand_names").realpath()));

            const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
            std::tr1::shared_ptr<const Choice> foo_cards;
            for (Choices::ConstIterator c(id1->choices_key()->value()->begin()), c_end(id1->choices_key()->value()->end()) ;
                    c != c_end ; ++c)
                if ((*c)->raw_name() == "FOO_CARDS")
                    foo_cards = *c;
            if (! foo_cards)
                throw InternalError(PALUDIS_HERE, "oops");
            std::tr1::shared_ptr<const Set<UnprefixedChoiceName> > k1(env.known_choice_value_names(id1, foo_cards));
            TEST_CHECK_EQUAL(join(k1->begin(), k1->end(), " "), "one three");
        }
    } test_known_use_expand;

    struct AcceptKeywordsTest : TestCase
    {
        AcceptKeywordsTest() : TestCase("accept_keywords") { }

        void run()
        {
            TestPortageEnvironment env("portage_environment_TEST_dir/accept_keywords");

            const std::tr1::shared_ptr<const PackageID> idx(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-x-1",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

            TEST_CHECK(accept_keyword(env, KeywordName("arch"), *idx));
            TEST_CHECK(accept_keyword(env, KeywordName("other_arch"), *idx));
            TEST_CHECK(! accept_keyword(env, KeywordName("~arch"), *idx));

            const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

            TEST_CHECK(accept_keyword(env, KeywordName("arch"), *id1));
            TEST_CHECK(accept_keyword(env, KeywordName("other_arch"), *id1));
            TEST_CHECK(accept_keyword(env, KeywordName("~arch"), *id1));

            const std::tr1::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-1",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

            TEST_CHECK(accept_keyword(env, KeywordName("other_arch"), *id2));
            TEST_CHECK(accept_keyword(env, KeywordName("arch"), *id2));
            TEST_CHECK(accept_keyword(env, KeywordName("~arch"), *id2));

            const std::tr1::shared_ptr<const PackageID> id3(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-three-1",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

            TEST_CHECK(! accept_keyword(env, KeywordName("other_arch"), *id3));
            TEST_CHECK(! accept_keyword(env, KeywordName("arch"), *id3));
            TEST_CHECK(! accept_keyword(env, KeywordName("~arch"), *id3));

            const std::tr1::shared_ptr<const PackageID> id4(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-four-1",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
            TEST_CHECK(accept_keyword(env, KeywordName("fred"), *id4));
        }
    } test_accept_keywords;
}

