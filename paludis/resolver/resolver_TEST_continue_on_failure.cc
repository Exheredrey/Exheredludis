/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/job_list.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/job_requirements.hh>
#include <paludis/resolver/make_uninstall_blocker.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/join.hh>

#include <paludis/user_dep_spec.hh>
#include <paludis/repository_factory.hh>

#include <paludis/resolver/resolver_test.hh>

#include <list>
#include <functional>
#include <algorithm>
#include <map>

using namespace paludis;
using namespace paludis::resolver;
using namespace paludis::resolver::resolver_test;

namespace
{
    struct ResolverContinueOnFailureTestCase :
        ResolverTestCase
    {
        std::shared_ptr<ResolverTestData> data;

        void SetUp()
        {
            data = std::make_shared<ResolverTestData>("continue_on_failure", "exheres-0", "exheres");
        }

        void TearDown()
        {
            data.reset();
        }
    };

    std::string
    stringify_req(const JobRequirement & r)
    {
        std::stringstream result;
        result << r.job_number();
        if (r.required_if()[jri_require_for_satisfied])
            result << " satisfied";
        if (r.required_if()[jri_require_for_independent])
            result << " independent";
        if (r.required_if()[jri_require_always])
            result << " always";
        return result.str();
    }
}

namespace
{
    template <bool direct_dep_installed_>
    struct TestContinueOnFailure :
        ResolverContinueOnFailureTestCase
    {
        void common_test_code()
        {
            if (direct_dep_installed_)
                data->install("continue-on-failure", "direct-dep", "0");
            data->install("continue-on-failure", "unchanged-dep", "1")->build_dependencies_key()->set_from_string("continue-on-failure/indirect-dep");

            data->get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_if_same);
            data->get_use_existing_nothing_helper.set_use_existing_for_targets(ue_if_same);

            std::shared_ptr<const Resolved> resolved(data->get_resolved("continue-on-failure/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("continue-on-failure/direct-dep"))
                        .change(QualifiedPackageName("continue-on-failure/indirect-dep"))
                        .change(QualifiedPackageName("continue-on-failure/target"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished())
                    );

            EXPECT_EQ(6, resolved->job_lists()->execute_job_list()->length());

            const InstallJob * const direct_dep_job(visitor_cast<const InstallJob>(**resolved->job_lists()->execute_job_list()->fetch(1)));
            ASSERT_TRUE(direct_dep_job);
            EXPECT_EQ("0 satisfied independent always",
                    join(direct_dep_job->requirements()->begin(), direct_dep_job->requirements()->end(), ", ", stringify_req));

            const InstallJob * const indirect_dep_job(visitor_cast<const InstallJob>(**resolved->job_lists()->execute_job_list()->fetch(3)));
            ASSERT_TRUE(indirect_dep_job);
            EXPECT_EQ("2 satisfied independent always",
                    join(indirect_dep_job->requirements()->begin(), indirect_dep_job->requirements()->end(), ", ", stringify_req));

            const InstallJob * const target_job(visitor_cast<const InstallJob>(**resolved->job_lists()->execute_job_list()->fetch(5)));
            ASSERT_TRUE(target_job);
            if (direct_dep_installed_)
                EXPECT_EQ("4 satisfied independent always, 3 independent, 1 independent",
                        join(target_job->requirements()->begin(), target_job->requirements()->end(), ", ", stringify_req));
            else
                EXPECT_EQ("4 satisfied independent always, 1 satisfied, 3 independent, 1 independent",
                        join(target_job->requirements()->begin(), target_job->requirements()->end(), ", ", stringify_req));
        }
    };
}

typedef TestContinueOnFailure<false> ContinueOnFailureF;
typedef TestContinueOnFailure<true> ContinueOnFailureT;

TEST_F(ContinueOnFailureF, Works) { common_test_code(); }
TEST_F(ContinueOnFailureT, Works) { common_test_code(); }

TEST_F(ResolverContinueOnFailureTestCase, Uninstall)
{
    data->install("continue-on-failure-uninstall", "dep-of-dep", "1")->build_dependencies_key()->set_from_string("");
    data->install("continue-on-failure-uninstall", "dep", "1")->build_dependencies_key()->set_from_string("continue-on-failure-uninstall/dep-of-dep");
    data->install("continue-on-failure-uninstall", "target", "1")->build_dependencies_key()->set_from_string("continue-on-failure-uninstall/dep");
    data->install("continue-on-failure-uninstall", "needs-target", "1")->build_dependencies_key()->set_from_string("continue-on-failure-uninstall/target");

    data->get_constraints_for_purge_helper.add_purge_spec(parse_user_package_dep_spec("continue-on-failure-uninstall/dep-of-dep", &data->env, { }));
    data->get_constraints_for_purge_helper.add_purge_spec(parse_user_package_dep_spec("continue-on-failure-uninstall/dep", &data->env, { }));
    data->get_constraints_for_purge_helper.add_purge_spec(parse_user_package_dep_spec("continue-on-failure-uninstall/target", &data->env, { }));

    data->allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("continue-on-failure-uninstall/needs-target", &data->env, { }));

    data->remove_if_dependent_helper.add_remove_if_dependent_spec(parse_user_package_dep_spec("continue-on-failure-uninstall/needs-target", &data->env, { }));

    std::shared_ptr<const Resolved> resolved(data->get_resolved(make_uninstall_blocker(
                    parse_user_package_dep_spec("continue-on-failure-uninstall/target", &data->env, { }))));

    check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .remove(QualifiedPackageName("continue-on-failure-uninstall/needs-target"))
                .remove(QualifiedPackageName("continue-on-failure-uninstall/target"))
                .remove(QualifiedPackageName("continue-on-failure-uninstall/dep"))
                .remove(QualifiedPackageName("continue-on-failure-uninstall/dep-of-dep"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );

    EXPECT_EQ(4, resolved->job_lists()->execute_job_list()->length());

    const UninstallJob * const needs_target_job(visitor_cast<const UninstallJob>(**resolved->job_lists()->execute_job_list()->fetch(0)));
    ASSERT_TRUE(needs_target_job);
    EXPECT_EQ("", join(needs_target_job->requirements()->begin(), needs_target_job->requirements()->end(), ", ", stringify_req));

    const UninstallJob * const target_job(visitor_cast<const UninstallJob>(**resolved->job_lists()->execute_job_list()->fetch(1)));
    ASSERT_TRUE(target_job);
    EXPECT_EQ("0 satisfied", join(target_job->requirements()->begin(), target_job->requirements()->end(), ", ", stringify_req));

    const UninstallJob * const dep_job(visitor_cast<const UninstallJob>(**resolved->job_lists()->execute_job_list()->fetch(2)));
    ASSERT_TRUE(dep_job);
    EXPECT_EQ("1 satisfied", join(dep_job->requirements()->begin(), dep_job->requirements()->end(), ", ", stringify_req));

    const UninstallJob * const dep_of_dep_job(visitor_cast<const UninstallJob>(**resolved->job_lists()->execute_job_list()->fetch(3)));
    ASSERT_TRUE(dep_of_dep_job);
    EXPECT_EQ("2 satisfied", join(dep_of_dep_job->requirements()->begin(), dep_of_dep_job->requirements()->end(), ", ", stringify_req));
}

