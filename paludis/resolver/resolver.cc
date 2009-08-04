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

#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/qpn_s.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/arrow.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/destinations.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/environment.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/spec_tree.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/metadata_key.hh>
#include <paludis/match_package.hh>
#include <paludis/action.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_requirements.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <iostream>
#include <iomanip>
#include <list>
#include <map>
#include <set>

using namespace paludis;
using namespace paludis::resolver;

typedef std::map<QPN_S, std::tr1::shared_ptr<Resolution> > ResolutionsByQPN_SMap;
typedef std::map<QPN_S, std::tr1::shared_ptr<Constraints> > InitialConstraints;
typedef std::list<std::tr1::shared_ptr<const Resolution> > OrderedResolutionsList;

namespace paludis
{
    template <>
    struct Implementation<Resolver>
    {
        const Environment * const env;
        const ResolverFunctions fns;

        ResolutionsByQPN_SMap resolutions_by_qpn_s;
        OrderedResolutionsList ordered_resolutions;

        Implementation(const Environment * const e, const ResolverFunctions & f) :
            env(e),
            fns(f)
        {
        }
    };
}

Resolver::Resolver(const Environment * const e, const ResolverFunctions & f) :
    PrivateImplementationPattern<Resolver>(new Implementation<Resolver>(e, f))
{
}

Resolver::~Resolver()
{
}

void
Resolver::resolve()
{
    Context context("When finding an appropriate resolution:");

    _resolve_dependencies();
    _resolve_destinations();
    _resolve_arrows();
    _resolve_order();
}

void
Resolver::_resolve_dependencies()
{
    Context context("When resolving dependencies recursively:");

    bool done(false);
    while (! done)
    {
        done = true;

        for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
                i != i_end ; ++i)
        {
            if (i->second->decision())
                continue;

            _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

            done = false;
            _decide(i->first, i->second);

            _add_dependencies(i->first, i->second);
        }
    }
}

void
Resolver::_resolve_destinations()
{
    Context context("When resolving destinations:");

    for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
            i != i_end ; ++i)
    {
        if (i->second->destinations())
            continue;

        i->second->destinations() = _make_destinations_for(i->first, i->second);
    }
}

const std::tr1::shared_ptr<Destinations>
Resolver::_make_destinations_for(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    Context context("When finding destinations for '" + stringify(qpn_s) + "':");

    if (resolution->decision()->is_installed())
        return make_shared_ptr(new Destinations(make_named_values<Destinations>(
                        value_for<n::slash>(make_null_shared_ptr())
                        )));

    bool requires_slash(false);

    for (Constraints::ConstIterator c(resolution->constraints()->begin()),
            c_end(resolution->constraints()->end()) ;
            c != c_end ; ++c)
        requires_slash = requires_slash || (*c)->to_destination_slash();

    return make_shared_ptr(new Destinations(
                make_named_values<Destinations>(
                    value_for<n::slash>(requires_slash ?
                        _make_slash_destination_for(qpn_s, resolution) :
                        make_null_shared_ptr())
                    )));
}

const std::tr1::shared_ptr<Destination>
Resolver::_make_slash_destination_for(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    Context context("When finding / destination for '" + stringify(qpn_s) + "':");

    if (! resolution->decision())
        throw InternalError(PALUDIS_HERE, "not decided. shouldn't happen.");

    std::tr1::shared_ptr<const Repository> repo;
    for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if ((*r)->destination_interface() && (*r)->destination_interface()->is_suitable_destination_for(
                    *resolution->decision()->package_id()))
        {
            if (repo)
                throw InternalError(PALUDIS_HERE, "multiple valid destinations.");
            else
                repo = *r;
        }
    }

    if (! repo)
        throw InternalError(PALUDIS_HERE, "no valid destination for " + stringify(*resolution));

    return make_shared_ptr(new Destination(make_named_values<Destination>(
                    value_for<n::replacing>(_find_replacing(resolution->decision()->package_id(), repo)),
                    value_for<n::repository>(repo->name())
                    )));
}

