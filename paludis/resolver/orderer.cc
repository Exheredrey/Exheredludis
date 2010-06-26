/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/resolver/orderer.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/decisions.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/nag.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/strongly_connected_component.hh>
#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/job_list.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/job_requirements.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/orderer_notes.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/join.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/environment.hh>
#include <paludis/notifier_callback.hh>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <list>

using namespace paludis;
using namespace paludis::resolver;

typedef std::tr1::unordered_map<NAGIndex, std::tr1::shared_ptr<const ChangeOrRemoveDecision>, Hash<NAGIndex> > ChangeOrRemoveIndices;
typedef std::tr1::unordered_map<NAGIndex, JobNumber, Hash<NAGIndex> > InstallJobNumbers;
typedef std::tr1::unordered_map<Resolvent, JobNumber, Hash<Resolvent> > FetchJobNumbers;

namespace paludis
{
    template <>
    struct Implementation<Orderer>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<Resolved> resolved;
        ChangeOrRemoveIndices change_or_remove_indices;
        FetchJobNumbers fetch_job_numbers;
        InstallJobNumbers install_job_numbers;

        Implementation(
                const Environment * const e,
                const std::tr1::shared_ptr<Resolved> & r) :
            env(e),
            resolved(r)
        {
        }
    };
}

Orderer::Orderer(
        const Environment * const e,
        const std::tr1::shared_ptr<Resolved> & r) :
    PrivateImplementationPattern<Orderer>(new Implementation<Orderer>(e, r))
{
}

Orderer::~Orderer()
{
}

namespace
{
    typedef std::tr1::unordered_set<Resolvent, Hash<Resolvent> > ResolventsSet;

    struct DecisionDispatcher
    {
        const std::tr1::shared_ptr<Resolved> resolved;
        ResolventsSet & ignore_dependencies_from_resolvents;
        ChangeOrRemoveIndices & change_or_remove_indices;
        const Resolvent resolvent;
        const std::tr1::shared_ptr<const Decision> decision;

        DecisionDispatcher(
                const std::tr1::shared_ptr<Resolved> & r,
                ResolventsSet & i,
                ChangeOrRemoveIndices & c,
                const Resolvent & v,
                const std::tr1::shared_ptr<const Decision> & d) :
            resolved(r),
            ignore_dependencies_from_resolvents(i),
            change_or_remove_indices(c),
            resolvent(v),
            decision(d)
        {
        }

        bool visit(const UnableToMakeDecision &)
        {
            if (decision->taken())
                resolved->taken_unable_to_make_decisions()->push_back(
                        std::tr1::static_pointer_cast<const UnableToMakeDecision>(decision));
            else
                resolved->untaken_unable_to_make_decisions()->push_back(
                        std::tr1::static_pointer_cast<const UnableToMakeDecision>(decision));

            ignore_dependencies_from_resolvents.insert(resolvent);

            return false;
        }

        bool visit(const NothingNoChangeDecision &)
        {
            resolved->nag()->add_node(make_named_values<NAGIndex>(
                        n::resolvent() = resolvent,
                        n::role() = nir_done
                        ));
            return true;
        }

        bool visit(const ExistingNoChangeDecision &)
        {
            resolved->nag()->add_node(make_named_values<NAGIndex>(
                        n::resolvent() = resolvent,
                        n::role() = nir_done
                        ));
            return true;
        }

        bool visit(const ChangesToMakeDecision &)
        {
            if (decision->taken())
            {
                NAGIndex fetched_index(make_named_values<NAGIndex>(
                            n::resolvent() = resolvent,
                            n::role() = nir_fetched
                            ));
                resolved->nag()->add_node(fetched_index);
                change_or_remove_indices.insert(std::make_pair(fetched_index,
                            std::tr1::static_pointer_cast<const ChangeOrRemoveDecision>(decision)));

                NAGIndex done_index(make_named_values<NAGIndex>(
                            n::resolvent() = resolvent,
                            n::role() = nir_done
                            ));
                resolved->nag()->add_node(done_index);
                change_or_remove_indices.insert(std::make_pair(done_index,
                            std::tr1::static_pointer_cast<const ChangeOrRemoveDecision>(decision)));

                resolved->nag()->add_edge(done_index, fetched_index,
                        make_named_values<NAGEdgeProperties>(
                            n::build() = true,
                            n::build_all_met() = false,
                            n::run() = false,
                            n::run_all_met() = true
                            ));

                return true;
            }
            else
            {
                resolved->untaken_change_or_remove_decisions()->push_back(
                        std::tr1::static_pointer_cast<const ChangesToMakeDecision>(decision));
                return false;
            }
        }

