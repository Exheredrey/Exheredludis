/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>

#include <paludis/standard_output_manager.hh>
#include <paludis/action.hh>
#include <paludis/choice.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>

#include <gtest/gtest.h>

#include <algorithm>

using namespace paludis;

namespace
{
    const auto exndbam_repository_TEST_dir =
        FSPath::cwd() / "exndbam_repository_TEST_dir";
    const auto exndbam_repository_TEST_root =
        exndbam_repository_TEST_dir / "root";

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

    void install(const Environment & env,
            const std::shared_ptr<Repository> & exndbam_repo,
            const std::string & chosen_one,
            const std::string & victim)
    {
        std::shared_ptr<PackageIDSequence> replacing(std::make_shared<PackageIDSequence>());
        if (! victim.empty())
            replacing->push_back(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec(victim,
                            &env, { })), nullptr, { }))]->begin());
        InstallAction install_action(make_named_values<InstallActionOptions>(
                    n::destination() = exndbam_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::perform_uninstall() = &do_uninstall,
                    n::replacing() = replacing,
                    n::want_phase() = &want_all_phases
                ));
        (*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec(chosen_one,
                            &env, { })), nullptr, { }))]->begin())->perform_action(install_action);
    }

    void
    uninstall(const Environment & env, const std::string & package)
    {
        UninstallAction action(make_named_values<UninstallActionOptions>(
                    n::config_protect() = "",
                    n::if_for_install_id() = nullptr,
                    n::ignore_for_unmerge() = [](const FSPath &) { return false; },
                    n::is_overwrite() = false,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::override_contents() = nullptr,
                    n::want_phase() = &want_all_phases
                    ));

        auto spec(parse_user_package_dep_spec(package, &env, { }));
        auto matches(generator::Matches(spec, nullptr, { }));
        auto id(*env[selection::RequireExactlyOne(matches)]->begin());

        id->perform_action(action);
    }

    std::shared_ptr<Repository>
    make_exndbam_repository(Environment & env, const FSPath & root,
                            const std::string & name)
    {
        using namespace std::placeholders;

        auto keys(std::make_shared<Map<std::string, std::string>>());

        keys->insert("format", "exndbam");
        keys->insert("location", stringify(root / name));
        keys->insert("builddir", stringify(root / "build"));

        keys->insert("root", stringify(root / "root"));

        return ExndbamRepository::repository_factory_create(&env,
                                                            std::bind(from_keys,
                                                                      keys, _1));
    }

    std::shared_ptr<Repository>
    make_exheres_0_repository(Environment & env, const FSPath & root,
                              const std::string & name)
    {
        using namespace std::placeholders;

        auto keys = std::make_shared<Map<std::string, std::string>>();

        keys->insert("eapi_when_unknown", "exheres-0");
        keys->insert("eapi_when_unspecified", "exheres-0");
        keys->insert("format", "e");
        keys->insert("layout", "exheres");
        keys->insert("location", stringify(root / name));
        keys->insert("names_cache", "/var/empty");
        keys->insert("profile_eapi", "exheres-0");
        keys->insert("profiles", stringify(root / name / "profiles" / "profile"));
        keys->insert("root", stringify(root / "root"));

        keys->insert("builddir", stringify(root / "build"));
        keys->insert("distdir", stringify(root / "distdir"));

        return ERepository::repository_factory_create(&env, std::bind(from_keys,
                                                                      keys, _1));
    }

    bool
    recursive_image_matches(const FSPath & root, const FSPath & directory,
                            std::vector<FSPath> & contents)
    {
        for (FSIterator dentry(directory, { fsio_include_dotfiles }), invalid;
                dentry != invalid; ++dentry)
        {
            const FSStat fstat(*dentry);
            const auto entry(dentry->strip_leading(root));

            auto expected = std::find(std::begin(contents), std::end(contents),
                                      entry);

            if (expected == std::end(contents))
                return false;

            contents.erase(expected);

            if (fstat.is_directory())
                if (! recursive_image_matches(root, *dentry, contents))
                    return false;
        }

        return true;
    }

    bool
    image_matches(const FSPath & root, const std::vector<FSPath> & contents)
    {
        std::vector<FSPath> unseen(contents);
        return recursive_image_matches(root, root, unseen) && !unseen.size();
    }
}

TEST(ExndbamRepository, RepoName)
{
    TestEnvironment env;
    auto repo(make_exndbam_repository(env, exndbam_repository_TEST_dir, "repo1"));
    EXPECT_EQ("installed", stringify(repo->name()));
}