const std::tr1::shared_ptr<const PackageIDSequence>
Resolver::_find_replacing(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Repository> & repo) const
{
    Context context("When working out what is replaced by '" + stringify(*id) +
            "' when it is installed to '" + stringify(repo->name()) + "':");

    std::set<RepositoryName, RepositoryNameComparator> repos;

    if (repo->installed_root_key())
    {
        for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
                r_end(_imp->env->package_database()->end_repositories()) ;
                r != r_end ; ++r)
            if ((*r)->installed_root_key() &&
                    (*r)->installed_root_key()->value() == repo->installed_root_key()->value())
                repos.insert((*r)->name());
    }
    else
        repos.insert(repo->name());

    std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);
    for (std::set<RepositoryName, RepositoryNameComparator>::const_iterator r(repos.begin()),
            r_end(repos.end()) ;
            r != r_end ; ++r)
    {
        std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsUnsorted(
                    generator::Package(id->name()) & generator::InRepository(*r))]);
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            if ((*i)->version() == id->version() || _same_slot(*i, id))
                result->push_back(*i);
        }
    }

    return result;
}

bool
Resolver::_same_slot(const std::tr1::shared_ptr<const PackageID> & a,
        const std::tr1::shared_ptr<const PackageID> & b) const
{
    if (a->slot_key())
        return b->slot_key() && a->slot_key()->value() == b->slot_key()->value();
    else
        return ! b->slot_key();
}

void
Resolver::add_target_with_reason(const PackageDepSpec & spec, const std::tr1::shared_ptr<const Reason> & reason)
{
    Context context("When adding target '" + stringify(spec) + "' with reason '" + stringify(*reason) + "':");
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

    const std::tr1::shared_ptr<const QPN_S_Sequence> qpn_s_s(_imp->fns.get_qpn_s_s_for_fn()(spec, reason));
    if (qpn_s_s->empty())
        throw InternalError(PALUDIS_HERE, "not implemented: no slot for " + stringify(spec));

    for (QPN_S_Sequence::ConstIterator qpn_s(qpn_s_s->begin()), qpn_s_end(qpn_s_s->end()) ;
            qpn_s != qpn_s_end ; ++qpn_s)
    {
        Context context_2("When adding constraints from target '" + stringify(spec) + "' to qpn:s '"
                + stringify(*qpn_s) + "':");

        const std::tr1::shared_ptr<Resolution> dep_resolution(_resolution_for_qpn_s(*qpn_s, true));
        const std::tr1::shared_ptr<Constraint> constraint(_make_constraint_from_target(*qpn_s, spec, reason));

        _apply_resolution_constraint(*qpn_s, dep_resolution, constraint);
    }
}

void
Resolver::add_target(const PackageDepSpec & spec)
{
    add_target_with_reason(spec, make_shared_ptr(new TargetReason));
}

void
Resolver::add_target(const SetName & set_name)
{
    Context context("When adding set target '" + stringify(set_name) + "':");
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

    const std::tr1::shared_ptr<const SetSpecTree> set(_imp->env->set(set_name));
    if (! set)
        throw InternalError(PALUDIS_HERE, "todo");

    DepSpecFlattener<SetSpecTree, PackageDepSpec> flattener(_imp->env);
    set->root()->accept(flattener);

    const std::tr1::shared_ptr<Reason> reason(new SetReason(set_name, make_shared_ptr(new TargetReason)));
    for (DepSpecFlattener<SetSpecTree, PackageDepSpec>::ConstIterator s(flattener.begin()), s_end(flattener.end()) ;
            s != s_end ; ++s)
        add_target_with_reason(**s, reason);
}

QualifiedPackageName
Resolver::_package_from_spec(const PackageDepSpec & spec) const
{
    if (! spec.package_ptr())
        throw InternalError(PALUDIS_HERE, "not implemented");

    return *spec.package_ptr();
}

const std::tr1::shared_ptr<Resolution>
Resolver::_create_resolution_for_qpn_s(const QPN_S & qpn_s) const
{
    return make_shared_ptr(new Resolution(make_named_values<Resolution>(
                    value_for<n::already_ordered>(false),
                    value_for<n::arrows>(make_shared_ptr(new ArrowSequence)),
                    value_for<n::constraints>(_initial_constraints_for(qpn_s)),
                    value_for<n::decision>(make_null_shared_ptr()),
                    value_for<n::destinations>(make_null_shared_ptr()),
                    value_for<n::sanitised_dependencies>(make_null_shared_ptr())
                    )));
}