        bool visit(const RemoveDecision &)
        {
            if (decision->taken())
            {
                NAGIndex index(make_named_values<NAGIndex>(
                            n::resolvent() = resolvent,
                            n::role() = nir_done
                            ));
                resolved->nag()->add_node(index);
                change_or_remove_indices.insert(std::make_pair(index,
                            std::tr1::static_pointer_cast<const ChangeOrRemoveDecision>(decision)));
                return true;
            }
            else
            {
                resolved->untaken_change_or_remove_decisions()->push_back(
                        std::tr1::static_pointer_cast<const ChangeOrRemoveDecision>(decision));
                return false;
            }
        }

        bool visit(const BreakDecision & d)
        {
            if (d.required_confirmations_if_any())
                resolved->taken_unconfirmed_decisions()->push_back(
                        std::tr1::static_pointer_cast<const BreakDecision>(decision));
            return false;
        }
    };

    struct LabelsClassifier
    {
        bool build;
        bool run;
        bool post;
        bool fetch;

        LabelsClassifier() :
            build(false),
            run(false),
            post(false),
            fetch(false)
        {
        }

        void visit(const DependenciesBuildLabel &)
        {
            build = true;
        }

        void visit(const DependenciesInstallLabel &)
        {
            build = true;
        }

        void visit(const DependenciesFetchLabel &)
        {
            fetch = true;
        }

        void visit(const DependenciesRunLabel &)
        {
            run = true;
        }

        void visit(const DependenciesTestLabel &)
        {
            build = true;
        }

        void visit(const DependenciesPostLabel &)
        {
            post = true;
        }

        void visit(const DependenciesSuggestionLabel &)
        {
            post = true;
        }

        void visit(const DependenciesRecommendationLabel &)
        {
            post = true;
        }

        void visit(const DependenciesCompileAgainstLabel &)
        {
            build = true;
        }
    };

    struct EdgesFromReasonVisitor
    {
        const std::tr1::shared_ptr<NAG> nag;
        const ResolventsSet & ignore_dependencies_from_resolvents;
        const Resolvent resolvent;

        EdgesFromReasonVisitor(
                const std::tr1::shared_ptr<NAG> & n,
                const ResolventsSet & i,
                const Resolvent & v) :
            nag(n),
            ignore_dependencies_from_resolvents(i),
            resolvent(v)
        {
        }

        void visit(const DependencyReason & r)
        {
            /* we may be constrained by a dep from a package that was changed
             * from a non error decision to an unable to make decision */
            if (ignore_dependencies_from_resolvents.end() != ignore_dependencies_from_resolvents.find(r.from_resolvent()))
                return;

            /* what sort of dep are we? */
            LabelsClassifier classifier;
            for (DependenciesLabelSequence::ConstIterator l(r.sanitised_dependency().active_dependency_labels()->begin()),
                    l_end(r.sanitised_dependency().active_dependency_labels()->end()) ;
                    l != l_end ; ++l)
                (*l)->accept(classifier);

            if (classifier.build || classifier.run || classifier.fetch)
            {
                bool arrow(true);
                if (r.sanitised_dependency().spec().if_block())
                    if (! r.sanitised_dependency().spec().if_block()->strong())
                        arrow = false;

                if (arrow)
                {
                    NAGIndex from(make_named_values<NAGIndex>(
                                n::resolvent() = r.from_resolvent(),
                                n::role() = classifier.fetch ? nir_fetched : nir_done
                                ));

                    NAGIndex to(make_named_values<NAGIndex>(
                                n::resolvent() = resolvent,
                                n::role() = nir_done
                                ));

                    nag->add_edge(from, to,
                            make_named_values<NAGEdgeProperties>(
                                n::build() = classifier.build || classifier.fetch,
                                n::build_all_met() = r.already_met() || ! (classifier.build || classifier.fetch),
                                n::run() = classifier.run,
                                n::run_all_met() = r.already_met() || ! classifier.run
                                ));
                }
            }
            else if (classifier.post)
            {
                /* we won't add a backwards edge, since most post deps dep upon
                 * the thing requiring them anyway */
                // nag->add_edge(resolvent, r.from_resolvent());
            }
            else
                throw InternalError(PALUDIS_HERE, "No classification");
        }

