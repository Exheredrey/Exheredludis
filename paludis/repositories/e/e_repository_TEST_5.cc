/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/repositories/e/spec_tree_pretty_printer.hh>

#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/system.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/stringify.hh>

#include <paludis/standard_output_manager.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/repository_factory.hh>
#include <paludis/choice.hh>

#include <functional>
#include <set>
#include <string>

#include "config.h"

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    void cannot_uninstall(const std::shared_ptr<const PackageID> & id, const UninstallActionOptions &)
    {
        if (id)
            throw InternalError(PALUDIS_HERE, "cannot uninstall");
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
}

TEST(ERepository, InstallEAPI5)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<FakeInstalledRepository> installed_repo(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("installed"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));
    env.add_repository(2, installed_repo);

    InstallAction action(make_named_values<InstallActionOptions>(
                n::destination() = installed_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &cannot_uninstall,
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = &want_all_phases
            ));

    PretendAction pretend_action(make_named_values<PretendActionOptions>(
                n::destination() = installed_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::replacing() = std::make_shared<PackageIDSequence>()
                ));

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/econf-disable-silent-rules-5",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/strict-use-5",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/strict-use-fail-5",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/strict-use-injection-5",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }
}

TEST(ERepository, RequiredUse)
{
    FSPath root(FSPath::cwd() / "e_repository_TEST_5_dir" / "root");

    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<FakeInstalledRepository> installed_repo(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("installed"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));
    env.add_repository(2, installed_repo);

    {
        PretendAction pretend_action(make_named_values<PretendActionOptions>(
                    n::destination() = installed_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::replacing() = std::make_shared<PackageIDSequence>()
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/required-use-at-most-one-none-5::test-repo",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(pretend_action);
        ASSERT_TRUE(! pretend_action.failed());
    }

    {
        PretendAction pretend_action(make_named_values<PretendActionOptions>(
                    n::destination() = installed_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::replacing() = std::make_shared<PackageIDSequence>()
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/required-use-at-most-one-one-5::test-repo",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(pretend_action);
        ASSERT_TRUE(! pretend_action.failed());
    }

    {
        PretendAction pretend_action(make_named_values<PretendActionOptions>(
                    n::destination() = installed_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::replacing() = std::make_shared<PackageIDSequence>()
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/required-use-at-most-one-two-5::test-repo",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(pretend_action);
        ASSERT_TRUE(pretend_action.failed());
    }
}

TEST(ERepository, UseX)
{
    FSPath root(FSPath::cwd() / "e_repository_TEST_5_dir" / "root");

    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<FakeInstalledRepository> installed_repo(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("installed"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));
    env.add_repository(2, installed_repo);

    {
        PretendAction pretend_action(make_named_values<PretendActionOptions>(
                    n::destination() = installed_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::replacing() = std::make_shared<PackageIDSequence>()
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/usex-5::test-repo",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(pretend_action);
        ASSERT_TRUE(! pretend_action.failed());
    }
}

TEST(ERepository, NewStdin)
{
    FSPath root(FSPath::cwd() / "e_repository_TEST_5_dir" / "root");

    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<Map<std::string, std::string> > v_keys(std::make_shared<Map<std::string, std::string>>());
    v_keys->insert("format", "vdb");
    v_keys->insert("names_cache", "/var/empty");
    v_keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "vdb"));
    v_keys->insert("root", stringify(root));
    std::shared_ptr<Repository> v_repo(VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, v_keys, std::placeholders::_1)));
    env.add_repository(1, v_repo);

    {
        InstallAction action(make_named_values<InstallActionOptions>(
                    n::destination() = v_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::perform_uninstall() = &cannot_uninstall,
                    n::replacing() = std::make_shared<PackageIDSequence>(),
                    n::want_phase() = &want_all_phases
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/new-stdin-5::test-repo",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }
}

TEST(ERepository, Doheader)
{
    FSPath root(FSPath::cwd() / "e_repository_TEST_5_dir" / "root");

    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<Map<std::string, std::string> > v_keys(std::make_shared<Map<std::string, std::string>>());
    v_keys->insert("format", "vdb");
    v_keys->insert("names_cache", "/var/empty");
    v_keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "vdb"));
    v_keys->insert("root", stringify(root));
    std::shared_ptr<Repository> v_repo(VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, v_keys, std::placeholders::_1)));
    env.add_repository(1, v_repo);

    {
        InstallAction action(make_named_values<InstallActionOptions>(
                    n::destination() = v_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::perform_uninstall() = &cannot_uninstall,
                    n::replacing() = std::make_shared<PackageIDSequence>(),
                    n::want_phase() = &want_all_phases
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/doheader-5::test-repo",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);

        const std::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/doheader-dies-5::test-repo",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id2));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id2->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id2->perform_action(action), ActionFailedError);
    }
}