TEST(ExndbamRepository, PhaseOrdering)
{
    TestEnvironment env(exndbam_repository_TEST_root);
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(exndbam_repository_TEST_dir / "postinsttest_src1"));
    keys->insert("profiles", stringify(exndbam_repository_TEST_dir / "postinsttest_src1/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(exndbam_repository_TEST_dir / "distdir"));
    keys->insert("builddir", stringify(exndbam_repository_TEST_dir / "build"));
    keys->insert("root", stringify(exndbam_repository_TEST_dir / "root"));
    std::shared_ptr<Repository> repo1(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo1);

    auto exndbam_repo(make_exndbam_repository(env, exndbam_repository_TEST_dir,
                                              "postinsttest"));
    env.add_repository(0, exndbam_repo);

    EXPECT_TRUE(exndbam_repo->package_ids(QualifiedPackageName("cat/pkg"), { })->empty());

    {
        install(env, exndbam_repo, "=cat/pkg-0::postinsttest", "");
        exndbam_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(exndbam_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-0::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        install(env, exndbam_repo, "=cat/pkg-0::postinsttest", "");
        exndbam_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(exndbam_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-0::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        install(env, exndbam_repo, "=cat/pkg-0.1::postinsttest", "=cat/pkg-0::installed");
        exndbam_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids2(exndbam_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-0.1::installed", join(indirect_iterator(ids2->begin()), indirect_iterator(ids2->end()), " "));
    }

    {
        install(env, exndbam_repo, "=cat/pkg-1::postinsttest", "=cat/pkg-0.1::installed");
        exndbam_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(env[selection::AllVersionsSorted(generator::Package(
                        QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
        EXPECT_EQ("cat/pkg-1::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        install(env, exndbam_repo, "=cat/pkg-1::postinsttest", "");
        exndbam_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(exndbam_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-1::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        install(env, exndbam_repo, "=cat/pkg-1.1::postinsttest", "=cat/pkg-1::installed");
        exndbam_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(exndbam_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-1.1::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        install(env, exndbam_repo, "=cat/pkg-2::postinsttest", "");
        exndbam_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(env[selection::AllVersionsSorted(generator::Package(
                        QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
        EXPECT_EQ("cat/pkg-1.1::installed cat/pkg-2::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        install(env, exndbam_repo, "=cat/pkg-0::postinsttest", "=cat/pkg-1.1::installed");
        exndbam_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids2(env[selection::AllVersionsSorted(generator::Package(
                        QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
        EXPECT_EQ("cat/pkg-0::installed cat/pkg-2::installed", join(indirect_iterator(ids2->begin()), indirect_iterator(ids2->end()), " "));
    }
}

TEST(ExndbamRepository, UnlistedPartsFails)
{
    const auto partitioned(QualifiedPackageName("category/partitioned"));

    TestEnvironment env(exndbam_repository_TEST_root);

    auto parts(make_exheres_0_repository(env, exndbam_repository_TEST_dir,
                                         "parts"));
    auto installed(make_exndbam_repository(env, exndbam_repository_TEST_dir,
                                           "installed"));
    env.add_repository(0, installed);
    env.add_repository(1, parts);

    EXPECT_TRUE(installed->package_ids(partitioned, { })->empty());
    EXPECT_TRUE(!parts->package_ids(partitioned, { })->empty());

    ASSERT_THROW(install(env, installed, "=category/partitioned-0::parts", ""),
                 paludis::ActionFailedError);

    EXPECT_TRUE(installed->package_ids(partitioned, { })->empty());
}

TEST(ExndbamRepository, SelectivePartsCoreOnly)
{
    const auto partitioned(QualifiedPackageName("category/partitioned"));

    TestEnvironment env(exndbam_repository_TEST_root);

    auto parts(make_exheres_0_repository(env, exndbam_repository_TEST_dir,
                                         "parts"));
    auto installed(make_exndbam_repository(env, exndbam_repository_TEST_dir,
                                           "installed"));
    env.add_repository(0, installed);
    env.add_repository(1, parts);

    EXPECT_TRUE(installed->package_ids(partitioned, { })->empty());
    EXPECT_TRUE(!parts->package_ids(partitioned, { })->empty());

    install(env, installed, "=category/partitioned-1::parts", "");
    installed->invalidate();

    EXPECT_TRUE(!installed->package_ids(partitioned, { })->empty());
    EXPECT_TRUE(image_matches(exndbam_repository_TEST_root, {
                FSPath("/etc"),
                FSPath("/usr"),
                FSPath("/usr/share"),
                FSPath("/usr/share/man"),
                FSPath("/usr/share/man/man1"),
                FSPath("/usr/share/man/man1/expart.1")
                }));

    uninstall(env, "=category/partitioned-1::installed");
}

TEST(ExndbamRepository, SelectivePartsCoreAndSelectedParts)
{
    const auto partitioned(QualifiedPackageName("category/partitioned"));

    TestEnvironment env(exndbam_repository_TEST_root);
    env.set_want_choice_enabled(ChoicePrefixName("parts"),
            UnprefixedChoiceName("binaries"),
            Tribool("true"));

    auto parts(make_exheres_0_repository(env, exndbam_repository_TEST_dir,
                                         "parts"));
    auto installed(make_exndbam_repository(env, exndbam_repository_TEST_dir,
                                           "installed"));
    env.add_repository(0, installed);
    env.add_repository(1, parts);

    EXPECT_TRUE(installed->package_ids(partitioned, { })->empty());
    EXPECT_TRUE(!parts->package_ids(partitioned, { })->empty());

    install(env, installed, "=category/partitioned-1::parts", "");
    installed->invalidate();

    EXPECT_TRUE(!installed->package_ids(partitioned, { })->empty());
    EXPECT_TRUE(image_matches(exndbam_repository_TEST_root, {
                FSPath("/etc"),
                FSPath("/usr"),
                FSPath("/usr/bin"),
                FSPath("/usr/bin/binary"),
                FSPath("/usr/share"),
                FSPath("/usr/share/man"),
                FSPath("/usr/share/man/man1"),
                FSPath("/usr/share/man/man1/expart.1")
                }));

    uninstall(env, "=category/partitioned-1::installed");
}

