/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/repository_factory.hh>
#include <paludis/choice.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <tr1/functional>
#include <set>
#include <string>

#include "config.h"

using namespace test;
using namespace paludis;

namespace
{
    std::string from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }

    void dummy_used_this_for_config_protect(const std::string &)
    {
    }

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }
}

namespace test_cases
{
    struct ERepositoryRepoNameTest : TestCase
    {
        ERepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo1/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "test-repo-1");
        }
    } test_e_repository_repo_name;

    struct ERepositoryNoRepoNameTest : TestCase
    {
        ERepositoryNoRepoNameTest() : TestCase("no repo name") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo2"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo2/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "x-repo2");
        }
    } test_e_repository_no_repo_name;

    struct ERepositoryEmptyRepoNameTest : TestCase
    {
        ERepositoryEmptyRepoNameTest() : TestCase("empty repo name") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo3"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo3/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "x-repo3");
        }
    } test_e_repository_empty_repo_name;

    struct ERepositoryHasCategoryNamedTest : TestCase
    {
        ERepositoryHasCategoryNamedTest() : TestCase("has category named") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo1/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-one")));
                TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-two")));
                TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-three")));
                TEST_CHECK(! repo->has_category_named(CategoryNamePart("cat-four")));
            }
        }
    } test_e_repository_has_category_named;

    struct ERepositoryCategoryNamesTest : TestCase
    {
        ERepositoryCategoryNamesTest() : TestCase("category names") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo1/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                std::tr1::shared_ptr<const CategoryNamePartSet> c(repo->category_names());
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-one")));
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-two")));
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-three")));
                TEST_CHECK(c->end() == c->find(CategoryNamePart("cat-four")));
                TEST_CHECK_EQUAL(3, std::distance(c->begin(), c->end()));
            }
        }
    } test_e_repository_category_names;

    struct ERepositoryHasPackageNamedTest : TestCase
    {
        ERepositoryHasPackageNamedTest() : TestCase("has package named") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo4"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo4/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-two/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-one")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-one/pkg-both")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-two/pkg-both")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-neither")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-neither")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-one")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-both")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-neither")));
            }
        }
    } test_e_repository_has_package_named;

    struct ERepositoryHasPackageNamedCachedTest : TestCase
    {
        ERepositoryHasPackageNamedCachedTest() : TestCase("has package named cached") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo4"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo4/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            repo->package_names(CategoryNamePart("cat-one"));
            repo->package_names(CategoryNamePart("cat-two"));
            repo->package_names(CategoryNamePart("cat-three"));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-two/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-one")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-one/pkg-both")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-two/pkg-both")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-neither")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-neither")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-one")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-both")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-neither")));
            }
        }
    } test_e_repository_has_package_named_cached;

    struct ERepositoryPackageNamesTest : TestCase
    {
        ERepositoryPackageNamesTest() : TestCase("package names") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo4"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo4/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            std::tr1::shared_ptr<const QualifiedPackageNameSet> names;

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                names = repo->package_names(CategoryNamePart("cat-one"));
                TEST_CHECK(! names->empty());
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-one/pkg-both")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-one/pkg-two")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-one/pkg-neither")));
                TEST_CHECK_EQUAL(2, std::distance(names->begin(), names->end()));

                names = repo->package_names(CategoryNamePart("cat-two"));
                TEST_CHECK(! names->empty());
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-two/pkg-one")));
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-two/pkg-both")));
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-two/pkg-two")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-two/pkg-neither")));
                TEST_CHECK_EQUAL(2, std::distance(names->begin(), names->end()));

                names = repo->package_names(CategoryNamePart("cat-three"));
                TEST_CHECK(names->empty());
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-three/pkg-one")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-three/pkg-both")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-three/pkg-two")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-three/pkg-neither")));
                TEST_CHECK_EQUAL(0, std::distance(names->begin(), names->end()));
            }
        }
    } test_e_repository_package_names;

    struct ERepositoryBadPackageNamesTest : TestCase
    {
        ERepositoryBadPackageNamesTest() : TestCase("bad package names") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo5"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo5/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            std::tr1::shared_ptr<const QualifiedPackageNameSet> names;

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                names = repo->package_names(CategoryNamePart("cat-one"));
                TEST_CHECK(! names->empty());
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK_EQUAL(1, std::distance(names->begin(), names->end()));
            }
        }
    } test_e_repository_bad_package_names;

    struct ERepositoryPackageIDTest : TestCase
    {
        ERepositoryPackageIDTest() : TestCase("package_ids") { }

        void run()
        {
            using namespace std::tr1::placeholders;

            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo4"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo4/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                std::tr1::shared_ptr<const PackageIDSequence> versions;

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-one"));
                TEST_CHECK(! versions->empty());
                TEST_CHECK_EQUAL(2, std::distance(versions->begin(), versions->end()));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::tr1::bind(std::equal_to<VersionSpec>(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1), VersionSpec("1"))));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::tr1::bind(std::equal_to<VersionSpec>(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1), VersionSpec("1.1-r1"))));
                TEST_CHECK(indirect_iterator(versions->end()) == std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::tr1::bind(std::equal_to<VersionSpec>(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1), VersionSpec("2"))));

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-neither"));
                TEST_CHECK(versions->empty());
                TEST_CHECK_EQUAL(0, std::distance(versions->begin(), versions->end()));
            }
        }
    } test_e_repository_versions;

    struct ERepositoryDuffVersionsTest : TestCase
    {
        ERepositoryDuffVersionsTest() : TestCase("duff versions") { }

        void run()
        {
            using namespace std::tr1::placeholders;

            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo8"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo8/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                std::tr1::shared_ptr<const PackageIDSequence> versions;

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-one"));
                TEST_CHECK(! versions->empty());
                TEST_CHECK_EQUAL(2, std::distance(versions->begin(), versions->end()));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::tr1::bind(std::equal_to<VersionSpec>(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1), VersionSpec("1"))));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::tr1::bind(std::equal_to<VersionSpec>(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1), VersionSpec("1.1-r1"))));
                TEST_CHECK(indirect_iterator(versions->end()) == std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::tr1::bind(std::equal_to<VersionSpec>(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1), VersionSpec("2"))));

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-neither"));
                TEST_CHECK(versions->empty());
                TEST_CHECK_EQUAL(0, std::distance(versions->begin(), versions->end()));
            }
        }
    } test_e_repository_duff_versions;

    struct ERepositoryMetadataUncachedTest : TestCase
    {
        ERepositoryMetadataUncachedTest() : TestCase("metadata uncached") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        void run()
        {
            for (int opass = 1 ; opass <= 3 ; ++opass)
            {
                TestMessageSuffix opass_suffix("opass=" + stringify(opass), true);

                TestEnvironment env;
                env.set_paludis_command("/bin/false");
                std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                        new Map<std::string, std::string>);
                keys->insert("format", "ebuild");
                keys->insert("names_cache", "/var/empty");
                keys->insert("write_cache", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo7/metadata/cache"));
                keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo7"));
                keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo7/profiles/profile"));
                keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
                std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                            std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
                env.package_database()->add_repository(1, repo);

                for (int pass = 1 ; pass <= 3 ; ++pass)
                {
                    TestMessageSuffix pass_suffix("pass=" + stringify(pass), true);

                    const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                            &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

                    TEST_CHECK(id1->end_metadata() != id1->find_metadata("EAPI"));
                    TEST_CHECK(simple_visitor_cast<const MetadataValueKey<std::string> >(**id1->find_metadata("EAPI")));
                    TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id1->find_metadata("EAPI"))->value(), "0");
                    TEST_CHECK(id1->short_description_key());
                    TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Description");
                    StringifyFormatter ff;
                    erepository::DepSpecPrettyPrinter pd(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
                    TEST_CHECK(id1->build_dependencies_key());
                    id1->build_dependencies_key()->value()->root()->accept(pd);
                    TEST_CHECK_STRINGIFY_EQUAL(pd, "foo/bar");
                    erepository::DepSpecPrettyPrinter pr(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
                    TEST_CHECK(id1->run_dependencies_key());
                    id1->run_dependencies_key()->value()->root()->accept(pr);
                    TEST_CHECK_STRINGIFY_EQUAL(pr, "foo/bar");

                    const std::tr1::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-2",
                                            &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

                    TEST_CHECK(id2->end_metadata() != id2->find_metadata("EAPI"));
                    TEST_CHECK(id2->short_description_key());
                    TEST_CHECK_EQUAL(id2->short_description_key()->value(), "dquote \" squote ' backslash \\ dollar $");
                    erepository::DepSpecPrettyPrinter pd2(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
                    TEST_CHECK(id2->build_dependencies_key());
                    id2->build_dependencies_key()->value()->root()->accept(pd2);
                    TEST_CHECK_STRINGIFY_EQUAL(pd2, "foo/bar bar/baz");
                    erepository::DepSpecPrettyPrinter pr2(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
                    TEST_CHECK(id2->run_dependencies_key());
                    id2->run_dependencies_key()->value()->root()->accept(pr2);
                    TEST_CHECK_STRINGIFY_EQUAL(pr2, "foo/bar");

                    const std::tr1::shared_ptr<const PackageID> id3(*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-3",
                                            &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

                    TEST_CHECK(id3->end_metadata() != id3->find_metadata("EAPI"));
                    TEST_CHECK(id3->short_description_key());
                    TEST_CHECK_EQUAL(id3->short_description_key()->value(), "This is the short description");
                    TEST_CHECK(id3->long_description_key());
                    TEST_CHECK_EQUAL(id3->long_description_key()->value(), "This is the long description");
                }
            }
        }
    } test_e_repository_metadata_uncached;

    struct ERepositoryMetadataUnparsableTest : TestCase
    {
        ERepositoryMetadataUnparsableTest() : TestCase("metadata unparsable") { }

        bool skip() const
        {
            return ! getenv_with_default("SANDBOX_ON", "").empty();
        }

        unsigned max_run_time() const
        {
            return 3000;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo7"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo7/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

                TEST_CHECK(id1->end_metadata() != id1->find_metadata("EAPI"));
                TEST_CHECK_EQUAL(std::tr1::static_pointer_cast<const erepository::ERepositoryID>(id1)->eapi()->name(), "UNKNOWN");
                TEST_CHECK(! id1->short_description_key());
            }
        }
    } test_e_repository_metadata_unparsable;

    struct ERepositoryQueryUseTest : TestCase
    {
        ERepositoryQueryUseTest() : TestCase("USE query") { }

        void test_choice(const std::tr1::shared_ptr<const PackageID> & p, const std::string & n, bool enabled, bool enabled_by_default, bool locked)
        {
            TestMessageSuffix s(stringify(*p) + "[" + n + "]", true);
            std::tr1::shared_ptr<const ChoiceValue> choice(p->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix(n)));
            TEST_CHECK_EQUAL(choice->enabled(), enabled);
            TEST_CHECK_EQUAL(choice->enabled_by_default(), enabled_by_default);
            TEST_CHECK_EQUAL(choice->locked(), locked);
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo9"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo9/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                            std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                const std::tr1::shared_ptr<const PackageID> p1(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                const std::tr1::shared_ptr<const PackageID> p2(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-two/pkg-two-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                const std::tr1::shared_ptr<const PackageID> p4(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-2",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

                test_choice(p1, "flag1",     true,  true,  false);
                test_choice(p1, "flag2",     false, false, true);
                test_choice(p1, "flag3",     true,  true,  false);
                test_choice(p1, "flag4",     true,  true,  true);
                test_choice(p1, "flag5",     false, false, false);
                test_choice(p1, "enabled",   true,  false, false);
                test_choice(p1, "disabled",  false, true,  false);
                test_choice(p1, "enabled2",  true,  false, false);
                test_choice(p1, "disabled2", false, true,  false);
                test_choice(p1, "enabled3",  false, false, true);
                test_choice(p1, "disabled3", true,  true,  true);

                test_choice(p2, "flag1", true,  true,  false);
                test_choice(p2, "flag2", false, false, true);
                test_choice(p2, "flag3", false, false, true);
                test_choice(p2, "flag4", true,  true,  true);
                test_choice(p2, "flag5", true,  true,  true);
                test_choice(p2, "flag6", true,  true,  false);

                test_choice(p4, "flag1", true,  true,  false);
                test_choice(p4, "flag2", false, false, true);
                test_choice(p4, "flag3", false, false, true);
                test_choice(p4, "flag4", true,  true,  true);
                test_choice(p4, "flag5", true,  true,  false);
                test_choice(p4, "flag6", false, false, false);

                test_choice(p1, "test",  true,  true,  true);
                test_choice(p1, "test2", false, false, true);
            }
        }
    } test_e_repository_query_use;

    struct ERepositoryRepositoryMasksTest : TestCase
    {
        ERepositoryRepositoryMasksTest() : TestCase("repository masks") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");

            std::tr1::shared_ptr<Map<std::string, std::string> > keys18(
                    new Map<std::string, std::string>);
            keys18->insert("format", "ebuild");
            keys18->insert("names_cache", "/var/empty");
            keys18->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo18"));
            keys18->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo18/profiles/profile"));
            keys18->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo18(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys18, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo18);

            std::tr1::shared_ptr<Map<std::string, std::string> > keys19(
                    new Map<std::string, std::string>);
            keys19->insert("format", "ebuild");
            keys19->insert("names_cache", "/var/empty");
            keys19->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo19"));
            keys19->insert("master_repository", "test-repo-18");
            keys19->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo19(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys19, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo19);

            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-1::test-repo-18",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-2::test-repo-18",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-3::test-repo-18",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-4::test-repo-18",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());

            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-1::test-repo-19",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-2::test-repo-19",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-3::test-repo-19",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-4::test-repo-19",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
        }
    } test_e_repository_repository_masks;

    struct ERepositoryQueryProfileMasksTest : TestCase
    {
        ERepositoryQueryProfileMasksTest() : TestCase("profiles package.mask") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo10"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo10/profiles/profile/subprofile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat/masked-0",
                                            &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
                TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                            &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
                TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat/not_masked-0",
                                            &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
            }
        }
    } test_e_repository_query_profile_masks;

    struct ERepositoryInvalidateMasksTest : TestCase
    {
        ERepositoryInvalidateMasksTest() : TestCase("invalidate_masks") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo10"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo10/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                            std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
            repo->set_profile(repo->find_profile(repo->params().location() / "profiles/profile/subprofile"));
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
            repo->set_profile(repo->find_profile(repo->params().location() / "profiles/profile"));
            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->masked());
        }
    } test_e_repository_invalidate_masks;

    struct ERepositoryVirtualsTest : TestCase
    {
        ERepositoryVirtualsTest() : TestCase("virtuals") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo15"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo15/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                            std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            bool has_one(false), has_two(false), has_three(false);
            int count(0);

            std::tr1::shared_ptr<const RepositoryVirtualsInterface::VirtualsSequence> seq(repo->virtual_packages());
            for (RepositoryVirtualsInterface::VirtualsSequence::ConstIterator it(seq->begin()),
                    it_end(seq->end()); it_end != it; ++it, ++count)
                if ("virtual/one" == stringify(it->virtual_name()))
                {
                    has_one = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec(), "cat-one/pkg-one");
                }
                else
                {
                    TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/two");
                    has_two = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec(), "cat-two/pkg-two");
                }

            TEST_CHECK(has_one);
            TEST_CHECK(has_two);
            TEST_CHECK_EQUAL(count, 2);

            repo->set_profile(repo->find_profile(repo->params().location() / "profiles/profile/subprofile"));

            has_one = has_two = false;
            count = 0;

            seq = repo->virtual_packages();
            for (RepositoryVirtualsInterface::VirtualsSequence::ConstIterator it(seq->begin()),
                    it_end(seq->end()); it_end != it; ++it, ++count)
                if ("virtual/one" == stringify(it->virtual_name()))
                {
                    has_one = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec(), "cat-two/pkg-two");
                }
                else if ("virtual/two" == stringify(it->virtual_name()))
                {
                    has_two = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec(), "cat-one/pkg-one");
                }
                else
                {
                    TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/three");
                    has_three = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec(), "cat-three/pkg-three");
                }

            TEST_CHECK(has_one);
            TEST_CHECK(has_two);
            TEST_CHECK(has_three);
            TEST_CHECK_EQUAL(count, 3);
        }
    } test_e_repository_virtuals;

    struct ERepositoryManifestTest : TestCase
    {
        ERepositoryManifestTest() : TestCase("manifest2") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo11"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo11/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                            std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            repo->make_manifest(QualifiedPackageName("category/package"));

            std::multiset<std::string> made_manifest, reference_manifest;
            SafeIFStream made_manifest_stream(FSEntry("e_repository_TEST_dir/repo11/category/package/Manifest")),
                reference_manifest_stream(FSEntry("e_repository_TEST_dir/repo11/Manifest_correct"));

            std::string line;

            while ( getline(made_manifest_stream, line) )
                made_manifest.insert(line);
            while ( getline(reference_manifest_stream, line) )
                reference_manifest.insert(line);

            TEST_CHECK(made_manifest == reference_manifest);

            TEST_CHECK_THROWS(repo->make_manifest(QualifiedPackageName("category/package-b")), SafeIFStreamError);
        }
    } test_e_repository_manifest;

    struct ERepositoryFetchTest : TestCase
    {
        ERepositoryFetchTest() : TestCase("fetch") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "exheres");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo12"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo12/profiles/profile"));
            keys->insert("layout", "exheres");
            keys->insert("eapi_when_unknown", "exheres-0");
            keys->insert("eapi_when_unspecified", "exheres-0");
            keys->insert("profile_eapi", "exheres-0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            FetchAction action(make_named_values<FetchActionOptions>(
                        value_for<n::exclude_unmirrorable>(false),
                        value_for<n::fetch_unneeded>(false),
                        value_for<n::maybe_output_deviant>(make_null_shared_ptr()),
                        value_for<n::safe_resume>(true)
                    ));

            {
                TestMessageSuffix suffix("no files", true);
                const std::tr1::shared_ptr<const PackageID> no_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/no-files",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(no_files_id);
                TEST_CHECK(no_files_id->short_description_key());
                TEST_CHECK_EQUAL(no_files_id->short_description_key()->value(), "The Short Description");
                no_files_id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("fetched files", true);
                const std::tr1::shared_ptr<const PackageID> fetched_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetched-files",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(fetched_files_id);
                TEST_CHECK((FSEntry("e_repository_TEST_dir") / "distdir" / "already-fetched.txt").is_regular_file());
                fetched_files_id->perform_action(action);
                TEST_CHECK((FSEntry("e_repository_TEST_dir") / "distdir" / "already-fetched.txt").is_regular_file());
            }

            {
                TestMessageSuffix suffix("fetchable files", true);
                TEST_CHECK(! (FSEntry("e_repository_TEST_dir") / "distdir" / "fetchable-1.txt").is_regular_file());
                const std::tr1::shared_ptr<const PackageID> fetchable_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetchable-files",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(fetchable_files_id);
                fetchable_files_id->perform_action(action);
                TEST_CHECK((FSEntry("e_repository_TEST_dir") / "distdir" / "fetchable-1.txt").is_regular_file());
            }

            {
                TestMessageSuffix suffix("arrow files", true);
                TEST_CHECK(! (FSEntry("e_repository_TEST_dir") / "distdir" / "arrowed.txt").is_regular_file());
                const std::tr1::shared_ptr<const PackageID> arrow_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/arrow-files",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(arrow_files_id);
                arrow_files_id->perform_action(action);
                TEST_CHECK((FSEntry("e_repository_TEST_dir") / "distdir" / "arrowed.txt").is_regular_file());
            }

            {
                TestMessageSuffix suffix("unfetchable files", true);
                const std::tr1::shared_ptr<const PackageID> unfetchable_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/unfetchable-files",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(unfetchable_files_id);
                TEST_CHECK_THROWS(unfetchable_files_id->perform_action(action), FetchActionError);
            }

            {
                const std::tr1::shared_ptr<const PackageID> no_files_restricted_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/no-files-restricted",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(no_files_restricted_id);
                no_files_restricted_id->perform_action(action);
            }

            {
                const std::tr1::shared_ptr<const PackageID> fetched_files_restricted_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetched-files-restricted",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(fetched_files_restricted_id);
                fetched_files_restricted_id->perform_action(action);
            }

            {
                const std::tr1::shared_ptr<const PackageID> fetchable_files_restricted_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetchable-files-restricted",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(fetchable_files_restricted_id);
                TEST_CHECK_THROWS(fetchable_files_restricted_id->perform_action(action), FetchActionError);
            }
        }
    } test_e_repository_fetch;

    struct ERepositoryManifestCheckTest : TestCase
    {
        ERepositoryManifestCheckTest() : TestCase("manifest_check") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo11"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo11/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                            std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            FetchAction action(make_named_values<FetchActionOptions>(
                        value_for<n::exclude_unmirrorable>(false),
                        value_for<n::fetch_unneeded>(false),
                        value_for<n::maybe_output_deviant>(make_null_shared_ptr()),
                        value_for<n::safe_resume>(true)
                    ));

            const std::tr1::shared_ptr<const PackageID> id(*env[selection::AllVersionsSorted(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("category/package",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
            TEST_CHECK(id);
            repo->make_manifest(id->name());
            id->perform_action(action);
        }
    } test_e_repository_manifest_check;

    struct ERepositoryInstallEAPI0Test : TestCase
    {
        ERepositoryInstallEAPI0Test() : TestCase("install_eapi_0") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
#ifdef ENABLE_VIRTUALS_REPOSITORY
            ::setenv("PALUDIS_ENABLE_VIRTUALS_REPOSITORY", "yes", 1);
#else
            ::setenv("PALUDIS_ENABLE_VIRTUALS_REPOSITORY", "", 1);
#endif

            TestEnvironment env;
            env.set_paludis_command("/bin/false");

            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo13"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo13/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            installed_repo->add_version("cat", "pretend-installed", "0")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            installed_repo->add_version("cat", "pretend-installed", "1")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            env.package_database()->add_repository(2, installed_repo);

#ifdef ENABLE_VIRTUALS_REPOSITORY
            std::tr1::shared_ptr<Map<std::string, std::string> > iv_keys(new Map<std::string, std::string>);
            iv_keys->insert("root", "/");
            iv_keys->insert("format", "installed_virtuals");
            env.package_database()->add_repository(-2, RepositoryFactory::get_instance()->create(&env,
                        std::tr1::bind(from_keys, iv_keys, std::tr1::placeholders::_1)));
            std::tr1::shared_ptr<Map<std::string, std::string> > v_keys(new Map<std::string, std::string>);
            v_keys->insert("format", "virtuals");
            env.package_database()->add_repository(-2, RepositoryFactory::get_instance()->create(&env,
                        std::tr1::bind(from_keys, v_keys, std::tr1::placeholders::_1)));
#endif

            InstallAction action(make_named_values<InstallActionOptions>(
                        value_for<n::destination>(installed_repo),
                        value_for<n::used_this_for_config_protect>(&dummy_used_this_for_config_protect),
                        value_for<n::want_phase>(&want_all_phases)
                    ));

#ifdef ENABLE_VIRTUALS_REPOSITORY
            {
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=virtual/virtual-pretend-installed-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
            }
#endif

            {
                TestMessageSuffix suffix("in-ebuild die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-ebuild-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("in-subshell die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-subshell-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("unpack die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/unpack-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("econf die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/econf-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("emake fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/emake-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("emake die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/emake-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("einstall die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/einstall-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("keepdir die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/keepdir-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("dobin fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/dobin-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("dobin die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/dobin-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("fperms fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fperms-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("fperms die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fperms-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("econf source 0", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "0");
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("doman 0", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/doman-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "0");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("no src_prepare 0", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_prepare-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "0");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("no src_configure 0", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_configure-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "0");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("best version", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/best-version-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("has version", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/has-version-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("match", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/match-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("vars", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/vars-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("expand vars", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/expand-vars-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }
        }
    } test_e_repository_install_eapi_0;

    struct ERepositoryInstallEAPI1Test : TestCase
    {
        ERepositoryInstallEAPI1Test() : TestCase("install_eapi_1") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo13"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo13/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            env.package_database()->add_repository(2, installed_repo);

            InstallAction action(make_named_values<InstallActionOptions>(
                        value_for<n::destination>(installed_repo),
                        value_for<n::used_this_for_config_protect>(&dummy_used_this_for_config_protect),
                        value_for<n::want_phase>(&want_all_phases)
                    ));

            {
                TestMessageSuffix suffix("econf source 1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("dosym success 1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/dosym-success-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("doman 1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/doman-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("no src_prepare 1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_prepare-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("no src_configure 1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_configure-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "1");
                id->perform_action(action);
            }
        }
    } test_e_repository_install_eapi_1;

    struct ERepositoryInstallEAPI2Test : TestCase
    {
        ERepositoryInstallEAPI2Test() : TestCase("install_eapi_2") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo13"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo13/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            env.package_database()->add_repository(2, installed_repo);

            InstallAction action(make_named_values<InstallActionOptions>(
                        value_for<n::destination>(installed_repo),
                        value_for<n::used_this_for_config_protect>(&dummy_used_this_for_config_protect),
                        value_for<n::want_phase>(&want_all_phases)
                    ));

            {
                TestMessageSuffix suffix("econf source 2", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-2",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("doman 2", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/doman-2",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("src_prepare 2", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_prepare-2",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("src_configure 2", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_configure-2",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("default src_configure 2", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/default-src_configure-2",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("default src_compile 2", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/default-src_compile-2",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("default_src_compile 2", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/default_src_compile-2",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("src_compile via default function 2", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_compile-via-default-func-2",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                id->perform_action(action);
            }
        }
    } test_e_repository_install_eapi_2;

    struct ERepositoryInstallEAPIKdebuild1Test : TestCase
    {
        ERepositoryInstallEAPIKdebuild1Test() : TestCase("install_eapi_kdebuild_1") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo13"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo13/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            env.package_database()->add_repository(2, installed_repo);

            InstallAction action(make_named_values<InstallActionOptions>(
                        value_for<n::destination>(installed_repo),
                        value_for<n::used_this_for_config_protect>(&dummy_used_this_for_config_protect),
                        value_for<n::want_phase>(&want_all_phases)
                        ));

            {
                TestMessageSuffix suffix("econf source kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("banned functions kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/banned-functions-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("banned vars kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/banned-vars-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "UNKNOWN");
            }

            {
                TestMessageSuffix suffix("dosym success kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/dosym-success-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("dosym fail kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/dosym-fail-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("doman kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/doman-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("no src_prepare kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_prepare-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("no src_configure kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_configure-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }
        }
    } test_e_repository_install_eapi_kdebuild_1;

    struct ERepositoryInfoEAPIKdebuild1Test : TestCase
    {
        ERepositoryInfoEAPIKdebuild1Test() : TestCase("info_eapi_kdebuild_1") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo13"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo13/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            env.package_database()->add_repository(2, installed_repo);

            InfoAction action;

            {
                TestMessageSuffix suffix("info success kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/info-success-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("info fail kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/info-fail-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                TEST_CHECK_THROWS(id->perform_action(action), InfoActionError);
            }
        }
    } test_e_repository_info_eapi_kdebuild_1;

    struct ERepositoryInstallExheres0Test : TestCase
    {
        ERepositoryInstallExheres0Test() : TestCase("install_exheres_0") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
#ifdef ENABLE_VIRTUALS_REPOSITORY
            ::setenv("PALUDIS_ENABLE_VIRTUALS_REPOSITORY", "yes", 1);
#else
            ::setenv("PALUDIS_ENABLE_VIRTUALS_REPOSITORY", "", 1);
#endif
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo14"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo14/profiles/profile"));
            keys->insert("layout", "exheres");
            keys->insert("eapi_when_unknown", "exheres-0");
            keys->insert("eapi_when_unspecified", "exheres-0");
            keys->insert("profile_eapi", "exheres-0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            installed_repo->add_version("cat", "pretend-installed", "0")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            installed_repo->add_version("cat", "pretend-installed", "1")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            env.package_database()->add_repository(2, installed_repo);

#ifdef ENABLE_VIRTUALS_REPOSITORY
            std::tr1::shared_ptr<Map<std::string, std::string> > iv_keys(new Map<std::string, std::string>);
            iv_keys->insert("root", "/");
            iv_keys->insert("format", "installed_virtuals");
            env.package_database()->add_repository(-2, RepositoryFactory::get_instance()->create(&env,
                        std::tr1::bind(from_keys, iv_keys, std::tr1::placeholders::_1)));
            std::tr1::shared_ptr<Map<std::string, std::string> > v_keys(new Map<std::string, std::string>);
            v_keys->insert("format", "virtuals");
            env.package_database()->add_repository(-2, RepositoryFactory::get_instance()->create(&env,
                        std::tr1::bind(from_keys, v_keys, std::tr1::placeholders::_1)));
#endif

            InstallAction action(make_named_values<InstallActionOptions>(
                        value_for<n::destination>(installed_repo),
                        value_for<n::used_this_for_config_protect>(&dummy_used_this_for_config_protect),
                        value_for<n::want_phase>(&want_all_phases)
                    ));

            {
                TestMessageSuffix suffix("in-ebuild die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-ebuild-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("in-subshell die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-subshell-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("expatch success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/expatch-success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("expatch die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/expatch-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal expatch fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-expatch-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal expatch die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-expatch-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("unpack die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/unpack-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal unpack fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-unpack-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal unpack die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-unpack-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("econf fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/econf-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal econf", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-econf",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal econf die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-econf-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("emake fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/emake-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal emake", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-emake",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal emake die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-emake-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("einstall fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/einstall-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal einstall", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-einstall",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal einstall die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-einstall-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("keepdir success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/keepdir-success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("keepdir fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/keepdir-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal keepdir", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-keepdir",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal keepdir die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-keepdir-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("dobin success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/dobin-success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("dobin fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/dobin-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal dobin success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-dobin-success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal dobin fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-dobin-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal dobin die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-dobin-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("herebin success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/herebin-success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("herebin fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/herebin-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("hereconfd success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/hereconfd-success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("hereconfd fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/hereconfd-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("hereenvd success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/hereenvd-success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("hereenvd fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/hereenvd-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("hereinitd success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/hereinitd-success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("hereinitd fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/hereinitd-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("hereins success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/hereins-success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("hereins fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/hereins-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("heresbin success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/heresbin-success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("heresbin fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/heresbin-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("fperms success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fperms-success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("fperms fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fperms-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal fperms success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-fperms-success",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal fperms fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-fperms-fail",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal fperms die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-fperms-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("best version", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/best-version-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("has version", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/has-version-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("match", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/match-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("econf phase", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-phase-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("econf vars", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-vars-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("expand vars", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/expand-vars-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("doman success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/doman-success-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("doman nofatal", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/doman-nonfatal-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("doman failure", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/doman-failure-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }
        }
    } test_e_repository_install_exheres_0;

    struct ERepositoryDependenciesRewriterTest : TestCase
    {
        ERepositoryDependenciesRewriterTest() : TestCase("dependencies_rewriter") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo17"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo17/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("category/package",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());

            StringifyFormatter ff;

            erepository::DepSpecPrettyPrinter pd(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
            TEST_CHECK(id->build_dependencies_key());
            id->build_dependencies_key()->value()->root()->accept(pd);
            TEST_CHECK_STRINGIFY_EQUAL(pd, "( cat/pkg1 build: cat/pkg2 build,run: cat/pkg3 suggested: cat/pkg4 post: )");

            erepository::DepSpecPrettyPrinter pr(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
            TEST_CHECK(id->run_dependencies_key());
            id->run_dependencies_key()->value()->root()->accept(pr);
            TEST_CHECK_STRINGIFY_EQUAL(pr, "( cat/pkg1 build: build,run: cat/pkg3 suggested: cat/pkg4 post: )");

            erepository::DepSpecPrettyPrinter pp(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
            TEST_CHECK(id->post_dependencies_key());
            id->post_dependencies_key()->value()->root()->accept(pp);
            TEST_CHECK_STRINGIFY_EQUAL(pp, "( build: build,run: suggested: post: cat/pkg5 )");
        }
    } test_e_repository_dependencies_rewriter;

    struct ERepositorySymlinkRewritingTest : TestCase
    {
        ERepositorySymlinkRewritingTest() : TestCase("symlink_rewriting") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");

            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo20"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "repo20/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "symlinked_build"));
            keys->insert("root", stringify(FSEntry(stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "root")).realpath()));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "vdb"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry(stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "root")).realpath()));
            std::tr1::shared_ptr<Repository> installed_repo(VDBRepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, installed_repo);

            InstallAction action(make_named_values<InstallActionOptions>(
                        value_for<n::destination>(installed_repo),
                        value_for<n::used_this_for_config_protect>(&dummy_used_this_for_config_protect),
                        value_for<n::want_phase>(&want_all_phases)
                    ));

            const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("cat/pkg",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
            TEST_CHECK(id);

            id->perform_action(action);
            TEST_CHECK_EQUAL(FSEntry(stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "root/bar")).readlink(), "/foo");
        }
    } test_e_repository_symlink_rewriting;
}

