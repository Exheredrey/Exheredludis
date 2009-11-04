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
#include <paludis/standard_output_manager.hh>
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
    void cannot_uninstall(const std::tr1::shared_ptr<const PackageID> & id, const UninstallActionOptions &)
    {
        if (id)
            throw InternalError(PALUDIS_HERE, "cannot uninstall");
    }

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

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }
}

namespace test_cases
{
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
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_0_dir" / "repo"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_0_dir" / "repo/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_0_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_0_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(
                        make_named_values<FakeInstalledRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::name>(RepositoryName("installed")),
                            value_for<n::suitable_destination>(true),
                            value_for<n::supports_uninstall>(true)
                            )));
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
                        value_for<n::make_output_manager>(&make_standard_output_manager),
                        value_for<n::perform_uninstall>(&cannot_uninstall),
                        value_for<n::replacing>(make_shared_ptr(new PackageIDSequence)),
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
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
            }

            {
                TestMessageSuffix suffix("in-subshell die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-subshell-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
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
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
            }

            {
                TestMessageSuffix suffix("econf die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/econf-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
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
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
            }

            {
                TestMessageSuffix suffix("einstall die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/einstall-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
            }

            {
                TestMessageSuffix suffix("keepdir die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/keepdir-die",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
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
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
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
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
            }

            {
                TestMessageSuffix suffix("econf source 0", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "0");
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
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

            {
                TestMessageSuffix suffix("econf disable dependency tracking", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-disable-dependency-tracking-0",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "0");
                id->perform_action(action);
            }
        }
    } test_e_repository_install_eapi_0;
}