        void visit(const SetReason & r)
        {
            if (r.reason_for_set())
                r.reason_for_set()->accept(*this);
        }

        void visit(const PresetReason &)
        {
        }

        void visit(const DependentReason &)
        {
        }

        void visit(const TargetReason &)
        {
        }

        void visit(const WasUsedByReason &)
        {
        }
    };

    bool no_build_dependencies(
            const Set<NAGIndex> & indices,
            const NAG & nag)
    {
        for (Set<NAGIndex>::ConstIterator r(indices.begin()), r_end(indices.end()) ;
                r != r_end ; ++r)
            for (NAG::EdgesFromConstIterator e(nag.begin_edges_from(*r)), e_end(nag.end_edges_from(*r)) ;
                    e != e_end ; ++e)
                if (e->second.build())
                    return false;

        return true;
    }
}

void
Orderer::resolve()
{
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Nodifying Decisions"));

    ResolventsSet ignore_dependencies_from_resolvents, ignore_edges_from_resolvents;
    for (ResolutionsByResolvent::ConstIterator r(_imp->resolved->resolutions_by_resolvent()->begin()),
            r_end(_imp->resolved->resolutions_by_resolvent()->end()) ;
            r != r_end ; ++r)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        DecisionDispatcher decision_dispatcher(
                _imp->resolved,
                ignore_dependencies_from_resolvents,
                _imp->change_or_remove_indices,
                (*r)->resolvent(),
                (*r)->decision());
        if (! (*r)->decision()->accept_returning<bool>(decision_dispatcher))
            ignore_edges_from_resolvents.insert((*r)->resolvent());
    }

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Building NAG Edges"));

    for (ResolutionsByResolvent::ConstIterator r(_imp->resolved->resolutions_by_resolvent()->begin()),
            r_end(_imp->resolved->resolutions_by_resolvent()->end()) ;
            r != r_end ; ++r)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        if (ignore_dependencies_from_resolvents.end() != ignore_edges_from_resolvents.find((*r)->resolvent()))
            continue;

        EdgesFromReasonVisitor edges_from_reason_visitor(_imp->resolved->nag(), ignore_dependencies_from_resolvents, (*r)->resolvent());
        for (Constraints::ConstIterator c((*r)->constraints()->begin()),
                c_end((*r)->constraints()->end()) ;
                c != c_end ; ++c)
            (*c)->reason()->accept(edges_from_reason_visitor);
    }

    _imp->resolved->nag()->verify_edges();

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Finding NAG SCCs"));
    const std::tr1::shared_ptr<const SortedStronglyConnectedComponents> ssccs(_imp->resolved->nag()->sorted_strongly_connected_components());

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Ordering SCCs"));
    for (SortedStronglyConnectedComponents::ConstIterator scc(ssccs->begin()), scc_end(ssccs->end()) ;
            scc != scc_end ; ++scc)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        /* some (or none, or all) of the nodes in our SCC are change or remove
         * nodes. this matters for cycle resolution. we identify them now, even
         * though our scc might just contain a single install, rather than
         * adding in extra useless code for the special easy case. */
        typedef std::tr1::unordered_set<NAGIndex, Hash<NAGIndex> > ChangesInSCC;
        ChangesInSCC changes_in_scc;

        for (Set<NAGIndex>::ConstIterator r(scc->nodes()->begin()), r_end(scc->nodes()->end()) ;
                r != r_end ; ++r)
            if (_imp->change_or_remove_indices.end() != _imp->change_or_remove_indices.find(*r))
                changes_in_scc.insert(*r);

        if (changes_in_scc.empty())
        {
            /* two or more installed packages are codependent, but we don't have
             * to care */
        }
        else if (changes_in_scc.size() == 1)
        {
            /* there's only one real package in the component, so there's no
             * need to try anything clever */
            _check_self_deps_and_schedule(*changes_in_scc.begin(),
                    _imp->change_or_remove_indices.find(*changes_in_scc.begin())->second,
                    make_shared_copy(make_named_values<OrdererNotes>(
                            n::cycle_breaking() = ""
                            )));
        }
        else
        {
            /* whoop de doo. what do our SCCs look like if we only count change
             * or remove nodes? */
            NAG scc_nag;
            for (ChangesInSCC::const_iterator r(changes_in_scc.begin()), r_end(changes_in_scc.end()) ;
                    r != r_end ; ++r)
            {
                scc_nag.add_node(*r);
                /* we only need edges inside our SCC, and only those to other
                 * change or remove nodes */
                for (NAG::EdgesFromConstIterator e(_imp->resolved->nag()->begin_edges_from(*r)), e_end(_imp->resolved->nag()->end_edges_from(*r)) ;
                        e != e_end ; ++e)
                    if (changes_in_scc.end() != changes_in_scc.find(e->first))
                        scc_nag.add_edge(*r, e->first, e->second);
            }

            scc_nag.verify_edges();

            /* now we try again, hopefully with lots of small SCCs now */
            const std::tr1::shared_ptr<const SortedStronglyConnectedComponents> sub_ssccs(scc_nag.sorted_strongly_connected_components());
            _order_sub_ssccs(scc_nag, *scc, sub_ssccs, true);
        }
    }
}

