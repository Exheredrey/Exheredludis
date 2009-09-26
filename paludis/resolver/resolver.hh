/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_HH 1

#include <paludis/resolver/resolver-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/use_existing-fwd.hh>
#include <paludis/resolver/resolutions-fwd.hh>
#include <paludis/resolver/resolver_functions-fwd.hh>
#include <paludis/resolver/destination_types-fwd.hh>
#include <paludis/resolver/destination-fwd.hh>
#include <paludis/resolver/unsuitable_candidates-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/name.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/spec_tree-fwd.hh>
#include <paludis/filtered_generator-fwd.hh>
#include <paludis/generator-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE Resolver :
            private PrivateImplementationPattern<Resolver>
        {
            private:
                const std::tr1::shared_ptr<Resolution> _create_resolution_for_resolvent(const Resolvent &) const;
                const std::tr1::shared_ptr<Resolution> _resolution_for_resolvent(const Resolvent &, const bool create);
                const std::tr1::shared_ptr<Resolution> _resolution_for_resolvent(const Resolvent &) const;

                const std::tr1::shared_ptr<const Resolvents> _get_resolvents_for_blocker(const BlockDepSpec &) const;

                const DestinationTypes _get_destination_types_for_blocker(const BlockDepSpec &) const;

                const std::tr1::shared_ptr<const Resolvents> _get_resolvents_for(
                        const PackageDepSpec & spec,
                        const std::tr1::shared_ptr<const Reason> & reason) const;

                const DestinationTypes _get_destination_types_for(
                        const PackageDepSpec & spec,
                        const std::tr1::shared_ptr<const Reason> & reason) const;

                const std::tr1::shared_ptr<const Resolvents> _get_error_resolvents_for(
                            const PackageDepSpec & spec,
                            const std::tr1::shared_ptr<const Reason> & reason) const;

                const std::tr1::shared_ptr<ConstraintSequence> _make_constraints_from_target(
                        const Resolvent &,
                        const PackageDepSpec &,
                        const std::tr1::shared_ptr<const Reason> &) const;

                const std::tr1::shared_ptr<ConstraintSequence> _make_constraints_from_dependency(
                        const Resolvent &, const SanitisedDependency &,
                        const std::tr1::shared_ptr<const Reason> &) const;

                void _apply_resolution_constraint(const Resolvent &,
                        const std::tr1::shared_ptr<Resolution> &,
                        const std::tr1::shared_ptr<const Constraint> &);

                bool _check_constraint(const Resolvent &,
                        const std::tr1::shared_ptr<const Constraint> & constraint,
                        const std::tr1::shared_ptr<const Decision> & decision) const;

                bool _verify_new_constraint(const Resolvent &,
                        const std::tr1::shared_ptr<const Resolution> &,
                        const std::tr1::shared_ptr<const Constraint> &);

                void _made_wrong_decision(const Resolvent &,
                        const std::tr1::shared_ptr<Resolution> & resolution,
                        const std::tr1::shared_ptr<const Constraint> & constraint);

                void _suggest_restart_with(const Resolvent &,
                        const std::tr1::shared_ptr<const Resolution> & resolution,
                        const std::tr1::shared_ptr<const Constraint> & constraint,
                        const std::tr1::shared_ptr<const Decision> & decision) const PALUDIS_ATTRIBUTE((noreturn));

                const std::tr1::shared_ptr<const Constraint> _make_constraint_for_preloading(
                        const Resolvent &,
                        const std::tr1::shared_ptr<const Decision> & d,
                        const std::tr1::shared_ptr<const Constraint> & c) const;

                const std::tr1::shared_ptr<const PackageIDSequence> _find_replacing(
                        const std::tr1::shared_ptr<const PackageID> &,
                        const std::tr1::shared_ptr<const Repository> &) const;

                const std::tr1::shared_ptr<const Repository> _find_repository_for(
                        const Resolvent &,
                        const std::tr1::shared_ptr<const Resolution> &,
                        const ChangesToMakeDecision &) const;

                void _resolve_arrow(const Resolvent &, const std::tr1::shared_ptr<Resolution> &,
                        const std::tr1::shared_ptr<const Constraint> &);

                void _resolve_decide_with_dependencies();
                void _resolve_destinations();
                void _resolve_arrows();
                void _resolve_order();

                const std::tr1::shared_ptr<Destination> _make_destination_for(
                        const Resolvent & resolvent,
                        const std::tr1::shared_ptr<const Resolution> & resolution,
                        const ChangesToMakeDecision &) const;

                FilteredGenerator _make_destination_filtered_generator(const Generator &, const Resolvent & resolvent) const;

                void _decide(const Resolvent &, const std::tr1::shared_ptr<Resolution> & resolution);

                const std::tr1::shared_ptr<Decision> _try_to_find_decision_for(
                        const Resolvent &, const std::tr1::shared_ptr<const Resolution> & resolution) const;

                const std::tr1::shared_ptr<Decision> _cannot_decide_for(
                        const Resolvent &, const std::tr1::shared_ptr<const Resolution> & resolution) const;

                void _do_destination_if_necessary(const Resolvent & our_resolvent,
                        const std::tr1::shared_ptr<Resolution> & our_resolution);

                void _add_dependencies_if_necessary(const Resolvent & our_resolvent,
                        const std::tr1::shared_ptr<Resolution> & our_resolution);

                bool _care_about_dependency_spec(const Resolvent &, const std::tr1::shared_ptr<const Resolution> &,
                        const SanitisedDependency &) const;

                bool _causes_pre_arrow(const DependencyReason &) const;

                bool _can_order_now(const Resolvent &, const std::tr1::shared_ptr<const Resolution> & resolution,
                        const int ignorable_pass) const;

                void _do_order(const Resolvent &, const std::tr1::shared_ptr<Resolution> & resolution);

                void _unable_to_order_more() const PALUDIS_ATTRIBUTE((noreturn));

                const std::tr1::shared_ptr<Constraints> _initial_constraints_for(const Resolvent &) const;

                bool _same_slot(const std::tr1::shared_ptr<const PackageID> & a,
                        const std::tr1::shared_ptr<const PackageID> & b) const;

                bool _already_met(const SanitisedDependency & dep) const;

                const std::string _find_cycle(const Resolvent &, const int ignorable_pass) const;

                const std::tr1::shared_ptr<const PackageID> _find_existing_id_for(
                        const Resolvent &, const std::tr1::shared_ptr<const Resolution> &) const;
                const std::tr1::shared_ptr<const PackageIDSequence> _find_installable_id_candidates_for(
                        const Resolvent &, const std::tr1::shared_ptr<const Resolution> &,
                        const bool include_errors) const;
                const std::pair<const std::tr1::shared_ptr<const PackageID>, bool> _find_installable_id_for(
                        const Resolvent &, const std::tr1::shared_ptr<const Resolution> &) const;
                const std::pair<const std::tr1::shared_ptr<const PackageID>, bool> _find_id_for_from(
                        const Resolvent &, const std::tr1::shared_ptr<const Resolution> &,
                        const std::tr1::shared_ptr<const PackageIDSequence> &) const;

                const std::tr1::shared_ptr<const Constraints> _get_unmatching_constraints(
                        const Resolvent &, const std::tr1::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                UnsuitableCandidate _make_unsuitable_candidate(
                        const Resolvent &,
                        const std::tr1::shared_ptr<const Resolution> &,
                        const std::tr1::shared_ptr<const PackageID> &) const;

                void _need_rewrites() const;

            public:
                Resolver(
                        const Environment * const,
                        const ResolverFunctions &);
                ~Resolver();

                void add_target_with_reason(const PackageDepSpec &, const std::tr1::shared_ptr<const Reason> &);
                void add_target(const PackageDepSpec &);
                void add_target(const SetName &);

                void resolve();

                const std::tr1::shared_ptr<const ResolutionLists> resolution_lists() const PALUDIS_ATTRIBUTE((warn_unused_result));

                struct ResolutionsByResolventConstIteratorTag;
                typedef WrappedForwardIterator<ResolutionsByResolventConstIteratorTag,
                        const std::pair<const Resolvent, std::tr1::shared_ptr<Resolution> > >
                            ResolutionsByResolventConstIterator;
                ResolutionsByResolventConstIterator begin_resolutions_by_resolvent() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                ResolutionsByResolventConstIterator end_resolutions_by_resolvent() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                int find_any_score(const Resolvent &, const SanitisedDependency &) const;

                const std::tr1::shared_ptr<DependencySpecTree> rewrite_if_special(const PackageOrBlockDepSpec &,
                        const Resolvent & from) const;
        };
    }
}

#endif