const std::tr1::shared_ptr<Resolution>
Resolver::_resolution_for_qpn_s(const QPN_S & qpn_s, const bool create)
{
    ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.find(qpn_s));
    if (_imp->resolutions_by_qpn_s.end() == i)
    {
        if (create)
            i = _imp->resolutions_by_qpn_s.insert(std::make_pair(qpn_s, _create_resolution_for_qpn_s(qpn_s))).first;
        else
            throw InternalError(PALUDIS_HERE, "doesn't exist");
    }

    return i->second;
}

const std::tr1::shared_ptr<Resolution>
Resolver::_resolution_for_qpn_s(const QPN_S & qpn_s) const
{
    ResolutionsByQPN_SMap::const_iterator i(_imp->resolutions_by_qpn_s.find(qpn_s));
    if (_imp->resolutions_by_qpn_s.end() == i)
        throw InternalError(PALUDIS_HERE, "doesn't exist");

    return i->second;
}

const std::tr1::shared_ptr<Constraint>
Resolver::_make_constraint_from_target(
        const QPN_S & qpn_s,
        const PackageDepSpec & spec,
        const std::tr1::shared_ptr<const Reason> & reason) const
{
    return make_shared_ptr(new Constraint(make_named_values<Constraint>(
                    value_for<n::reason>(reason),
                    value_for<n::spec>(spec),
                    value_for<n::to_destination_slash>(true),
                    value_for<n::use_installed>(_imp->fns.get_use_installed_fn()(qpn_s, spec, reason))
                    )));
}

const std::tr1::shared_ptr<Constraint>
Resolver::_make_constraint_from_dependency(const QPN_S & qpn_s, const SanitisedDependency & dep,
        const std::tr1::shared_ptr<const Reason> & reason) const
{
    return make_shared_ptr(new Constraint(make_named_values<Constraint>(
                    value_for<n::reason>(reason),
                    value_for<n::spec>(dep.spec()),
                    value_for<n::to_destination_slash>(_dependency_to_destination_slash(qpn_s, dep)),
                    value_for<n::use_installed>(_imp->fns.get_use_installed_fn()(qpn_s, dep.spec(), reason))
                    )));
}

void
Resolver::_apply_resolution_constraint(
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint)
{
    if (resolution->decision())
        _verify_new_constraint(qpn_s, resolution, constraint);

    resolution->constraints()->add(constraint);
}

void
Resolver::_verify_new_constraint(
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & r,
        const std::tr1::shared_ptr<const Constraint> & c)
{
    if (! _constraint_matches(c, r->decision()))
        _made_wrong_decision(qpn_s, r, c);
}

bool
Resolver::_constraint_matches(
        const std::tr1::shared_ptr<const Constraint> & c,
        const std::tr1::shared_ptr<const Decision> & decision) const
{
    Context context("When working out whether '" + stringify(*decision) + "' is matched by '" + stringify(*c) + "':");

    bool ok(true);

    switch (c->use_installed())
    {
        case ui_never:
            ok = ok && ! decision->is_installed();
            break;

        case ui_only_if_transient:
            ok = ok && ((! decision->is_installed()) || (decision->is_transient()));
            break;

        case ui_if_possible:
            break;

        case ui_if_same:
            if (decision->is_installed())
                ok = ok && (decision->is_same() || decision->is_transient());
            break;

        case ui_if_same_version:
            if (decision->is_installed())
                ok = ok && (decision->is_same_version() || decision->is_transient());
            break;

        case last_ui:
            break;
    }

    ok = ok && match_package(*_imp->env, c->spec(), *decision->package_id(), MatchPackageOptions());

    return ok;
}

void
Resolver::_decide(const QPN_S & qpn_s, const std::tr1::shared_ptr<Resolution> & resolution)
{
    Context context("When deciding upon an origin ID to use for '" + stringify(qpn_s) + "' with '"
            + stringify(*resolution) + "':");

    std::tr1::shared_ptr<Decision> decision(_try_to_find_decision_for(qpn_s, resolution));
    if (decision)
        resolution->decision() = decision;
    else
        _unable_to_decide(qpn_s, resolution);
}

