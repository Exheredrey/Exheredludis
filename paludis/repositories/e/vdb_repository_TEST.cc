/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/action.hh>
#include <paludis/choice.hh>

#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <functional>
#include <algorithm>
#include <iterator>
#include <vector>

using namespace test;
using namespace paludis;

namespace
{
    void do_uninstall(const std::shared_ptr<const PackageID> & id, const UninstallActionOptions & u)
    {
        UninstallAction a(u);
        id->perform_action(a);
    }

    std::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return std::make_shared<StandardOutputManager>();
    }

    std::string from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }

    bool ignore_nothing(const FSEntry &)
    {
        return false;
    }
}

namespace test_cases
{
    struct VDBRepositoryRepoNameTest : TestCase
    {
        VDBRepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "repo1"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "installed");
        }
    } test_vdb_repository_repo_name;

    struct VDBRepositoryHasCategoryNamedTest : TestCase
    {
        VDBRepositoryHasCategoryNamedTest() : TestCase("has category named") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "repo1"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));

            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-one")));
            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-two")));
            TEST_CHECK(! repo->has_category_named(CategoryNamePart("cat-three")));
        }
    } test_vdb_repository_has_category_named;

    struct VDBRepositoryQueryUseTest : TestCase
    {
        VDBRepositoryQueryUseTest() : TestCase("query USE") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "repo1"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> e1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

            TEST_CHECK(bool(e1->choices_key()));
            TEST_CHECK(bool(e1->choices_key()->value()));
            TEST_CHECK(bool(e1->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix("flag1"))));
            TEST_CHECK(e1->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix("flag1"))->enabled());
            TEST_CHECK(e1->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix("flag2"))->enabled());
            TEST_CHECK(! e1->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix("flag3"))->enabled());
            TEST_CHECK(e1->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix("test"))->enabled());
            TEST_CHECK(e1->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix("kernel_linux"))->enabled());
            TEST_CHECK(! e1->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix("test2")));
            TEST_CHECK(! e1->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix("kernel_freebsd")));
        }
    } test_vdb_repository_query_use;

    struct VDBRepositoryContentsTest : TestCase
    {
        VDBRepositoryContentsTest() : TestCase("CONTENTS") { }

        struct ContentsGatherer
        {
            std::string _str;

            void visit(const ContentsFileEntry & e)
            {
                _str += "file\n";
                _str += stringify(e.location_key()->value());
                _str += '\n';
            }

            void visit(const ContentsDirEntry & e)
            {
                _str += "directory\n";
                _str += stringify(e.location_key()->value());
                _str += '\n';
            }

            void visit(const ContentsSymEntry & e)
            {
                _str += "symlink\n";
                _str += stringify(e.location_key()->value());
                _str += '\n';
                _str += stringify(e.target_key()->value());
                _str += '\n';
            }

            void visit(const ContentsOtherEntry & e)
            {
                _str += "other\n";
                _str += stringify(e.location_key()->value());
                _str += '\n';
            }
        };

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "repo1"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("world", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "world-no-match-no-eol"));
            std::shared_ptr<Repository> repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> e1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
            ContentsGatherer gatherer;
            std::for_each(indirect_iterator(e1->contents_key()->value()->begin()),
                          indirect_iterator(e1->contents_key()->value()->end()),
                          accept_visitor(gatherer));
            TEST_CHECK_EQUAL(gatherer._str,
                             "directory\n/directory\n"
                             "file\n/directory/file\n"
                             "symlink\n/directory/symlink\ntarget\n"
                             "directory\n/directory with spaces\n"
                             "directory\n/directory with trailing space \n"
                             "directory\n/directory  with  consecutive  spaces\n"
                             "file\n/file with spaces\n"
                             "file\n/file  with  consecutive  spaces\n"
                             "file\n/file with  trailing   space\t \n"
                             "symlink\n/symlink\ntarget  with  consecutive  spaces\n"
                             "symlink\n/symlink with spaces\ntarget with spaces\n"
                             "symlink\n/symlink  with  consecutive  spaces\ntarget  with  consecutive  spaces\n"
                             "symlink\n/symlink\ntarget -> with -> multiple -> arrows\n"
                             "symlink\n/symlink\ntarget with trailing space \n"
                             "symlink\n/symlink\n target with leading space\n"
                             "symlink\n/symlink with trailing space \ntarget\n"
                             "other\n/fifo\n"
                             "other\n/fifo with spaces\n"
                             "other\n/fifo  with  consecutive  spaces\n"
                             "other\n/device\n"
                             "other\n/device with spaces\n"
                             "other\n/device  with  consecutive  spaces\n"
                             "other\n/miscellaneous\n"
                             "other\n/miscellaneous with spaces\n"
                             "other\n/miscellaneous  with  consecutive  spaces\n");
        }
    } vdb_repository_contents_test;

    struct VDBRepositoryDependenciesRewriterTest : TestCase
    {
        VDBRepositoryDependenciesRewriterTest() : TestCase("dependencies_rewriter") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "repo2"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("category/package",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

            StringifyFormatter ff;

            erepository::DepSpecPrettyPrinter pd(0, std::shared_ptr<const PackageID>(), ff, 0, false, false);
            TEST_CHECK(bool(id->build_dependencies_key()));
            id->build_dependencies_key()->value()->root()->accept(pd);
            TEST_CHECK_STRINGIFY_EQUAL(pd, "( cat/pkg1 build: cat/pkg2 build+run: cat/pkg3 suggestion: post: )");

            erepository::DepSpecPrettyPrinter pr(0, std::shared_ptr<const PackageID>(), ff, 0, false, false);
            TEST_CHECK(bool(id->run_dependencies_key()));
            id->run_dependencies_key()->value()->root()->accept(pr);
            TEST_CHECK_STRINGIFY_EQUAL(pr, "( cat/pkg1 build: build+run: cat/pkg3 suggestion: post: )");

            erepository::DepSpecPrettyPrinter pp(0, std::shared_ptr<const PackageID>(), ff, 0, false, false);
            TEST_CHECK(bool(id->post_dependencies_key()));
            id->post_dependencies_key()->value()->root()->accept(pp);
            TEST_CHECK_STRINGIFY_EQUAL(pp, "( build: build+run: suggestion: cat/pkg4 post: cat/pkg5 )");
        }
    } test_vdb_repository_dependencies_rewriter;

    struct PhasesTest : TestCase
    {
        const std::string eapi;

        PhasesTest(const std::string & e) :
            TestCase("phases eapi " + e),
            eapi(e)
        {
        }

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
            std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "srcrepo"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "srcrepo/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", eapi);
            keys->insert("eapi_when_unspecified", eapi);
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "repo3"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            InstallAction install_action(make_named_values<InstallActionOptions>(
                        n::destination() = vdb_repo,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::perform_uninstall() = &do_uninstall,
                        n::replacing() = std::make_shared<PackageIDSequence>(),
                        n::want_phase() = &want_all_phases
                    ));

            UninstallAction uninstall_action(make_named_values<UninstallActionOptions>(
                        n::config_protect() = "",
                        n::if_for_install_id() = make_null_shared_ptr(),
                        n::ignore_for_unmerge() = &ignore_nothing,
                        n::is_overwrite() = false,
                        n::make_output_manager() = &make_standard_output_manager
                    ));

            InfoActionOptions info_action_options(make_named_values<InfoActionOptions>(
                        n::make_output_manager() = &make_standard_output_manager
                        ));

            ConfigActionOptions config_action_options(make_named_values<ConfigActionOptions>(
                        n::make_output_manager() = &make_standard_output_manager
                        ));

            InfoAction info_action(info_action_options);
            ConfigAction config_action(config_action_options);

            {
                TestMessageSuffix suffix("install", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::srcrepo",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                TEST_CHECK(bool(id));
                id->perform_action(install_action);
            }

            vdb_repo->invalidate();

            {
                TestMessageSuffix suffix("reinstall", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::srcrepo",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                TEST_CHECK(bool(id));
                id->perform_action(install_action);
            }

            vdb_repo->invalidate();

            {
                TestMessageSuffix suffix("info", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                TEST_CHECK(bool(id));
                id->perform_action(info_action);
            }

            {
                TestMessageSuffix suffix("config", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                TEST_CHECK(bool(id));
                id->perform_action(config_action);
            }

            {
                TestMessageSuffix suffix("uninstall", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                TEST_CHECK(bool(id));
                id->perform_action(uninstall_action);
            }
        }
    } test_phases_eapi_0("0"), test_phases_eapi_1("1"), test_phases_eapi_2("2"), test_phases_eapi_3("3"),
                        test_phases_eapi_exheres_0("exheres-0");

    struct VarsTest : TestCase
    {
        const std::string eapi;

        VarsTest(const std::string & e) :
            TestCase("vars eapi " + e),
            eapi(e)
        {
        }

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
            std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "srcrepo"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "srcrepo/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", eapi);
            keys->insert("eapi_when_unspecified", eapi);
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "repo3"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            InstallAction install_action(make_named_values<InstallActionOptions>(
                        n::destination() = vdb_repo,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::perform_uninstall() = &do_uninstall,
                        n::replacing() = std::make_shared<PackageIDSequence>(),
                        n::want_phase() = &want_all_phases
                    ));

            UninstallAction uninstall_action(make_named_values<UninstallActionOptions>(
                        n::config_protect() = "",
                        n::if_for_install_id() = make_null_shared_ptr(),
                        n::ignore_for_unmerge() = &ignore_nothing,
                        n::is_overwrite() = false,
                        n::make_output_manager() = &make_standard_output_manager
                    ));

            InfoActionOptions info_action_options(make_named_values<InfoActionOptions>(
                        n::make_output_manager() = &make_standard_output_manager
                        ));

            ConfigActionOptions config_action_options(make_named_values<ConfigActionOptions>(
                        n::make_output_manager() = &make_standard_output_manager
                        ));

            InfoAction info_action(info_action_options);
            ConfigAction config_action(config_action_options);

            {
                TestMessageSuffix suffix("vars", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::srcrepo",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                TEST_CHECK(bool(id));
                id->perform_action(install_action);
            }

            vdb_repo->invalidate();

            {
                TestMessageSuffix suffix("reinstall", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::srcrepo",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                TEST_CHECK(bool(id));
                id->perform_action(install_action);
            }

            vdb_repo->invalidate();

            {
                TestMessageSuffix suffix("info", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                TEST_CHECK(bool(id));
                id->perform_action(info_action);
            }

            {
                TestMessageSuffix suffix("config", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                TEST_CHECK(bool(id));
                id->perform_action(config_action);
            }

            {
                TestMessageSuffix suffix("uninstall", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                TEST_CHECK(bool(id));
                id->perform_action(uninstall_action);
            }
        }
    } test_vdb_vars_eapi_0("0"), test_vdb_vars_eapi_1("1"), test_vdb_vars_eapi_2("2"),
                      test_vdb_vars_eapi_exheres_0("exheres-0");

    struct NamesCacheIncrementalTest : TestCase
    {
        FSEntry names_cache;

        NamesCacheIncrementalTest() :
            TestCase("names cache incremental"),
            names_cache(stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "namesincrtest/.cache/names/installed"))
        {
        }

        bool repeatable() const
        {
            return false;
        }

        unsigned max_run_time() const
        {
            return 3000;
        }

        void install(const Environment & env,
                const std::shared_ptr<Repository> & vdb_repo,
                const std::string & chosen_one,
                const std::string & victim) const
        {
            std::shared_ptr<PackageIDSequence> replacing(new PackageIDSequence);
            if (! victim.empty())
                replacing->push_back(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec(victim,
                                &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
            InstallAction install_action(make_named_values<InstallActionOptions>(
                        n::destination() = vdb_repo,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::perform_uninstall() = &do_uninstall,
                        n::replacing() = replacing,
                        n::want_phase() = &want_all_phases
                    ));
            (*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec(chosen_one,
                                &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->perform_action(install_action);
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "namesincrtest_src"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "namesincrtest_src/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "vdb");
            keys->insert("names_cache", stringify(names_cache.dirname()));
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "namesincrtest"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            UninstallAction uninstall_action(make_named_values<UninstallActionOptions>(
                        n::config_protect() = "",
                        n::if_for_install_id() = make_null_shared_ptr(),
                        n::ignore_for_unmerge() = &ignore_nothing,
                        n::is_overwrite() = false,
                        n::make_output_manager() = &make_standard_output_manager
                    ));

            {
                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 0U);
            }

            {
                TestMessageSuffix suffix("install", true);
                install(env, vdb_repo, "=cat1/pkg1-1::namesincrtest_src", "");
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("reinstall", true);
                install(env, vdb_repo, "=cat1/pkg1-1::namesincrtest_src", "");
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("upgrade", true);
                install(env, vdb_repo, "=cat1/pkg1-1.1::namesincrtest_src", "=cat1/pkg1-1::installed");
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("downgrade", true);
                install(env, vdb_repo, "=cat1/pkg1-1::namesincrtest_src", "=cat1/pkg1-1.1::installed");
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("new slot", true);
                install(env, vdb_repo, "=cat1/pkg1-2::namesincrtest_src", "");
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("remove other slot", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-2::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("new package", true);
                install(env, vdb_repo, "=cat1/pkg2-1::namesincrtest_src", "");
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 2U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(cache_contents.back().basename(), "pkg2");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg2"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("remove other package", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg2-1::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("new category", true);
                install(env, vdb_repo, "=cat2/pkg1-1::namesincrtest_src", "");
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\ncat2\n");
            }

            {
                TestMessageSuffix suffix("remove other category", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat2/pkg1-1::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("uninstall", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 0U);
            }

            {
                TestMessageSuffix suffix("install paludis-1", true);
                install(env, vdb_repo, "=cat3/pkg1-1::namesincrtest_src", "");
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat3\n");
            }

            {
                TestMessageSuffix suffix("upgrade paludis-1", true);
                install(env, vdb_repo, "=cat3/pkg1-2::namesincrtest_src", "=cat3/pkg1-1::installed");
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat3\n");
            }
        }

        void read_cache(std::vector<FSEntry> & vec)
        {
            using namespace std::placeholders;
            std::remove_copy_if(DirIterator(names_cache, DirIteratorOptions() + dio_include_dotfiles),
                                DirIterator(), std::back_inserter(vec),
                                std::bind(&std::equal_to<std::string>::operator(),
                                          std::equal_to<std::string>(),
                                          "_VERSION_", std::bind(&FSEntry::basename, _1)));
        }

        std::string read_file(const FSEntry & f)
        {
            SafeIFStream s(f);
            std::stringstream ss;
            std::copy(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>(),
                      std::ostreambuf_iterator<char>(ss));
            return ss.str();
        }
    } test_names_cache_incremental;

    struct ProvidesCacheTest : TestCase
    {
        FSEntry provides_cache;

        ProvidesCacheTest() :
            TestCase("provides cache"),
            provides_cache(stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "providestest/.cache/provides"))
        {
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", stringify(provides_cache));
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "providestest"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            TEST_CHECK(! provides_cache.exists());

            {
                std::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence> seq(vdb_repo->provides_interface()->provided_packages());

                for (RepositoryProvidesInterface::ProvidesSequence::ConstIterator s(seq->begin()), s_end(seq->end()) ;
                        s != s_end ; ++s)
                    TestMessageSuffix x(stringify(s->virtual_name()) + " by " + stringify(*s->provided_by()), true);

                TEST_CHECK_EQUAL(std::distance(seq->begin(), seq->end()), 5);

                RepositoryProvidesInterface::ProvidesSequence::ConstIterator it(seq->begin());
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg1-1:1::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg1-2:2::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg2-1:1::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/bar");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg2-1:1::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/bar");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg2-2:2::installed");
            }

            vdb_repo->regenerate_cache();
            TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/foo\ncat1/pkg1 2 virtual/foo\ncat1/pkg2 1 virtual/foo virtual/bar\ncat1/pkg2 2 virtual/bar\n");
            vdb_repo->invalidate();

            {
                std::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence> seq(vdb_repo->provides_interface()->provided_packages());
                TEST_CHECK_EQUAL(std::distance(seq->begin(), seq->end()), 5);

                RepositoryProvidesInterface::ProvidesSequence::ConstIterator it(seq->begin());
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg1-1:1::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg1-2:2::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg2-1:1::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/bar");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg2-1:1::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/bar");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg2-2:2::installed");
            }
        }

        std::string read_file(const FSEntry & f)
        {
            SafeIFStream s(f);
            std::stringstream ss;
            std::copy(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>(),
                      std::ostreambuf_iterator<char>(ss));
            return ss.str();
        }
    } test_provides_cache;

    struct ProvidesCacheIncrementalTest : TestCase
    {
        FSEntry provides_cache;

        ProvidesCacheIncrementalTest() :
            TestCase("provides cache incremental"),
            provides_cache(stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "providesincrtest/.cache/provides"))
        {
        }

        bool repeatable() const
        {
            return false;
        }

        unsigned max_run_time() const
        {
            return 3000;
        }

        void install(const Environment & env,
                const std::shared_ptr<Repository> & vdb_repo,
                const std::string & chosen_one,
                const std::string & victim) const
        {
            std::shared_ptr<PackageIDSequence> replacing(new PackageIDSequence);
            if (! victim.empty())
                replacing->push_back(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec(victim,
                                &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
            InstallAction install_action(make_named_values<InstallActionOptions>(
                        n::destination() = vdb_repo,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::perform_uninstall() = &do_uninstall,
                        n::replacing() = replacing,
                        n::want_phase() = &want_all_phases
                    ));
            (*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec(chosen_one,
                                &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->perform_action(install_action);
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "providesincrtest_src1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "providesincrtest_src1/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> repo1(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo1);

            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "providesincrtest_src2"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "providesincrtest_src1/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> repo2(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(2, repo2);

            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", stringify(provides_cache));
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "providesincrtest"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            UninstallAction uninstall_action(make_named_values<UninstallActionOptions>(
                        n::config_protect() = "",
                        n::if_for_install_id() = make_null_shared_ptr(),
                        n::ignore_for_unmerge() = &ignore_nothing,
                        n::is_overwrite() = false,
                        n::make_output_manager() = &make_standard_output_manager
                    ));

            TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\n");

            {
                TestMessageSuffix suffix("install", true);
                install(env, vdb_repo, "=cat1/pkg1-1::providesincrtest_src1", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("reinstall", true);
                install(env, vdb_repo, "=cat1/pkg1-1::providesincrtest_src1", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("upgrade", true);
                install(env, vdb_repo, "=cat1/pkg1-1.1::providesincrtest_src1", "=cat1/pkg1-1::installed");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1.1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("reinstall equivalent", true);
                install(env, vdb_repo, "=cat1/pkg1-1.1::providesincrtest_src2", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1.1-r0 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("downgrade", true);
                install(env, vdb_repo, "=cat1/pkg1-1::providesincrtest_src1", "=cat1/pkg1-1.1::installed");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("reinstall different PROVIDE", true);
                install(env, vdb_repo, "=cat1/pkg1-1::providesincrtest_src2", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\n");
            }

            {
                TestMessageSuffix suffix("new slot", true);
                install(env, vdb_repo, "=cat1/pkg1-2::providesincrtest_src1", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\ncat1/pkg1 2 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("remove other slot", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-2::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\n");
            }

            {
                TestMessageSuffix suffix("new package", true);
                install(env, vdb_repo, "=cat1/pkg2-1::providesincrtest_src1", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\ncat1/pkg2 1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("remove other package", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg2-1::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\n");
            }

            {
                TestMessageSuffix suffix("uninstall", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\n");
            }

            {
                TestMessageSuffix suffix("install paludis-1", true);
                install(env, vdb_repo, "=cat2/pkg1-1::providesincrtest_src1", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat2/pkg1 1 virtual/moo\n");
            }

            {
                TestMessageSuffix suffix("upgrade paludis-1", true);
                install(env, vdb_repo, "=cat2/pkg1-2::providesincrtest_src1", "=cat2/pkg1-1::installed");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat2/pkg1 2 virtual/moo\n");
            }
        }

        std::string read_file(const FSEntry & f)
        {
            SafeIFStream s(f);
            std::stringstream ss;
            std::copy(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>(),
                      std::ostreambuf_iterator<char>(ss));
            return ss.str();
        }
    } test_provides_cache_incremental;

    struct ReinstallTest : TestCase
    {
        ReinstallTest() : TestCase("reinstall") { }

        bool repeatable() const
        {
            return false;
        }

        unsigned max_run_time() const
        {
            return 3000;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "reinstalltest_src1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "reinstalltest_src1/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> repo1(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo1);

            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "reinstalltest_src2"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "reinstalltest_src1/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> repo2(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(2, repo2);

            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "reinstalltest"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            InstallAction install_action(make_named_values<InstallActionOptions>(
                        n::destination() = vdb_repo,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::perform_uninstall() = &do_uninstall,
                        n::replacing() = std::make_shared<PackageIDSequence>(),
                        n::want_phase() = &want_all_phases
                    ));

            TEST_CHECK(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"))->empty());

            {
                TestMessageSuffix suffix("install", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1::reinstalltest_src1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1::installed");
            }

            {
                TestMessageSuffix suffix("reinstall", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1::reinstalltest_src1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1::installed");
            }

            {
                TestMessageSuffix suffix("reinstall equivalent", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1::reinstalltest_src2",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1-r0::installed");
            }
        }
    } reinstall_test;

    struct PkgPostinstPhaseOrderingTest : TestCase
    {
        PkgPostinstPhaseOrderingTest() : TestCase("pkg_postinst phase ordering") { }

        bool repeatable() const
        {
            return false;
        }

        unsigned max_run_time() const
        {
            return 3000;
        }

        void install(const Environment & env,
                const std::shared_ptr<Repository> & vdb_repo,
                const std::string & chosen_one,
                const std::string & victim) const
        {
            std::shared_ptr<PackageIDSequence> replacing(new PackageIDSequence);
            if (! victim.empty())
                replacing->push_back(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec(victim,
                                &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
            InstallAction install_action(make_named_values<InstallActionOptions>(
                        n::destination() = vdb_repo,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::perform_uninstall() = &do_uninstall,
                        n::replacing() = replacing,
                        n::want_phase() = &want_all_phases
                    ));
            (*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec(chosen_one,
                                &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin())->perform_action(install_action);
        }

        void run()
        {
            TestEnvironment env(FSEntry(stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "root")).realpath());
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "postinsttest_src1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "postinsttest_src1/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> repo1(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo1);

            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "postinsttest"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            TEST_CHECK(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"))->empty());

            {
                TestMessageSuffix suffix("install eapi 1", true);

                ::setenv("VDB_REPOSITORY_TEST_RMDIR", "x-install-eapi-1", 1);
                install(env, vdb_repo, "=cat/pkg-0::postinsttest", "");
                vdb_repo->invalidate();

                std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-0::installed");
            }

            {
                TestMessageSuffix suffix("reinstall eapi 1", true);

                ::setenv("VDB_REPOSITORY_TEST_RMDIR", "0", 1);
                install(env, vdb_repo, "=cat/pkg-0::postinsttest", "");
                vdb_repo->invalidate();

                std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-0::installed");
            }

            {
                TestMessageSuffix suffix("upgrade eapi 1 -> 1", true);

                ::setenv("VDB_REPOSITORY_TEST_RMDIR", "x-upgrade-eapi-1-1", 1);
                install(env, vdb_repo, "=cat/pkg-0.1::postinsttest", "=cat/pkg-0::installed");
                vdb_repo->invalidate();

                std::shared_ptr<const PackageIDSequence> ids2(vdb_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids2->begin()), indirect_iterator(ids2->end()), " "), "cat/pkg-0.1::installed");
            }

            {
                TestMessageSuffix suffix("upgrade eapi 1 -> paludis-1", true);

                ::setenv("VDB_REPOSITORY_TEST_RMDIR", "x-upgrade-eapi-1-paludis-1", 1);
                install(env, vdb_repo, "=cat/pkg-1::postinsttest", "=cat/pkg-0.1::installed");
                vdb_repo->invalidate();

                std::shared_ptr<const PackageIDSequence> ids(env[selection::AllVersionsSorted(generator::Package(
                                QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1::installed");
            }

            {
                TestMessageSuffix suffix("reinstall eapi paludis-1", true);

                ::setenv("VDB_REPOSITORY_TEST_RMDIR", "x-reinstall-eapi-paludis-1", 1);
                install(env, vdb_repo, "=cat/pkg-1::postinsttest", "");
                vdb_repo->invalidate();

                std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1::installed");
            }

            {
                TestMessageSuffix suffix("upgrade eapi paludis-1 -> paludis-1", true);

                ::setenv("VDB_REPOSITORY_TEST_RMDIR", "1.1", 1);
                install(env, vdb_repo, "=cat/pkg-1.1::postinsttest", "=cat/pkg-1::installed");
                vdb_repo->invalidate();

                std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1.1::installed");
            }

            {
                TestMessageSuffix suffix("new slot", true);

                ::setenv("VDB_REPOSITORY_TEST_RMDIR", "x-new-slot", 1);
                install(env, vdb_repo, "=cat/pkg-2::postinsttest", "");
                vdb_repo->invalidate();

                std::shared_ptr<const PackageIDSequence> ids(env[selection::AllVersionsSorted(generator::Package(
                                QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1.1::installed cat/pkg-2::installed");
            }

            {
                TestMessageSuffix suffix("downgrade eapi paludis-1 -> 1", true);

                ::setenv("VDB_REPOSITORY_TEST_RMDIR", "x-downgrade-eapi-paludis-1-1", 1);
                install(env, vdb_repo, "=cat/pkg-0::postinsttest", "=cat/pkg-1.1::installed");
                vdb_repo->invalidate();

                std::shared_ptr<const PackageIDSequence> ids2(env[selection::AllVersionsSorted(generator::Package(
                                QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
                TEST_CHECK_EQUAL(join(indirect_iterator(ids2->begin()), indirect_iterator(ids2->end()), " "), "cat/pkg-0::installed cat/pkg-2::installed");
            }
        }
    } pkg_postinst_phase_ordering_test;
}

