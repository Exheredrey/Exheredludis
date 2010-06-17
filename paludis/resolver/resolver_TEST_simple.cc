/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/repository_factory.hh>
#include <paludis/package_database.hh>

#include <paludis/resolver/resolver_test.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>

#include <list>
#include <tr1/functional>
#include <algorithm>
#include <map>

using namespace paludis;
using namespace paludis::resolver;
using namespace paludis::resolver::resolver_test;
using namespace test;

namespace
{
    struct ResolverSimpleTestCase : ResolverTestCase
    {
        ResolverSimpleTestCase(const std::string & s) :
            ResolverTestCase("simple", s, "exheres-0", "exheres")
        {
        }
    };
}

namespace test_cases
{
    struct TestNoDeps : ResolverSimpleTestCase
    {
        TestNoDeps() : ResolverSimpleTestCase("no-deps") { }

        void run()
        {
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("no-deps/target"));
            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("no-deps/target"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished())
                    );
        }
    } test_no_deps;

    struct TestBuildDeps : ResolverSimpleTestCase
    {
        TestBuildDeps() : ResolverSimpleTestCase("build-deps") { }

        void run()
        {
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("build-deps/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("build-deps/a-dep"))
                        .change(QualifiedPackageName("build-deps/b-dep"))
                        .change(QualifiedPackageName("build-deps/z-dep"))
                        .change(QualifiedPackageName("build-deps/target"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished())
                    );
        }
    } test_build_deps;

    struct TestRunDeps : ResolverSimpleTestCase
    {
        TestRunDeps() : ResolverSimpleTestCase("run-deps") { }

        void run()
        {
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("run-deps/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("run-deps/a-dep"))
                        .change(QualifiedPackageName("run-deps/b-dep"))
                        .change(QualifiedPackageName("run-deps/z-dep"))
                        .change(QualifiedPackageName("run-deps/target"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished())
                    );
        }
    } test_run_deps;

    struct TestPostDeps : ResolverSimpleTestCase
    {
        TestPostDeps() : ResolverSimpleTestCase("post-deps") { }

        void run()
        {
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("post-deps/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("post-deps/a-dep"))
                        .change(QualifiedPackageName("post-deps/b-dep"))
                        .change(QualifiedPackageName("post-deps/target"))
                        .change(QualifiedPackageName("post-deps/z-dep"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished())
                    );
        }
    } test_post_deps;
}