const std::tr1::shared_ptr<Decision>
Resolver::_try_to_find_decision_for(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    std::tr1::shared_ptr<const PackageID> id;

    if (((id = _best_installed_id_for(qpn_s, resolution))))
        return _decision_from_package_id(qpn_s, id);

    if (((id = _best_installable_id_for(qpn_s, resolution))))
        return _decision_from_package_id(qpn_s, id);

    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const PackageID>
Resolver::_best_installed_id_for(const QPN_S & qpn_s, const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsSorted(
                generator::Package(qpn_s.package()) |
                qpn_s.make_slot_filter() |
                filter::SupportsAction<InstalledAction>())]);

    return _best_id_from(ids, qpn_s, resolution);
}

const std::tr1::shared_ptr<const PackageID>
Resolver::_best_installable_id_for(const QPN_S & qpn_s, const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsSorted(
                generator::Package(qpn_s.package()) |
                qpn_s.make_slot_filter() |
                filter::SupportsAction<InstallAction>() |
                filter::NotMasked())]);

    return _best_id_from(ids, qpn_s, resolution);
}

const std::tr1::shared_ptr<const PackageID>
Resolver::_best_id_from(
        const std::tr1::shared_ptr<const PackageIDSequence> & ids,
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    for (PackageIDSequence::ReverseConstIterator i(ids->rbegin()), i_end(ids->rend()) ;
            i != i_end ; ++i)
    {
        bool ok(true);

        for (Constraints::ConstIterator c(resolution->constraints()->begin()),
                c_end(resolution->constraints()->end()) ;
                c != c_end ; ++c)
        {
            if (! _constraint_matches(*c, _decision_from_package_id(qpn_s, *i)))
            {
                ok = false;
                break;
            }
        }

        if (ok)
            return *i;
    }

    return make_null_shared_ptr();
}

void
Resolver::_add_dependencies(const QPN_S & our_qpn_s, const std::tr1::shared_ptr<Resolution> & our_resolution)
{
    Context context("When adding dependencies for '" + stringify(our_qpn_s) + "' with '"
            + stringify(*our_resolution) + "':");

    const std::tr1::shared_ptr<SanitisedDependencies> deps(new SanitisedDependencies);
    deps->populate(*this, our_resolution->decision()->package_id());
    our_resolution->sanitised_dependencies() = deps;

    for (SanitisedDependencies::ConstIterator s(deps->begin()), s_end(deps->end()) ;
            s != s_end ; ++s)
    {
        Context context_2("When handling dependency '" + stringify(*s) + "':");

        if (! _care_about_dependency_spec(our_qpn_s, our_resolution, *s))
            continue;

        const std::tr1::shared_ptr<DependencyReason> reason(new DependencyReason(our_qpn_s, *s));

        const std::tr1::shared_ptr<const QPN_S_Sequence> qpn_s_s(_imp->fns.get_qpn_s_s_for_fn()(s->spec(), reason));
        if (qpn_s_s->empty())
        {
            if (our_resolution->decision()->is_installed())
                Log::get_instance()->message("resolver.cannot_find_installed_dep", ll_warning, lc_context)
                    << "Installed package '" << *our_resolution->decision() << "' dependency '" << *s << " cannot be met";
            else
                throw InternalError(PALUDIS_HERE, "not implemented: no slot for " + stringify(s->spec()));
        }

        for (QPN_S_Sequence::ConstIterator qpn_s(qpn_s_s->begin()), qpn_s_end(qpn_s_s->end()) ;
                qpn_s != qpn_s_end ; ++qpn_s)
        {
            const std::tr1::shared_ptr<Resolution> dep_resolution(_resolution_for_qpn_s(*qpn_s, true));
            const std::tr1::shared_ptr<Constraint> constraint(_make_constraint_from_dependency(our_qpn_s, *s, reason));

            _apply_resolution_constraint(*qpn_s, dep_resolution, constraint);
        }
    }
}

bool
Resolver::_care_about_dependency_spec(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution, const SanitisedDependency & dep) const
{
    return _imp->fns.care_about_dep_fn()(qpn_s, resolution, dep);
}

namespace
{
    struct GetDependencyReason
    {
        const DependencyReason * visit(const DependencyReason & r) const
        {
            return &r;
        }

