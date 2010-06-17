/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
 * Copyright (c) 2009 David Leverton
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
#include <paludis/util/map-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/tribool.hh>
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
    struct ResolverAnyTestCase : ResolverTestCase
    {
        ResolverAnyTestCase(const std::string & s) :
            ResolverTestCase("any", s, "exheres-0", "exheres")
        {
        }
    };
}

namespace test_cases
{
    struct TestEmptyAlternative : ResolverAnyTestCase
    {
        TestEmptyAlternative() : ResolverAnyTestCase("empty alternative") { }

        void run()
        {
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("test/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("test/target"))
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
        }
    } test_empty_alternative;

    struct TestEmptyAlternativeWithUpgrade : ResolverAnyTestCase
    {
        TestEmptyAlternativeWithUpgrade() :
            ResolverAnyTestCase("empty alternative with upgrade")
        {
            install("test", "dep", "2");
        }

        void run()
        {
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("test/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("test/dep"))
                        .change(QualifiedPackageName("test/target"))
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
        }
    } test_empty_alternative_with_upgrade;

    struct TestEmptyAlternativeWithUntakenUpgrade : ResolverAnyTestCase
    {
        TestEmptyAlternativeWithUntakenUpgrade() :
            ResolverAnyTestCase("empty alternative with untaken upgrade")
        {
            install("test", "dep", "1");
        }

        void run()
        {
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("test/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("test/target"))
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
        }
    } test_empty_alternative_with_untaken_upgrade;

    struct TestEmptyPreferences : ResolverAnyTestCase
    {
        const Tribool a, b;

        TestEmptyPreferences(const Tribool aa, const Tribool bb) :
            ResolverAnyTestCase("empty preferences " + stringify(aa) + " " + stringify(bb)),
            a(aa),
            b(bb)
        {
            if (! a.is_indeterminate())
                prefer_or_avoid_names->insert(QualifiedPackageName("preferences/dep-a"), a.is_true());
            if (! b.is_indeterminate())
                prefer_or_avoid_names->insert(QualifiedPackageName("preferences/dep-b"), b.is_true());
        }

        void run()
        {
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("preferences/target"));

            std::tr1::shared_ptr<DecisionChecks> checks;

            if (a.is_true())
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("preferences/dep-a"))
                        .change(QualifiedPackageName("preferences/target"))
                        .finished());
            }
            else if (b.is_true())
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("preferences/dep-b"))
                        .change(QualifiedPackageName("preferences/target"))
                        .finished());
            }
            else if (a.is_false())
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("preferences/dep-middle"))
                        .change(QualifiedPackageName("preferences/target"))
                        .finished());
            }
            else
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("preferences/dep-a"))
                        .change(QualifiedPackageName("preferences/target"))
                        .finished());
            }

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = checks,
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
        }
    } test_empty_preferences_tt(true, true),
           test_empty_preferences_ti(true, indeterminate), test_empty_preferences_tf(true, false),
           test_empty_preferences_it(indeterminate, true), test_empty_preferences_ii(indeterminate, indeterminate),
           test_empty_preferences_if(indeterminate, false), test_empty_preferences_ft(false, true),
           test_empty_preferences_fi(false, indeterminate), test_empty_preferences_ff(false, false);
}

