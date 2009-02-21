/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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
#include <paludis/repositories/e/exndbam_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/action.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace
{
    std::tr1::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return make_shared_ptr(new StandardOutputManager);
    }

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
    struct ExndbamRepositoryRepoNameTest : TestCase
    {
        ExndbamRepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "exndbam");
            keys->insert("location", stringify(FSEntry::cwd() / "exndbam_repository_TEST_dir" / "repo1"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "exndbam_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ExndbamRepository::ExndbamRepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "installed");
        }
    } test_exndbam_repository_repo_name;

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

        void run()
        {
            TestEnvironment env(FSEntry("exndbam_repository_TEST_dir/root").realpath());
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "exndbam_repository_TEST_dir" / "postinsttest_src1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "exndbam_repository_TEST_dir" / "postinsttest_src1/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "exndbam_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "exndbam_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("exndbam_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<Repository> repo1(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo1);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "exndbam");
            keys->insert("location", stringify(FSEntry::cwd() / "exndbam_repository_TEST_dir" / "postinsttest"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "exndbam_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("exndbam_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<Repository> exndbam_repo(ExndbamRepository::ExndbamRepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(0, exndbam_repo);

            InstallAction install_action(make_named_values<InstallActionOptions>(
                        value_for<n::destination>(exndbam_repo),
                        value_for<n::make_output_manager>(&make_standard_output_manager),
                        value_for<n::replacing>(make_shared_ptr(new PackageIDSequence)),
                        value_for<n::used_this_for_config_protect>(&dummy_used_this_for_config_protect),
                        value_for<n::want_phase>(&want_all_phases)
                    ));

            UninstallAction uninstall_action(make_named_values<UninstallActionOptions>(
                        value_for<n::config_protect>(""),
                        value_for<n::make_output_manager>(&make_standard_output_manager)
                    ));

            TEST_CHECK(exndbam_repo->package_ids(QualifiedPackageName("cat/pkg"))->empty());

            {
                TestMessageSuffix suffix("install eapi 1", true);

                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-0::postinsttest",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                id->perform_action(install_action);
                exndbam_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids(exndbam_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-0::installed");
            }

            {
                TestMessageSuffix suffix("reinstall eapi 1", true);

                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-0::postinsttest",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                id->perform_action(install_action);
                exndbam_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids(exndbam_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-0::installed");
            }

            {
                TestMessageSuffix suffix("upgrade eapi 1 -> 1", true);

                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-0.1::postinsttest",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                id->perform_action(install_action);
                exndbam_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids(env[selection::AllVersionsSorted(generator::Package(
                                QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-0::installed cat/pkg-0.1::installed");

                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-0::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                inst_id->perform_action(uninstall_action);
                exndbam_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids2(exndbam_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids2->begin()), indirect_iterator(ids2->end()), " "), "cat/pkg-0.1::installed");
            }

            {
                TestMessageSuffix suffix("upgrade eapi 1 -> paludis-1", true);

                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1::postinsttest",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                id->perform_action(install_action);
                exndbam_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids(env[selection::AllVersionsSorted(generator::Package(
                                QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1::installed");
            }

            {
                TestMessageSuffix suffix("reinstall eapi paludis-1", true);

                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1::postinsttest",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                id->perform_action(install_action);
                exndbam_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids(exndbam_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1::installed");
            }

            {
                TestMessageSuffix suffix("upgrade eapi paludis-1 -> paludis-1", true);

                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1.1::postinsttest",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                id->perform_action(install_action);
                exndbam_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids(exndbam_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1.1::installed");
            }

            {
                TestMessageSuffix suffix("new slot", true);

                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-2::postinsttest",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                id->perform_action(install_action);
                exndbam_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids(env[selection::AllVersionsSorted(generator::Package(
                                QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1.1::installed cat/pkg-2::installed");
            }

            {
                TestMessageSuffix suffix("downgrade eapi paludis-1 -> 1", true);

                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-0::postinsttest",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                id->perform_action(install_action);
                exndbam_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids(env[selection::AllVersionsSorted(generator::Package(
                                QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-0::installed cat/pkg-1.1::installed cat/pkg-2::installed");

                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1.1::installed",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
                inst_id->perform_action(uninstall_action);
                exndbam_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids2(env[selection::AllVersionsSorted(generator::Package(
                                QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
                TEST_CHECK_EQUAL(join(indirect_iterator(ids2->begin()), indirect_iterator(ids2->end()), " "), "cat/pkg-0::installed cat/pkg-2::installed");
            }
        }
    } pkg_postinst_phase_ordering_test;
}