        const DependencyReason * visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<const DependencyReason *>(*this);
        }

        const DependencyReason * visit(const TargetReason &) const
        {
            return 0;
        }

        const DependencyReason * visit(const PresetReason &) const
        {
            return 0;
        }
    };

    struct ArrowInfo
    {
        bool causes_pre_arrow;
        bool ignorable;

        ArrowInfo(const DependencyReason & reason) :
            causes_pre_arrow(false),
            ignorable(true)
        {
            const std::tr1::shared_ptr<const ActiveDependencyLabels> labels(
                    reason.sanitised_dependency().active_dependency_labels());

            if (labels->type_labels()->empty())
                throw InternalError(PALUDIS_HERE, "why did that happen?");

            std::for_each(indirect_iterator(labels->type_labels()->begin()),
                    indirect_iterator(labels->type_labels()->end()), accept_visitor(*this));
        }

        void visit(const DependencyBuildLabel &)
        {
            causes_pre_arrow = true;
            ignorable = false;
        }

        void visit(const DependencyRunLabel &)
        {
            causes_pre_arrow = true;
        }

        void visit(const DependencyPostLabel &)
        {
        }

        void visit(const DependencyInstallLabel &)
        {
            causes_pre_arrow = true;
            ignorable = false;
        }

        void visit(const DependencyCompileLabel &)
        {
            causes_pre_arrow = true;
            ignorable = false;
        }
    };
}

void
Resolver::_resolve_arrows()
{
    Context context("When creating arrows for order resolution:");

    for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
            i != i_end ; ++i)
    {
        for (Constraints::ConstIterator c(i->second->constraints()->begin()),
                c_end(i->second->constraints()->end()) ;
                c != c_end ; ++c)
        {
            GetDependencyReason gdr;
            const DependencyReason * if_dependency_reason((*c)->reason()->accept_returning<const DependencyReason *>(gdr));
            if (! if_dependency_reason)
                continue;

            const QPN_S from_qpns(if_dependency_reason->qpn_s());
            const std::tr1::shared_ptr<Resolution> resolution(_resolution_for_qpn_s(from_qpns, false));

            ArrowInfo a(*if_dependency_reason);
            if (a.causes_pre_arrow)
            {
                int ignorable_pass(0);
                if (_already_met(if_dependency_reason->sanitised_dependency()))
                    ignorable_pass = 1;
                else if (a.ignorable)
                    ignorable_pass = 2;

                resolution->arrows()->push_back(make_shared_ptr(new Arrow(make_named_values<Arrow>(
                                    value_for<n::ignorable_pass>(ignorable_pass),
                                    value_for<n::to_qpn_s>(i->first)
                                    ))));
            }
        }
    }
}

bool
Resolver::_already_met(const SanitisedDependency & dep) const
{
    std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::SomeArbitraryVersion(
                generator::Matches(dep.spec(), MatchPackageOptions()) |
                filter::SupportsAction<InstalledAction>())]);
    return ! ids->empty();
}

void
Resolver::_resolve_order()
{
    Context context("When finding an order for selected packages:");

    bool done(false);

    for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
            i != i_end ; ++i)
        if (i->second->decision()->is_installed())
            i->second->already_ordered() = true;

    while (! done)
    {
        bool any(false);
        done = true;

        int ignore_pass(0);
        while (true)
        {
            for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
                    i != i_end ; ++i)
            {
                if (i->second->already_ordered())
                    continue;

                if (_can_order_now(i->first, i->second, ignore_pass))
                {
                    if (0 != ignore_pass)
                        Log::get_instance()->message("resolver.cycle_breaking", ll_warning, lc_context)
                            << "Had to use cycle breaking with ignore pass " << ignore_pass
                            << " to order " << i->first << " because of cycle "
                            << _find_cycle(i->first, false);

                    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());
                    _do_order(i->first, i->second);
                    any = true;

                    if (0 != ignore_pass)
                        break;
                }
                else
                    done = false;
            }

            if ((! done) && (! any))
            {
                if (ignore_pass >= 2)
                    _unable_to_order_more();
                else
                    ++ignore_pass;
            }
            else
                break;
        }
    }
}