namespace
{
    std::string nice_index(const NAGIndex & x)
    {
        return stringify(x.resolvent().package()) + stringify(x.resolvent().slot());
    }
}

void
Orderer::_order_sub_ssccs(
        const NAG & scc_nag,
        const StronglyConnectedComponent & top_scc,
        const std::tr1::shared_ptr<const SortedStronglyConnectedComponents> & sub_ssccs,
        const bool can_recurse)
{
    for (SortedStronglyConnectedComponents::ConstIterator sub_scc(sub_ssccs->begin()), sub_scc_end(sub_ssccs->end()) ;
            sub_scc != sub_scc_end ; ++sub_scc)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        if (sub_scc->nodes()->size() == 1)
        {
            /* yay. it's all on its own. */
            _check_self_deps_and_schedule(*sub_scc->nodes()->begin(),
                    _imp->change_or_remove_indices.find(*sub_scc->nodes()->begin())->second,
                    make_shared_copy(make_named_values<OrdererNotes>(
                            n::cycle_breaking() = (can_recurse ?
                                "In dependency cycle with existing packages: " + join(scc_nag.begin_nodes(), scc_nag.end_nodes(), ", ", nice_index) :
                                "In dependency cycle with: " + join(top_scc.nodes()->begin(), top_scc.nodes()->end(), ", ", nice_index))
                            )));
        }
        else if (no_build_dependencies(*sub_scc->nodes(), scc_nag))
        {
            /* what's that, timmy? we have directly codependent nodes?
             * well i'm jolly glad that's because they're run
             * dependency cycles which we can order however we like! */
            for (Set<NAGIndex>::ConstIterator r(sub_scc->nodes()->begin()), r_end(sub_scc->nodes()->end()) ;
                    r != r_end ; ++r)
                _check_self_deps_and_schedule(*r,
                        _imp->change_or_remove_indices.find(*r)->second,
                        make_shared_copy(make_named_values<OrdererNotes>(
                                n::cycle_breaking() = "In run dependency cycle with: " + join(
                                    sub_scc->nodes()->begin(), sub_scc->nodes()->end(), ", ", nice_index) + (can_recurse ?
                                    " in dependency cycle with " + join(top_scc.nodes()->begin(), top_scc.nodes()->end(), ", ", nice_index) : "")
                                )));
        }
        else if (can_recurse)
        {
            /* no, at least one of the deps is a build dep. let's try
             * this whole mess again, except without any edges for
             * dependencies that're already met */
            NAG scc_nag_without_met_deps;
            for (Set<NAGIndex>::ConstIterator r(sub_scc->nodes()->begin()), r_end(sub_scc->nodes()->end()) ;
                    r != r_end ; ++r)
            {
                scc_nag_without_met_deps.add_node(*r);
                for (NAG::EdgesFromConstIterator e(scc_nag.begin_edges_from(*r)), e_end(scc_nag.end_edges_from(*r)) ;
                        e != e_end ; ++e)
                    if ((! e->second.build_all_met()) || (! e->second.run_all_met()))
                        scc_nag_without_met_deps.add_edge(*r, e->first, make_named_values<NAGEdgeProperties>(
                                    n::build() = e->second.build() && ! e->second.build_all_met(),
                                    n::build_all_met() = e->second.build_all_met(),
                                    n::run() = e->second.run() && ! e->second.run_all_met(),
                                    n::run_all_met() = e->second.run_all_met()
                                    ));
            }

            scc_nag_without_met_deps.verify_edges();

            const std::tr1::shared_ptr<const SortedStronglyConnectedComponents> sub_ssccs_without_met_deps(
                    scc_nag_without_met_deps.sorted_strongly_connected_components());
            _order_sub_ssccs(scc_nag_without_met_deps, top_scc, sub_ssccs_without_met_deps, false);
        }
        else
        {
            for (Set<NAGIndex>::ConstIterator r(sub_scc->nodes()->begin()), r_end(sub_scc->nodes()->end()) ;
                    r != r_end ; ++r)
                _imp->resolved->taken_unorderable_decisions()->push_back(
                        _imp->change_or_remove_indices.find(*r)->second,
                        make_shared_copy(make_named_values<OrdererNotes>(
                                n::cycle_breaking() = "In unsolvable cycle with " + join(
                                    top_scc.nodes()->begin(), top_scc.nodes()->end(), ", ", nice_index))));
        }
    }
}