bool
Resolver::_can_order_now(const QPN_S &, const std::tr1::shared_ptr<const Resolution> & resolution,
        const int ignorable_pass) const
{
    for (ArrowSequence::ConstIterator a(resolution->arrows()->begin()), a_end(resolution->arrows()->end()) ;
            a != a_end ; ++a)
    {
        if (0 != (*a)->ignorable_pass())
            if ((*a)->ignorable_pass() <= ignorable_pass)
                continue;

        const std::tr1::shared_ptr<const Resolution> dep_resolution(_resolution_for_qpn_s((*a)->to_qpn_s()));
        if (! dep_resolution->already_ordered())
            return false;
    }

    return true;
}

void
Resolver::_do_order(const QPN_S &, const std::tr1::shared_ptr<Resolution> & resolution)
{
    _imp->ordered_resolutions.push_back(resolution);
    resolution->already_ordered() = true;
}

const std::tr1::shared_ptr<Decision>
Resolver::_decision_from_package_id(const QPN_S & qpn_s, const std::tr1::shared_ptr<const PackageID> & id) const
{
    bool is_installed, is_transient, is_new, is_same, is_same_version;

    is_installed = id->supports_action(SupportsActionTest<InstalledAction>());
    is_transient = is_installed && id->transient_key() && id->transient_key()->value();

    std::tr1::shared_ptr<const PackageIDSequence> comparison_ids;

    if (is_installed)
        comparison_ids = ((*_imp->env)[selection::BestVersionOnly(
                    generator::Package(qpn_s.package()) |
                    qpn_s.make_slot_filter() |
                    filter::SupportsAction<InstallAction>() |
                    filter::NotMasked())]);
    else
        comparison_ids = ((*_imp->env)[selection::BestVersionOnly(
                    generator::Package(qpn_s.package()) |
                    qpn_s.make_slot_filter() |
                    filter::SupportsAction<InstalledAction>())]);

    if (comparison_ids->empty())
    {
        is_new = true;
        is_same = false;
        is_same_version = false;
    }
    else
    {
        is_new = false;
        is_same = ((*comparison_ids->begin())->version() == id->version());
        is_same_version = is_same;
    }

    return make_shared_ptr(new Decision(make_named_values<Decision>(
                    value_for<n::is_installed>(is_installed),
                    value_for<n::is_new>(is_new),
                    value_for<n::is_same>(is_same),
                    value_for<n::is_same_version>(is_same_version),
                    value_for<n::is_transient>(is_transient),
                    value_for<n::package_id>(id)
                    )));
}

void
Resolver::_unable_to_decide(
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsSorted(
                generator::Package(qpn_s.package()) |
                qpn_s.make_slot_filter() |
                filter::SupportsAction<InstallAction>() |
                filter::NotMasked())]);

    if (ids->empty())
    {
        std::cout << "Unable to find anything at all unmasked and installable for " << qpn_s << std::endl;
    }
    else
    {
        std::cout << "Unable to find anything suitable for " << qpn_s << ":" << std::endl;
        for (PackageIDSequence::ReverseConstIterator i(ids->rbegin()), i_end(ids->rend()) ;
                i != i_end ; ++i)
        {
            std::cout << "  * " << **i << " doesn't match:" << std::endl;
            for (Constraints::ConstIterator c(resolution->constraints()->begin()),
                    c_end(resolution->constraints()->end()) ;
                    c != c_end ; ++c)
                if (! _constraint_matches(*c, _decision_from_package_id(qpn_s, *i)))
                    std::cout << "    * " << **c << std::endl;
        }
    }

    throw InternalError(PALUDIS_HERE, "not implemented");
}

void
Resolver::_unable_to_order_more() const
{
    std::cout << "Unable to order any of the following:" << std::endl;

    for (ResolutionsByQPN_SMap::const_iterator i(_imp->resolutions_by_qpn_s.begin()),
            i_end(_imp->resolutions_by_qpn_s.end()) ;
            i != i_end ; ++i)
    {
        if (i->second->already_ordered())
            continue;

        std::cout << "  * " << *i->second->decision() << " because of cycle " << _find_cycle(i->first, true)
            << std::endl;
    }

    throw InternalError(PALUDIS_HERE, "not implemented");
}

const std::tr1::shared_ptr<Constraints>
Resolver::_initial_constraints_for(const QPN_S & qpn_s) const
{
    return _imp->fns.get_initial_constraints_for_fn()(qpn_s);
}

void
Resolver::_made_wrong_decision(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint)
{
    /* can we find a resolution that works for all our constraints? */
    std::tr1::shared_ptr<Resolution> adapted_resolution(make_shared_ptr(new Resolution(*resolution)));
    adapted_resolution->constraints()->add(constraint);

    const std::tr1::shared_ptr<Decision> decision(_try_to_find_decision_for(qpn_s, adapted_resolution));
    if (decision)
    {
        /* can we preload and restart? */
        if (_initial_constraints_for(qpn_s)->empty())
        {
            /* we've not already locked this to something. yes! */
            _suggest_restart_with(qpn_s, resolution, constraint, decision);
        }
        else
        {
            /* we can restart if we've selected the same or a newer version
             * than before. but we don't support that yet. */
            throw InternalError(PALUDIS_HERE, "should have selected " + stringify(*decision));
        }
    }
    else
        _unable_to_decide(qpn_s, adapted_resolution);
}

void
Resolver::_suggest_restart_with(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint,
        const std::tr1::shared_ptr<const Decision> & decision) const
{
    throw SuggestRestart(qpn_s, resolution->decision(), constraint, decision, _make_constraint_for_preloading(qpn_s, decision));
}

const std::tr1::shared_ptr<const Constraint>
Resolver::_make_constraint_for_preloading(
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Decision> & d) const
{
    const std::tr1::shared_ptr<PresetReason> reason(new PresetReason);

    return make_shared_ptr(new Constraint(make_named_values<Constraint>(
                    value_for<n::reason>(reason),
                    value_for<n::spec>(d->package_id()->uniquely_identifying_spec()),
                    value_for<n::to_destination_slash>(false),
                    value_for<n::use_installed>(_imp->fns.get_use_installed_fn()(qpn_s, d->package_id()->uniquely_identifying_spec(), reason))
                    )));
}

bool
Resolver::_dependency_to_destination_slash(const QPN_S &, const SanitisedDependency &) const
{
    return true;
}

Resolver::ConstIterator
Resolver::begin() const
{
    return ConstIterator(_imp->ordered_resolutions.begin());
}

Resolver::ConstIterator
Resolver::end() const
{
    return ConstIterator(_imp->ordered_resolutions.end());
}

Resolver::ResolutionsByQPN_SConstIterator
Resolver::begin_resolutions_by_qpn_s() const
{
    return ResolutionsByQPN_SConstIterator(_imp->resolutions_by_qpn_s.begin());
}

Resolver::ResolutionsByQPN_SConstIterator
Resolver::end_resolutions_by_qpn_s() const
{
    return ResolutionsByQPN_SConstIterator(_imp->resolutions_by_qpn_s.end());
}