namespace
{
    typedef std::tr1::unordered_set<NAGIndex, Hash<NAGIndex> > RecursedRequirements;

    void populate_requirements(
            const std::tr1::shared_ptr<const NAG> & nag,
            const InstallJobNumbers & install_job_numbers,
            const NAGIndex & index,
            const std::tr1::shared_ptr<JobRequirements> & requirements,
            const bool recursing,
            RecursedRequirements & recursed)
    {
        if (! recursing)
            for (NAG::EdgesFromConstIterator e(nag->begin_edges_from(index)),
                    e_end(nag->end_edges_from(index)) ;
                    e != e_end ; ++e)
            {
                if ((! e->second.build_all_met()) || (! e->second.run_all_met()))
                {
                    InstallJobNumbers::const_iterator n(install_job_numbers.find(e->first));
                    if (n != install_job_numbers.end())
                        requirements->push_back(make_named_values<JobRequirement>(
                                    n::job_number() = n->second,
                                    n::required_if() = JobRequirementIfs() + jri_require_for_satisfied
                                    ));
                }
            }

        if (recursed.insert(index).second)
            for (NAG::EdgesFromConstIterator e(nag->begin_edges_from(index)),
                    e_end(nag->end_edges_from(index)) ;
                    e != e_end ; ++e)
            {
                InstallJobNumbers::const_iterator n(install_job_numbers.find(e->first));
                if (n != install_job_numbers.end())
                    requirements->push_back(make_named_values<JobRequirement>(
                                n::job_number() = n->second,
                                n::required_if() = JobRequirementIfs() + jri_require_for_independent
                                ));
                populate_requirements(nag, install_job_numbers, e->first, requirements, true, recursed);
            }
    }
}

void
Orderer::_check_self_deps_and_schedule(
        const NAGIndex & index,
        const std::tr1::shared_ptr<const ChangeOrRemoveDecision> & d,
        const std::tr1::shared_ptr<OrdererNotes> & n)
{
    /* do we dep directly upon ourself? */
    bool direct_self_dep(false), self_dep_is_met(true), self_dep_is_not_build(true);
    for (NAG::EdgesFromConstIterator e(_imp->resolved->nag()->begin_edges_from(index)),
            e_end(_imp->resolved->nag()->end_edges_from(index)) ;
            e != e_end ; ++e)
    {
        if (e->first == index)
        {
            direct_self_dep = true;
            self_dep_is_met = self_dep_is_met && e->second.build_all_met() && e->second.run_all_met();
            self_dep_is_not_build = self_dep_is_not_build && ! e->second.build();
        }
    }

    if (direct_self_dep)
    {
        if (! n->cycle_breaking().empty())
            n->cycle_breaking().append("; ");

        if (self_dep_is_met)
            n->cycle_breaking().append("Self dependent (already met)");
        else if (self_dep_is_not_build)
            n->cycle_breaking().append("Self dependent (runtime only)");
        else
            n->cycle_breaking().append("Self dependent (unsolvable)");
    }

    if (direct_self_dep && ! self_dep_is_met && ! self_dep_is_not_build)
    {
        _imp->resolved->taken_unorderable_decisions()->push_back(
                _imp->change_or_remove_indices.find(index)->second,
                n);
    }
    else
        _schedule(index, d, n);
}

namespace
{
    struct ExtraScheduler
    {
        const std::tr1::shared_ptr<const Resolved> resolved;
        FetchJobNumbers & fetch_job_numbers;
        InstallJobNumbers & install_job_numbers;
        const NAGIndex index;

        ExtraScheduler(
                const std::tr1::shared_ptr<const Resolved> & r,
                FetchJobNumbers & f,
                InstallJobNumbers & i,
                const NAGIndex & v) :
            resolved(r),
            fetch_job_numbers(f),
            install_job_numbers(i),
            index(v)
        {
        }

        void visit(const ChangesToMakeDecision & changes_to_make_decision) const
        {
            switch (index.role())
            {
                case nir_done:
                    {
                        FetchJobNumbers::const_iterator fetch_job_n(fetch_job_numbers.find(index.resolvent()));
                        if (fetch_job_n == fetch_job_numbers.end())
                            throw InternalError(PALUDIS_HERE, "haven't scheduled the fetch for " + stringify(index.resolvent()) + " yet");

                        resolved->job_lists()->pretend_job_list()->append(make_shared_ptr(new PretendJob(
                                        changes_to_make_decision.origin_id()->uniquely_identifying_spec())));

                        const std::tr1::shared_ptr<JobRequirements> requirements(new JobRequirements);
                        requirements->push_back(make_named_values<JobRequirement>(
                                    n::job_number() = fetch_job_n->second,
                                    n::required_if() = JobRequirementIfs() + jri_require_for_satisfied + jri_require_for_independent + jri_require_always
                                    ));

                        RecursedRequirements recursed;
                        populate_requirements(
                                resolved->nag(),
                                install_job_numbers,
                                index,
                                requirements,
                                false,
                                recursed
                                );

                        const std::tr1::shared_ptr<Sequence<PackageDepSpec> > replacing(new Sequence<PackageDepSpec>);
                        for (PackageIDSequence::ConstIterator i(changes_to_make_decision.destination()->replacing()->begin()),
                                i_end(changes_to_make_decision.destination()->replacing()->end()) ;
                                i != i_end ; ++i)
                            replacing->push_back((*i)->uniquely_identifying_spec());

                        JobNumber install_job_n(resolved->job_lists()->execute_job_list()->append(make_shared_ptr(new InstallJob(
                                            requirements,
                                            changes_to_make_decision.origin_id()->uniquely_identifying_spec(),
                                            changes_to_make_decision.destination()->repository(),
                                            changes_to_make_decision.resolvent().destination_type(),
                                            replacing
                                            ))));

                        install_job_numbers.insert(std::make_pair(index, install_job_n));
                    }
                    return;

                case nir_fetched:
                    {
                        JobNumber fetch_job_n(resolved->job_lists()->execute_job_list()->append(make_shared_ptr(new FetchJob(
                                            make_shared_ptr(new JobRequirements),
                                            changes_to_make_decision.origin_id()->uniquely_identifying_spec()))));
                        fetch_job_numbers.insert(std::make_pair(index.resolvent(), fetch_job_n));
                    }
                    return;

                case last_nir:
                    break;
            }

            throw InternalError(PALUDIS_HERE, "bad index.role");
        }

        void visit(const RemoveDecision & remove_decision) const
        {
            const std::tr1::shared_ptr<Sequence<PackageDepSpec> > removing(new Sequence<PackageDepSpec>);
            for (PackageIDSequence::ConstIterator i(remove_decision.ids()->begin()),
                    i_end(remove_decision.ids()->end()) ;
                    i != i_end ; ++i)
                removing->push_back((*i)->uniquely_identifying_spec());

            resolved->job_lists()->execute_job_list()->append(make_shared_ptr(new UninstallJob(
                            make_shared_ptr(new JobRequirements),
                            removing
                            )));
        }
    };
}

void
Orderer::_schedule(
        const NAGIndex & index,
        const std::tr1::shared_ptr<const ChangeOrRemoveDecision> & d,
        const std::tr1::shared_ptr<const OrdererNotes> & n)
{
    do
    {
        switch (index.role())
        {
            case nir_done:
                _imp->resolved->taken_change_or_remove_decisions()->push_back(d, n);
                if (d->required_confirmations_if_any())
                    _imp->resolved->taken_unconfirmed_decisions()->push_back(d);
                continue;

            case nir_fetched:
                continue;

            case last_nir:
                break;
        }

        throw InternalError(PALUDIS_HERE, "bad index.role");
    } while (false);

    d->accept(ExtraScheduler(_imp->resolved, _imp->fetch_job_numbers, _imp->install_job_numbers, index));
}