int
Resolver::find_any_score(const QPN_S & our_qpn_s, const SanitisedDependency & dep) const
{
    Context context("When working out whether we'd like '" + stringify(dep) + "' because of '"
            + stringify(our_qpn_s) + "':");

    int operator_bias(0);
    if (dep.spec().version_requirements_ptr() && ! dep.spec().version_requirements_ptr()->empty())
    {
        int score(-1);
        for (VersionRequirements::ConstIterator v(dep.spec().version_requirements_ptr()->begin()),
                v_end(dep.spec().version_requirements_ptr()->end()) ;
                v != v_end ; ++v)
        {
            int local_score(0);

            switch (v->version_operator().value())
            {
                case vo_greater:
                case vo_greater_equal:
                    local_score = 9;
                    break;

                case vo_equal:
                case vo_tilde:
                case vo_nice_equal_star:
                case vo_stupid_equal_star:
                case vo_tilde_greater:
                    local_score = 2;
                    break;

                case vo_less_equal:
                case vo_less:
                    local_score = 1;
                    break;

                case last_vo:
                    local_score = 1;
                    break;
            }

            if (score == -1)
                score = local_score;
            else
                switch (dep.spec().version_requirements_mode())
                {
                    case vr_and:
                        score = std::min(score, local_score);
                        break;

                    case vr_or:
                        score = std::max(score, local_score);
                        break;

                    case last_vr:
                        break;
                }
        }
        operator_bias = score;
    }
    else
    {
        /* don't bias no operator over a >= operator, so || ( >=foo-2 bar )
         * still likes foo. */
        operator_bias = 9;
    }

    /* best: already installed */
    {
        const std::tr1::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(dep.spec(), MatchPackageOptions() + mpo_ignore_additional_requirements) |
                    filter::SupportsAction<InstalledAction>())]);
        if (! installed_ids->empty())
            return 40 + operator_bias;
    }

    const std::tr1::shared_ptr<DependencyReason> reason(new DependencyReason(our_qpn_s, dep));
    const std::tr1::shared_ptr<const QPN_S_Sequence> qpn_s_s(_imp->fns.get_qpn_s_s_for_fn()(dep.spec(), reason));

    /* next: will already be installing */
    {
        for (QPN_S_Sequence::ConstIterator qpn_s(qpn_s_s->begin()), qpn_s_end(qpn_s_s->end()) ;
                qpn_s != qpn_s_end ; ++qpn_s)
        {
            ResolutionsByQPN_SMap::const_iterator i(_imp->resolutions_by_qpn_s.find(*qpn_s));
            if (i != _imp->resolutions_by_qpn_s.end())
                return 30 + operator_bias;
        }
    }

    /* next: could install */
    {
        for (QPN_S_Sequence::ConstIterator qpn_s(qpn_s_s->begin()), qpn_s_end(qpn_s_s->end()) ;
                qpn_s != qpn_s_end ; ++qpn_s)
        {
            const std::tr1::shared_ptr<Resolution> resolution(_create_resolution_for_qpn_s(*qpn_s));
            const std::tr1::shared_ptr<Constraint> constraint(_make_constraint_from_dependency(our_qpn_s, dep, reason));
            resolution->constraints()->add(constraint);
            const std::tr1::shared_ptr<Decision> decision(_try_to_find_decision_for(*qpn_s, resolution));
            if (decision)
                return 20 + operator_bias;
        }
    }

    /* next: exists */
    {
        const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(dep.spec(), MatchPackageOptions() + mpo_ignore_additional_requirements)
                    )]);
        if (! ids->empty())
            return 10 + operator_bias;
    }

    /* yay, people are depping upon packages that don't exist again. I SMELL A LESSPIPE. */
    return 0;
}

const std::string
Resolver::_find_cycle(const QPN_S & start_qpn_s, const int ignorable_pass) const
{
    std::stringstream result;

    std::set<QPN_S> seen;
    QPN_S current(start_qpn_s);

    bool first(true);
    while (true)
    {
        if (! first)
            result << " -> ";
        first = false;

        result << current;

        if (! seen.insert(current).second)
            break;

        bool ok(false);
        const std::tr1::shared_ptr<const Resolution> resolution(_resolution_for_qpn_s(current));
        for (ArrowSequence::ConstIterator a(resolution->arrows()->begin()), a_end(resolution->arrows()->end()) ;
                a != a_end ; ++a)
        {
            if (_can_order_now(current, resolution, ignorable_pass))
                continue;

            const std::tr1::shared_ptr<const Resolution> to_resolution(_resolution_for_qpn_s((*a)->to_qpn_s()));
            if (to_resolution->already_ordered())
                continue;

            ok = true;
            current = (*a)->to_qpn_s();
            break;
        }

        if (! ok)
            throw InternalError(PALUDIS_HERE, "why did that happen? start is " + stringify(start_qpn_s)
                    + ", current is " + stringify(current) + ", seen is { "
                    + join(seen.begin(), seen.end(), ", ") + " }, result is "
                    + result.str() + ", resolution->arrows is { "
                    + join(indirect_iterator(resolution->arrows()->begin()),
                        indirect_iterator(resolution->arrows()->end()), ", ") + "}"
                    );
    }

    return result.str();
}

template class WrappedForwardIterator<Resolver::ConstIteratorTag, const std::tr1::shared_ptr<const Resolution> >;
template class WrappedForwardIterator<Resolver::ResolutionsByQPN_SConstIteratorTag,
         const std::pair<const QPN_S, std::tr1::shared_ptr<Resolution> > >;

