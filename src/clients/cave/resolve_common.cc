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

#include "resolve_common.hh"
#include "cmd_resolve_display_callback.hh"
#include "cmd_resolve_dump.hh"
#include "cmd_display_resolution.hh"
#include "cmd_execute_resolution.hh"
#include "exceptions.hh"
#include "command_command_line.hh"
#include "match_qpns.hh"

#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/system.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/string_list_stream.hh>
#include <paludis/util/thread.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/args/do_help.hh>
#include <paludis/args/escape.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/required_confirmations.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/decisions.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/selection_cache.hh>
#include <paludis/package_dep_spec_properties.hh>
#include <paludis/elike_slot_requirement.hh>

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <list>
#include <map>

using namespace paludis;
using namespace paludis::resolver;
using namespace cave;

using std::cout;
using std::endl;

namespace
{
    typedef std::map<Resolvent, std::tr1::shared_ptr<Constraints> > InitialConstraints;
    typedef std::list<PackageDepSpec> PackageDepSpecList;

    struct LabelTypesVisitor
    {
        bool is_suggestion;
        bool is_recommendation;
        bool is_requirement;
        bool seen_buildish_dep;
        bool seen_runish_dep;
        bool seen_compiled_against_dep;
        bool seen_enabled_dep;

        LabelTypesVisitor() :
            is_suggestion(false),
            is_recommendation(false),
            is_requirement(false),
            seen_buildish_dep(false),
            seen_runish_dep(false),
            seen_compiled_against_dep(false),
            seen_enabled_dep(false)
        {
        }

        void visit(const DependenciesBuildLabel & l)
        {
            if (l.enabled())
            {
                is_requirement = true;
                seen_buildish_dep = true;
                seen_enabled_dep = true;
            }
        }

        void visit(const DependenciesTestLabel & l)
        {
            if (l.enabled())
            {
                is_requirement = true;
                seen_buildish_dep = true;
                seen_enabled_dep = true;
            }
        }

        void visit(const DependenciesFetchLabel & l)
        {
            if (l.enabled())
            {
                is_requirement = true;
                seen_buildish_dep = true;
                seen_enabled_dep = true;
            }
        }

        void visit(const DependenciesRunLabel & l)
        {
            if (l.enabled())
            {
                is_requirement = true;
                seen_runish_dep = true;
                seen_enabled_dep = true;
            }
        }

        void visit(const DependenciesPostLabel & l)
        {
            if (l.enabled())
            {
                is_requirement = true;
                seen_runish_dep = true;
                seen_enabled_dep = true;
            }
        }

        void visit(const DependenciesInstallLabel & l)
        {
            if (l.enabled())
            {
                is_requirement = true;
                seen_buildish_dep = true;
                seen_enabled_dep = true;
            }
        }

        void visit(const DependenciesCompileAgainstLabel & l)
        {
            if (l.enabled())
            {
                is_requirement = true;
                seen_runish_dep = true;
                seen_buildish_dep = true;
                seen_enabled_dep = true;
            }
        }

        void visit(const DependenciesRecommendationLabel & l)
        {
            if (l.enabled())
            {
                is_recommendation = true;
                seen_runish_dep = true;
                seen_enabled_dep = true;
            }
        }

        void visit(const DependenciesSuggestionLabel & l)
        {
            if (l.enabled())
            {
                is_suggestion = true;
                seen_runish_dep = true;
                seen_enabled_dep = true;
            }
        }
    };

    bool is_suggestion(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->empty())
            return false;

        LabelTypesVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->end()),
                accept_visitor(v));
        return v.is_suggestion && (! v.is_recommendation) && (! v.is_requirement);
    }

    bool is_recommendation(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->empty())
            return false;

        LabelTypesVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->end()),
                accept_visitor(v));
        return v.is_recommendation && (! v.is_requirement);
    }

    bool is_just_build_dep(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->empty())
            throw InternalError(PALUDIS_HERE, "not implemented");

        LabelTypesVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->end()),
                accept_visitor(v));
        return v.seen_buildish_dep && ! v.seen_runish_dep;
    }

    bool is_compiled_against_dep(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->empty())
            throw InternalError(PALUDIS_HERE, "not implemented");

        LabelTypesVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->end()),
                accept_visitor(v));
        return v.seen_compiled_against_dep;
    }

    bool is_enabled_dep(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->empty())
            throw InternalError(PALUDIS_HERE, "not implemented");

        LabelTypesVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->end()),
                accept_visitor(v));
        return v.seen_enabled_dep;
    }

    struct DestinationTypesFinder
    {
        const Environment * const env;
        const ResolveCommandLineResolutionOptions & resolution_options;
        const std::tr1::shared_ptr<const PackageID> package_id;

        DestinationTypesFinder(
                const Environment * const e,
                const ResolveCommandLineResolutionOptions & c,
                const std::tr1::shared_ptr<const PackageID> & i) :
            env(e),
            resolution_options(c),
            package_id(i)
        {
        }

        DestinationTypes visit(const TargetReason &) const
        {
            if (resolution_options.a_make.argument() == "binaries")
                return DestinationTypes() + dt_create_binary;
            else if (resolution_options.a_make.argument() == "install")
                return DestinationTypes() + dt_install_to_slash;
            else
                throw args::DoHelp("Don't understand argument '" + resolution_options.a_make.argument() + "' to '--"
                        + resolution_options.a_make.long_name() + "'");
        }

        DestinationTypes visit(const DependentReason &) const
        {
            return DestinationTypes() + dt_install_to_slash;
        }

        DestinationTypes visit(const WasUsedByReason &) const
        {
            return DestinationTypes() + dt_install_to_slash;
        }

        DestinationTypes visit(const DependencyReason &) const
        {
            return DestinationTypes() + dt_install_to_slash;
        }

        DestinationTypes visit(const PresetReason &) const
        {
            return DestinationTypes();
        }

        DestinationTypes visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<DestinationTypes>(*this);
        }
    };

    DestinationTypes get_destination_types_for_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const PackageDepSpec &,
            const std::tr1::shared_ptr<const PackageID> & id,
            const std::tr1::shared_ptr<const Reason> & reason)
    {
        DestinationTypesFinder f(env, resolution_options, id);
        return reason->accept_returning<DestinationTypes>(f);
    }

    FilteredGenerator make_destination_filtered_generator(
            const Environment * const,
            const ResolveCommandLineResolutionOptions &,
            const std::tr1::shared_ptr<const Generator> & all_binary_repos_generator,
            const Generator & g,
            const Resolvent & r)
    {
        switch (r.destination_type())
        {
            case dt_install_to_slash:
                return g | filter::InstalledAtRoot(FSEntry("/"));

            case dt_create_binary:
                if (all_binary_repos_generator)
                    return g & *all_binary_repos_generator;
                else
                    return generator::Nothing();

            case last_dt:
                break;
        }

        throw InternalError(PALUDIS_HERE, stringify(r.destination_type()));
    }

    void add_resolver_targets(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<Resolver> & resolver,
            const ResolveCommandLineResolutionOptions &,
            const std::tr1::shared_ptr<const Sequence<std::string> > & targets,
            bool & is_set)
    {
        Context context("When adding targets from commandline:");

        if (targets->empty())
            throw args::DoHelp("Must specify at least one target");

        bool seen_sets(false), seen_packages(false);
        for (Sequence<std::string>::ConstIterator p(targets->begin()), p_end(targets->end()) ;
                p != p_end ; ++p)
        {
            if (p->empty())
                continue;

            try
            {
                if ('!' == p->at(0))
                {
                    seen_packages = true;
                    PackageDepSpec s(parse_user_package_dep_spec(p->substr(1), env.get(), UserPackageDepSpecOptions()));
                    resolver->add_target(BlockDepSpec(*p, s, false));
                }
                else
                {
                    resolver->add_target(parse_user_package_dep_spec(*p, env.get(),
                                UserPackageDepSpecOptions() + updso_throw_if_set));
                    seen_packages = true;
                }
            }
            catch (const GotASetNotAPackageDepSpec &)
            {
                if (seen_sets)
                    throw args::DoHelp("Cannot specify multiple set targets");

                resolver->add_target(SetName(*p));
                seen_sets = true;
            }
        }

        if (seen_sets + seen_packages > 1)
            throw args::DoHelp("Cannot specify set and non-set targets simultaneously");

        if (seen_sets)
            is_set = true;
    }

    UseExisting use_existing_from_cmdline(const args::EnumArg & a, const bool is_set)
    {
        if (a.argument() == "auto")
            return is_set ? ue_if_same : ue_never;
        else if (a.argument() == "never")
            return ue_never;
        else if (a.argument() == "if-transient")
            return ue_only_if_transient;
        else if (a.argument() == "if-same")
            return ue_if_same;
        else if (a.argument() == "if-same-version")
            return ue_if_same_version;
        else if (a.argument() == "if-possible")
            return ue_if_possible;
        else
            throw args::DoHelp("Don't understand argument '" + a.argument() + "' to '--" + a.long_name() + "'");
    }

    struct UseExistingVisitor
    {
        const ResolveCommandLineResolutionOptions & resolution_options;
        const bool from_set;

        UseExistingVisitor(const ResolveCommandLineResolutionOptions & c, const bool f) :
            resolution_options(c),
            from_set(f)
        {
        }

        UseExisting visit(const DependencyReason &) const
        {
            return use_existing_from_cmdline(resolution_options.a_keep, false);
        }

        UseExisting visit(const TargetReason &) const
        {
            return use_existing_from_cmdline(resolution_options.a_keep_targets, from_set);
        }

        UseExisting visit(const DependentReason &) const
        {
            return ue_if_possible;
        }

        UseExisting visit(const WasUsedByReason &) const
        {
            return ue_if_possible;
        }

        UseExisting visit(const PresetReason &) const
        {
            return ue_if_possible;
        }

        UseExisting visit(const SetReason & r) const
        {
            UseExistingVisitor v(resolution_options, true);
            return r.reason_for_set()->accept_returning<UseExisting>(v);
        }
    };

    bool use_existing_from_withish(
            const Environment * const,
            const QualifiedPackageName & name,
            const PackageDepSpecList & list)
    {
        for (PackageDepSpecList::const_iterator l(list.begin()), l_end(list.end()) ;
                l != l_end ; ++l)
        {
            if (! package_dep_spec_has_properties(*l, make_named_values<PackageDepSpecProperties>(
                            n::has_additional_requirements() = false,
                            n::has_category_name_part() = false,
                            n::has_from_repository() = false,
                            n::has_in_repository() = false,
                            n::has_installable_to_path() = false,
                            n::has_installable_to_repository() = false,
                            n::has_installed_at_path() = false,
                            n::has_package() = true,
                            n::has_package_name_part() = false,
                            n::has_slot_requirement() = false,
                            n::has_tag() = indeterminate,
                            n::has_version_requirements() = false
                            )))
                throw args::DoHelp("'" + stringify(*l) + "' is not a simple cat/pkg");

            if (name == *l->package_ptr())
                return true;
        }

        return false;
    }

    UseExisting use_existing_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const PackageDepSpecList & without,
            const PackageDepSpecList & with,
            const std::tr1::shared_ptr<const Resolution> &,
            const PackageDepSpec & spec,
            const std::tr1::shared_ptr<const Reason> & reason)
    {
        if (spec.package_ptr())
        {
            if (use_existing_from_withish(env, *spec.package_ptr(), without))
                return ue_if_possible;
            if (use_existing_from_withish(env, *spec.package_ptr(), with))
                return ue_never;
        }

        UseExistingVisitor v(resolution_options, false);
        return reason->accept_returning<UseExisting>(v);
    }

    int reinstall_scm_days(const ResolveCommandLineResolutionOptions & resolution_options)
    {
        if (resolution_options.a_reinstall_scm.argument() == "always")
            return 0;
        else if (resolution_options.a_reinstall_scm.argument() == "daily")
            return 1;
        else if (resolution_options.a_reinstall_scm.argument() == "weekly")
            return 7;
        else if (resolution_options.a_reinstall_scm.argument() == "never")
            return -1;
        else
            throw args::DoHelp("Don't understand argument '" + resolution_options.a_reinstall_scm.argument() + "' to '--"
                    + resolution_options.a_reinstall_scm.long_name() + "'");
    }

    bool is_scm_name(const QualifiedPackageName & n)
    {
        std::string pkg(stringify(n.package()));
        switch (pkg.length())
        {
            case 0:
            case 1:
            case 2:
            case 3:
                return false;

            default:
                if (0 == pkg.compare(pkg.length() - 6, 6, "-darcs"))
                    return true;

            case 5:
                if (0 == pkg.compare(pkg.length() - 5, 5, "-live"))
                    return true;

            case 4:
                if (0 == pkg.compare(pkg.length() - 4, 4, "-cvs"))
                    return true;
                if (0 == pkg.compare(pkg.length() - 4, 4, "-svn"))
                    return true;
                return false;
        }
    }

    bool is_scm_older_than(const std::tr1::shared_ptr<const PackageID> & id, const int n)
    {
        if (id->version().is_scm() || is_scm_name(id->name()))
        {
            static Timestamp current_time(Timestamp::now()); /* static to avoid weirdness */
            time_t installed_time(current_time.seconds());
            if (id->installed_time_key())
                installed_time = id->installed_time_key()->value().seconds();

            return (current_time.seconds() - installed_time) > (24 * 60 * 60 * n);
        }
        else
            return false;
    }

    bool installed_is_scm_older_than(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const std::tr1::shared_ptr<const Generator> & all_binary_repos_generator,
            const Resolvent & q,
            const int n)
    {
        Context context("When working out whether '" + stringify(q) + "' has installed SCM packages:");

        const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsUnsorted(
                    make_destination_filtered_generator(env, resolution_options, all_binary_repos_generator,
                        generator::Package(q.package()), q) |
                    make_slot_filter(q)
                    )]);

        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            if (is_scm_older_than(*i, n))
                return true;
        }

        return false;
    }

    const std::tr1::shared_ptr<Constraints> make_initial_constraints_for(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const std::tr1::shared_ptr<const Generator> & all_binary_repos_generator,
            const PackageDepSpecList & without,
            const Resolvent & resolvent)
    {
        const std::tr1::shared_ptr<Constraints> result(new Constraints);

        int n(reinstall_scm_days(resolution_options));
        if ((-1 != n) && installed_is_scm_older_than(env, resolution_options, all_binary_repos_generator, resolvent, n)
                && ! use_existing_from_withish(env, resolvent.package(), without))
        {
            result->add(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                                n::destination_type() = resolvent.destination_type(),
                                n::nothing_is_fine_too() = true,
                                n::reason() = make_shared_ptr(new PresetReason("is scm", make_null_shared_ptr())),
                                n::spec() = make_package_dep_spec(PartiallyMadePackageDepSpecOptions()).package(resolvent.package()),
                                n::untaken() = false,
                                n::use_existing() = ue_only_if_transient
                                ))));
        }

        return result;
    }

    const std::tr1::shared_ptr<Constraints> initial_constraints_for_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const PackageDepSpecList & without,
            const InitialConstraints & initial_constraints,
            const std::tr1::shared_ptr<const Generator> & all_binary_repos_generator,
            const Resolvent & resolvent)
    {
        InitialConstraints::const_iterator i(initial_constraints.find(resolvent));
        if (i == initial_constraints.end())
            return make_initial_constraints_for(env, resolution_options, all_binary_repos_generator, without, resolvent);
        else
            return i->second;
    }

    struct IsTargetVisitor
    {
        bool visit(const DependencyReason &) const
        {
            return false;
        }

        bool visit(const DependentReason &) const
        {
            return false;
        }

        bool visit(const WasUsedByReason &) const
        {
            return false;
        }

        bool visit(const PresetReason &) const
        {
            return false;
        }

        bool visit(const TargetReason &) const
        {
            return true;
        }

        bool visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<bool>(*this);
        }
    };

    bool is_target(const std::tr1::shared_ptr<const Reason> & reason)
    {
        IsTargetVisitor v;
        return reason->accept_returning<bool>(v);
    }

    const std::tr1::shared_ptr<Resolvents>
    get_resolvents_for_fn(const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const PackageDepSpec & spec,
            const std::tr1::shared_ptr<const SlotName> & maybe_slot,
            const std::tr1::shared_ptr<const Reason> & reason)
    {
        std::tr1::shared_ptr<PackageIDSequence> result_ids(new PackageIDSequence);
        std::tr1::shared_ptr<const PackageID> best;

        const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(
                    generator::Matches(spec, MatchPackageOptions() + mpo_ignore_additional_requirements) |
                    filter::SupportsAction<InstallAction>() |
                    filter::NotMasked() |
                    (maybe_slot ? Filter(filter::Slot(*maybe_slot)) : Filter(filter::All())))]);

        if (! ids->empty())
            best = *ids->begin();

        const std::tr1::shared_ptr<const PackageIDSequence> installed_ids((*env)[selection::BestVersionInEachSlot(
                    generator::Matches(spec, MatchPackageOptions()) |
                    filter::InstalledAtRoot(FSEntry("/")))]);

        const args::EnumArg & arg(is_target(reason) ? resolution_options.a_target_slots : resolution_options.a_slots);

        if (! best)
            std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());
        else if (arg.argument() == "best-or-installed")
        {
            if (indirect_iterator(installed_ids->end()) == std::find(indirect_iterator(installed_ids->begin()),
                        indirect_iterator(installed_ids->end()), *best))
                result_ids->push_back(best);
            else
                std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());
        }
        else if (arg.argument() == "installed-or-best")
        {
            if (installed_ids->empty())
                result_ids->push_back(best);
            else
                std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());
        }
        else if (arg.argument() == "all")
        {
            if (indirect_iterator(installed_ids->end()) == std::find(indirect_iterator(installed_ids->begin()),
                        indirect_iterator(installed_ids->end()), *best))
                result_ids->push_back(best);
            std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());
        }
        else if (arg.argument() == "best")
            result_ids->push_back(best);
        else
            throw args::DoHelp("Don't understand argument '" + arg.argument() + "' to '--"
                    + arg.long_name() + "'");

        std::tr1::shared_ptr<Resolvents> result(new Resolvents);
        for (PackageIDSequence::ConstIterator i(result_ids->begin()), i_end(result_ids->end()) ;
                i != i_end ; ++i)
        {
            DestinationTypes destination_types(get_destination_types_for_fn(env, resolution_options, spec, *i, reason));
            for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
                if (destination_types[*t])
                    result->push_back(Resolvent(*i, *t));
        }

        return result;
    }

    bool ignore_dep_from(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions &,
            const PackageDepSpecList & no_blockers_from,
            const PackageDepSpecList & no_dependencies_from,
            const std::tr1::shared_ptr<const PackageID> & id,
            const bool is_block)
    {
        const PackageDepSpecList & list(is_block ? no_blockers_from : no_dependencies_from);

        for (PackageDepSpecList::const_iterator l(list.begin()), l_end(list.end()) ;
                l != l_end ; ++l)
            if (match_package(*env, *l, *id, MatchPackageOptions()))
                return true;

        return false;
    }

    struct CareAboutDepFnVisitor
    {
        const Environment * const env;
        const ResolveCommandLineResolutionOptions & resolution_options;
        const PackageDepSpecList & no_blockers_from;
        const PackageDepSpecList & no_dependencies_from;
        const SanitisedDependency dep;

        CareAboutDepFnVisitor(const Environment * const e,
                const ResolveCommandLineResolutionOptions & c,
                const PackageDepSpecList & b,
                const PackageDepSpecList & f,
                const SanitisedDependency & d) :
            env(e),
            resolution_options(c),
            no_blockers_from(b),
            no_dependencies_from(f),
            dep(d)
        {
        }

        bool visit(const ExistingNoChangeDecision & decision) const
        {
            if (ignore_dep_from(env, resolution_options, no_blockers_from, no_dependencies_from,
                        decision.existing_id(), dep.spec().if_block()))
                return false;

            if (! is_enabled_dep(dep))
                return false;

            if (! resolution_options.a_follow_installed_build_dependencies.specified())
                if (is_just_build_dep(dep))
                    return false;
            if (resolution_options.a_no_follow_installed_dependencies.specified())
                if (! is_compiled_against_dep(dep))
                    return false;

            if (is_suggestion(dep) || is_recommendation(dep))
            {
                /* we only take a suggestion or recommendation for an existing
                 * package if it's already met. for now, we ignore suggested
                 * and recommended blocks no matter what. */
                if (dep.spec().if_block())
                    return false;

                const std::tr1::shared_ptr<const PackageIDSequence> installed_ids(
                        (*env)[selection::SomeArbitraryVersion(
                            generator::Matches(*dep.spec().if_package(), MatchPackageOptions()) |
                            filter::InstalledAtRoot(FSEntry("/")))]);
                if (installed_ids->empty())
                    return false;
            }

            return true;
        }

        bool visit(const NothingNoChangeDecision &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "NothingNoChangeDecision shouldn't have deps");
        }

        bool visit(const UnableToMakeDecision &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "UnableToMakeDecision shouldn't have deps");
        }

        bool visit(const RemoveDecision &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "RemoveDecision shouldn't have deps");
        }

        bool visit(const BreakDecision &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "BreakDecision shouldn't have deps");
        }

        bool visit(const ChangesToMakeDecision & decision) const
        {
            if (ignore_dep_from(env, resolution_options, no_blockers_from,
                        no_dependencies_from, decision.origin_id(), dep.spec().if_block()))
                return false;

            if (is_enabled_dep(dep))
                return true;

            return false;
        }
    };

    SpecInterest interest_in_spec_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const PackageDepSpecList & take,
            const PackageDepSpecList & take_from,
            const PackageDepSpecList & ignore,
            const PackageDepSpecList & ignore_from,
            const PackageDepSpecList & no_blockers_from,
            const PackageDepSpecList & no_dependencies_from,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const SanitisedDependency & dep)
    {
        CareAboutDepFnVisitor v(env, resolution_options, no_blockers_from, no_dependencies_from, dep);
        if (resolution->decision()->accept_returning<bool>(v))
        {
            bool suggestion(is_suggestion(dep)), recommendation(is_recommendation(dep));

            if (! (suggestion || recommendation))
                return si_take;

            for (PackageDepSpecList::const_iterator l(take.begin()), l_end(take.end()) ;
                    l != l_end ; ++l)
            {
                PackageDepSpec spec(*dep.spec().if_package());
                if (match_qpns(*env, *l, *spec.package_ptr()))
                    return si_take;
            }

            for (PackageDepSpecList::const_iterator l(take_from.begin()), l_end(take_from.end()) ;
                    l != l_end ; ++l)
            {
                if (match_qpns(*env, *l, resolution->resolvent().package()))
                    return si_take;
            }

            for (PackageDepSpecList::const_iterator l(ignore.begin()), l_end(ignore.end()) ;
                    l != l_end ; ++l)
            {
                PackageDepSpec spec(*dep.spec().if_package());
                if (match_qpns(*env, *l, *spec.package_ptr()))
                    return si_ignore;
            }

            for (PackageDepSpecList::const_iterator l(ignore_from.begin()), l_end(ignore_from.end()) ;
                    l != l_end ; ++l)
            {
                if (match_qpns(*env, *l, resolution->resolvent().package()))
                    return si_ignore;
            }

            if (suggestion)
            {
                if (resolution_options.a_suggestions.argument() == "take")
                    return si_take;
                else if (resolution_options.a_suggestions.argument() == "ignore")
                    return si_ignore;
            }

            if (recommendation)
            {
                if (resolution_options.a_recommendations.argument() == "take")
                    return si_take;
                else if (resolution_options.a_recommendations.argument() == "ignore")
                    return si_ignore;
            }

            /* we also take suggestions and recommendations that have already been installed */
            if (dep.spec().if_package())
            {
                const std::tr1::shared_ptr<const PackageIDSequence> installed_ids(
                        (*env)[selection::SomeArbitraryVersion(
                            generator::Matches(*dep.spec().if_package(), MatchPackageOptions()) |
                            filter::InstalledAtRoot(FSEntry("/")))]);
                if (! installed_ids->empty())
                    return si_take;
            }

            return si_untaken;
        }
        else
            return si_ignore;
    }

    const std::tr1::shared_ptr<const Repository>
    find_repository_for_fn(const Environment * const env,
            const ResolveCommandLineResolutionOptions &,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const ChangesToMakeDecision & decision)
    {
        std::tr1::shared_ptr<const Repository> result;
        for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
                r_end(env->package_database()->end_repositories()) ;
                r != r_end ; ++r)
        {
            switch (resolution->resolvent().destination_type())
            {
                case dt_install_to_slash:
                    if ((! (*r)->installed_root_key()) || ((*r)->installed_root_key()->value() != FSEntry("/")))
                        continue;
                    break;

                case dt_create_binary:
                    if ((*r)->installed_root_key())
                        continue;
                    break;

                case last_dt:
                    break;
            }

            if ((*r)->destination_interface() &&
                    (*r)->destination_interface()->is_suitable_destination_for(*decision.origin_id()))
            {
                if (result)
                    throw ConfigurationError("For '" + stringify(*decision.origin_id())
                            + "' with destination type " + stringify(resolution->resolvent().destination_type())
                            + ", don't know whether to install to ::" + stringify(result->name())
                            + " or ::" + stringify((*r)->name()));
                else
                    result = *r;
            }
        }

        if (! result)
            throw ConfigurationError("No repository suitable for '" + stringify(*decision.origin_id())
                    + "' with destination type " + stringify(resolution->resolvent().destination_type()) + " has been configured");
        return result;
    }

    Filter make_destination_filter_fn(const Resolvent & resolvent)
    {
        switch (resolvent.destination_type())
        {
            case dt_install_to_slash:
                return filter::InstalledAtRoot(FSEntry("/"));

            case dt_create_binary:
                throw InternalError(PALUDIS_HERE, "no dt_create_binary yet");

            case last_dt:
                break;
        }

        throw InternalError(PALUDIS_HERE, "unhandled dt");
    }

    bool match_any(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::tr1::shared_ptr<const PackageID> & i)
    {
        for (PackageDepSpecList::const_iterator l(list.begin()), l_end(list.end()) ;
                l != l_end ; ++l)
            if (match_package(*env, *l, *i, MatchPackageOptions()))
                return true;

        return false;
    }

    struct AllowedToRemoveVisitor
    {
        bool visit(const DependentReason &) const
        {
            return true;
        }

        bool visit(const TargetReason &) const
        {
            return true;
        }

        bool visit(const DependencyReason &) const
        {
            return false;
        }

        bool visit(const WasUsedByReason &) const
        {
            return true;
        }

        bool visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<bool>(*this);
        }

        bool visit(const PresetReason &) const
        {
            return false;
        }
    };

    bool allowed_to_remove_fn(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const std::tr1::shared_ptr<const PackageID> & i)
    {
        for (Constraints::ConstIterator c(resolution->constraints()->begin()),
                c_end(resolution->constraints()->end()) ;
                c != c_end ; ++c)
            if ((*c)->reason()->accept_returning<bool>(AllowedToRemoveVisitor()))
                return true;

        return match_any(env, list, i);
    }

    bool remove_if_dependent_fn(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::tr1::shared_ptr<const PackageID> & i)
    {
        return match_any(env, list, i);
    }

    bool prefer_or_avoid_one(const Environment * const, const QualifiedPackageName & q, const PackageDepSpec & s)
    {
        Context context("When working out whether we favour or avoid '" + stringify(q) + "' due to '"
                + stringify(s) + "':");

        if (! package_dep_spec_has_properties(s, make_named_values<PackageDepSpecProperties>(
                        n::has_additional_requirements() = false,
                        n::has_category_name_part() = false,
                        n::has_from_repository() = false,
                        n::has_in_repository() = false,
                        n::has_installable_to_path() = false,
                        n::has_installable_to_repository() = false,
                        n::has_installed_at_path() = false,
                        n::has_package() = true,
                        n::has_package_name_part() = false,
                        n::has_slot_requirement() = false,
                        n::has_tag() = indeterminate,
                        n::has_version_requirements() = false
                        )))
            throw args::DoHelp("'" + stringify(s) + "' is not a simple cat/pkg");
        return *s.package_ptr() == q;
    }

    Tribool prefer_or_avoid_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions &,
            const PackageDepSpecList & favour,
            const PackageDepSpecList & avoid,
            const QualifiedPackageName & q)
    {
        if (favour.end() != std::find_if(favour.begin(), favour.end(),
                    std::tr1::bind(&prefer_or_avoid_one, env, q, std::tr1::placeholders::_1)))
            return true;
        if (avoid.end() != std::find_if(avoid.begin(), avoid.end(),
                    std::tr1::bind(&prefer_or_avoid_one, env, q, std::tr1::placeholders::_1)))
            return false;
        return indeterminate;
    }

    struct ChosenIDVisitor
    {
        const std::tr1::shared_ptr<const PackageID> visit(const ChangesToMakeDecision & decision) const
        {
            return decision.origin_id();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const BreakDecision & decision) const
        {
            return decision.existing_id();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const ExistingNoChangeDecision & decision) const
        {
            return decision.existing_id();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const NothingNoChangeDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const RemoveDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const UnableToMakeDecision &) const
        {
            return make_null_shared_ptr();
        }
    };

    struct ConfirmFnVisitor
    {
        const Environment * const env;
        const ResolveCommandLineResolutionOptions & resolution_options;
        const PackageDepSpecList & permit_downgrade;
        const PackageDepSpecList & permit_old_version;
        const PackageDepSpecList & allowed_to_break_specs;
        const bool allowed_to_break_system;
        const std::tr1::shared_ptr<const PackageID> id;

        ConfirmFnVisitor(const Environment * const e,
                const ResolveCommandLineResolutionOptions & r,
                const PackageDepSpecList & d,
                const PackageDepSpecList & o,
                const PackageDepSpecList & a,
                const bool s,
                const std::tr1::shared_ptr<const PackageID> & i) :
            env(e),
            resolution_options(r),
            permit_downgrade(d),
            permit_old_version(o),
            allowed_to_break_specs(a),
            allowed_to_break_system(s),
            id(i)
        {
        }

        bool visit(const DowngradeConfirmation &) const
        {
            if (id)
                for (PackageDepSpecList::const_iterator l(permit_downgrade.begin()), l_end(permit_downgrade.end()) ;
                        l != l_end ; ++l)
                {
                    if (match_package(*env, *l, *id, MatchPackageOptions()))
                        return true;
                }

            return false;
        }

        bool visit(const NotBestConfirmation &) const
        {
            if (id)
                for (PackageDepSpecList::const_iterator l(permit_old_version.begin()), l_end(permit_old_version.end()) ;
                        l != l_end ; ++l)
                {
                    if (match_package(*env, *l, *id, MatchPackageOptions()))
                        return true;
                }

            return false;
        }

        bool visit(const BreakConfirmation &) const
        {
            if (id)
                for (PackageDepSpecList::const_iterator l(allowed_to_break_specs.begin()), l_end(allowed_to_break_specs.end()) ;
                        l != l_end ; ++l)
                {
                    if (match_package(*env, *l, *id, MatchPackageOptions()))
                        return true;
                }

            return false;
        }

        bool visit(const RemoveSystemPackageConfirmation &) const
        {
            return allowed_to_break_system;
        }
    };

    bool confirm_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const PackageDepSpecList & permit_downgrade,
            const PackageDepSpecList & permit_old_version,
            const PackageDepSpecList & allowed_to_break_specs,
            const bool allowed_to_break_system,
            const std::tr1::shared_ptr<const Resolution> & r,
            const std::tr1::shared_ptr<const RequiredConfirmation> & c)
    {
        return c->accept_returning<bool>(ConfirmFnVisitor(env, resolution_options, permit_downgrade, permit_old_version,
                    allowed_to_break_specs, allowed_to_break_system,
                    r->decision()->accept_returning<std::tr1::shared_ptr<const PackageID> >(ChosenIDVisitor())
                    ));
    }

    const std::tr1::shared_ptr<ConstraintSequence> get_constraints_for_dependent_fn(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::tr1::shared_ptr<const Resolution> &,
            const std::tr1::shared_ptr<const PackageID> & id,
            const std::tr1::shared_ptr<const PackageIDSequence> & ids)
    {
        const std::tr1::shared_ptr<ConstraintSequence> result(new ConstraintSequence);

        std::tr1::shared_ptr<PackageDepSpec> spec;
        if (match_any(env, list, id))
            spec = make_shared_copy(id->uniquely_identifying_spec());
        else
        {
            PartiallyMadePackageDepSpec partial_spec((PartiallyMadePackageDepSpecOptions()));
            partial_spec.package(id->name());
            if (id->slot_key())
                partial_spec.slot_requirement(make_shared_ptr(new ELikeSlotExactRequirement(
                                id->slot_key()->value(), false)));
            spec = make_shared_ptr(new PackageDepSpec(partial_spec));
        }

        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            const std::tr1::shared_ptr<DependentReason> reason(new DependentReason(*i));

            result->push_back(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                                n::destination_type() = dt_install_to_slash,
                                n::nothing_is_fine_too() = true,
                                n::reason() = reason,
                                n::spec() = BlockDepSpec("!" + stringify(*spec), *spec, false),
                                n::untaken() = false,
                                n::use_existing() = ue_if_possible
                                ))));
        }

        return result;
    }

    const std::tr1::shared_ptr<ConstraintSequence> get_constraints_for_purge_fn(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::tr1::shared_ptr<const Resolution> &,
            const std::tr1::shared_ptr<const PackageID> & id,
            const std::tr1::shared_ptr<const PackageIDSequence> & ids)
    {
        const std::tr1::shared_ptr<ConstraintSequence> result(new ConstraintSequence);

        PartiallyMadePackageDepSpec partial_spec((PartiallyMadePackageDepSpecOptions()));
        partial_spec.package(id->name());
        if (id->slot_key())
            partial_spec.slot_requirement(make_shared_ptr(new ELikeSlotExactRequirement(
                            id->slot_key()->value(), false)));
        PackageDepSpec spec(partial_spec);

        const std::tr1::shared_ptr<WasUsedByReason> reason(new WasUsedByReason(ids));

        result->push_back(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                            n::destination_type() = dt_install_to_slash,
                            n::nothing_is_fine_too() = true,
                            n::reason() = reason,
                            n::spec() = BlockDepSpec("!" + stringify(spec), spec, false),
                            n::untaken() = ! match_any(env, list, id),
                            n::use_existing() = ue_if_possible
                            ))));

        return result;
    }

    bool can_use_fn(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::tr1::shared_ptr<const PackageID> & id)
    {
        return ! match_any(env, list, id);
    }

    void serialise_resolved(StringListStream & ser_stream, const Resolved & resolved)
    {
        Serialiser ser(ser_stream);
        resolved.serialise(ser);
        ser_stream.nothing_more_to_write();
    }

    int display_resolution(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolved> & resolved,
            const ResolveCommandLineResolutionOptions &,
            const ResolveCommandLineDisplayOptions & display_options,
            const ResolveCommandLineProgramOptions & program_options,
            const std::tr1::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
            const std::tr1::shared_ptr<const Sequence<std::string> > & targets)
    {
        Context context("When displaying chosen resolution:");

        StringListStream ser_stream;
        Thread ser_thread(std::tr1::bind(&serialise_resolved,
                    std::tr1::ref(ser_stream),
                    std::tr1::cref(*resolved)));

        std::tr1::shared_ptr<Sequence<std::string> > args(new Sequence<std::string>);

        for (args::ArgsSection::GroupsConstIterator g(display_options.begin()), g_end(display_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                {
                    const std::tr1::shared_ptr<const Sequence<std::string> > f((*o)->forwardable_args());
                    std::copy(f->begin(), f->end(), args->back_inserter());
                }
        }

        for (Sequence<std::string>::ConstIterator p(targets->begin()), p_end(targets->end()) ;
                p != p_end ; ++p)
            args->push_back(*p);

        if (program_options.a_display_resolution_program.specified())
        {
            std::string command(program_options.a_display_resolution_program.argument());

            if (keys_if_import)
                for (Map<std::string, std::string>::ConstIterator k(keys_if_import->begin()),
                        k_end(keys_if_import->end()) ;
                        k != k_end ; ++k)
                {
                    args->push_back("--unpackaged-repository-params");
                    args->push_back(k->first + "=" + k->second);
                }

            for (Sequence<std::string>::ConstIterator a(args->begin()), a_end(args->end()) ;
                    a != a_end ; ++a)
                command = command + " " + args::escape(*a);

            paludis::Command cmd(command);
            cmd
                .with_input_stream(&ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

            return run_command(cmd);
        }
        else
            return DisplayResolutionCommand().run(env, args, resolved);
    }

    void serialise_job_lists(StringListStream & ser_stream, const JobLists & job_lists)
    {
        Serialiser ser(ser_stream);
        job_lists.serialise(ser);
        ser_stream.nothing_more_to_write();
    }

    int perform_resolution(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<const Resolved> & resolved,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const ResolveCommandLineExecutionOptions & execution_options,
            const ResolveCommandLineProgramOptions & program_options,
            const std::tr1::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
            const std::tr1::shared_ptr<const Sequence<std::string> > & targets,
            const bool is_set)
    {
        Context context("When performing chosen resolution:");

        std::tr1::shared_ptr<Sequence<std::string> > args(new Sequence<std::string>);

        if (is_set)
            args->push_back("--set");

        for (args::ArgsSection::GroupsConstIterator g(program_options.begin()), g_end(program_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                {
                    const std::tr1::shared_ptr<const Sequence<std::string> > f((*o)->forwardable_args());
                    std::copy(f->begin(), f->end(), args->back_inserter());
                }
        }

        for (args::ArgsSection::GroupsConstIterator g(execution_options.begin()), g_end(execution_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                {
                    const std::tr1::shared_ptr<const Sequence<std::string> > f((*o)->forwardable_args());
                    std::copy(f->begin(), f->end(), args->back_inserter());
                }
        }

        if (! resolution_options.a_execute.specified())
            args->push_back("--pretend");

        for (Sequence<std::string>::ConstIterator p(targets->begin()), p_end(targets->end()) ;
                p != p_end ; ++p)
            args->push_back(*p);

        if (program_options.a_execute_resolution_program.specified() || resolution_options.a_execute.specified())
        {
            /* backgrounding this barfs with become_command. working out why could
             * be a fun exercise for someone with way too much time on their hands.
             * */
            StringListStream ser_stream;
            serialise_job_lists(ser_stream, *resolved->job_lists());

            std::string command;
            if (program_options.a_execute_resolution_program.specified())
                command = program_options.a_execute_resolution_program.argument();
            else
                command = "$CAVE execute-resolution";

            if (keys_if_import)
                for (Map<std::string, std::string>::ConstIterator k(keys_if_import->begin()),
                        k_end(keys_if_import->end()) ;
                        k != k_end ; ++k)
                {
                    args->push_back("--unpackaged-repository-params");
                    args->push_back(k->first + "=" + k->second);
                }

            for (Sequence<std::string>::ConstIterator a(args->begin()), a_end(args->end()) ;
                    a != a_end ; ++a)
                command = command + " " + args::escape(*a);

            paludis::Command cmd(command);
            cmd
                .with_input_stream(&ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

            become_command(cmd);
        }
        else
            return ExecuteResolutionCommand().run(env, args, resolved->job_lists());
    }

    struct KindNameVisitor
    {
        const std::string visit(const RemoveDecision &) const
        {
            return "remove_decision";
        }

        const std::string visit(const BreakDecision &) const
        {
            return "break_decision";
        }

        const std::string visit(const UnableToMakeDecision &) const
        {
            return "unable_to_make_decision";
        }

        const std::string visit(const NothingNoChangeDecision &) const
        {
            return "nothing_no_change";
        }

        const std::string visit(const ExistingNoChangeDecision &) const
        {
            return "existing_no_change";
        }

        const std::string visit(const ChangesToMakeDecision &) const
        {
            return "changes_to_make";
        }
    };

    struct ShortReasonName
    {
        const std::string visit(const DependencyReason & r) const
        {
            return "from " + stringify(*r.from_id()) + " dependency " + (r.sanitised_dependency().spec().if_package() ?
                    stringify(*r.sanitised_dependency().spec().if_package()) : stringify(*r.sanitised_dependency().spec().if_block()));
        }

        const std::string visit(const WasUsedByReason & r) const
        {
            if (r.ids_being_removed()->empty())
                return "from was unused";
            else
                return "from was used by " + join(indirect_iterator(r.ids_being_removed()->begin()),
                        indirect_iterator(r.ids_being_removed()->end()), ", ");
        }

        const std::string visit(const DependentReason & r) const
        {
            return "from dependent " + stringify(*r.id_being_removed());
        }

        const std::string visit(const TargetReason &) const
        {
            return "from target";
        }

        const std::string visit(const PresetReason & r) const
        {
            std::string result("from preset");
            if (! r.maybe_explanation().empty())
                result = result + " (" + r.maybe_explanation() + ")";
            if (r.maybe_reason_for_preset())
                result = result + " (" + r.maybe_reason_for_preset()->accept_returning<std::string>(*this) + ")";
            return result;
        }

        const std::string visit(const SetReason & r) const
        {
            return "from " + stringify(r.set_name()) + " (" + r.reason_for_set()->accept_returning<std::string>(*this) + ")";
        }
    };

    void display_restarts_if_requested(const std::list<SuggestRestart> & restarts,
            const ResolveCommandLineResolutionOptions & resolution_options)
    {
        if (! resolution_options.a_dump_restarts.specified())
            return;

        std::cout << "Dumping restarts:" << std::endl << std::endl;

        for (std::list<SuggestRestart>::const_iterator r(restarts.begin()), r_end(restarts.end()) ;
                r != r_end ; ++r)
        {
            std::cout << "* " << r->resolvent() << std::endl;

            std::cout << "    Had decided upon ";
            const std::tr1::shared_ptr<const PackageID> id(r->previous_decision()->accept_returning<
                    std::tr1::shared_ptr<const PackageID> >(ChosenIDVisitor()));
            if (id)
                std::cout << *id;
            else
                std::cout << r->previous_decision()->accept_returning<std::string>(KindNameVisitor());
            std::cout << std::endl;

            std::cout << "    Which did not satisfy " << r->problematic_constraint()->spec()
                << ", use existing " << r->problematic_constraint()->use_existing();
            if (r->problematic_constraint()->nothing_is_fine_too())
                std::cout << ", nothing is fine too";
            std::cout << " " << r->problematic_constraint()->reason()->accept_returning<std::string>(ShortReasonName());
            std::cout << std::endl;
        }

        std::cout << std::endl;
    }
}

int
paludis::cave::resolve_common(
        const std::tr1::shared_ptr<Environment> & env,
        const ResolveCommandLineResolutionOptions & resolution_options,
        const ResolveCommandLineExecutionOptions & execution_options,
        const ResolveCommandLineDisplayOptions & display_options,
        const ResolveCommandLineProgramOptions & program_options,
        const std::tr1::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
        const std::tr1::shared_ptr<const Sequence<std::string> > & targets_if_not_purge,
        const bool purge)
{
    int retcode(0);

    InitialConstraints initial_constraints;
    PackageDepSpecList allowed_to_remove_specs, allowed_to_break_specs, remove_if_dependent_specs,
                       less_restrictive_remove_blockers_specs, purge_specs, with, without,
                       permit_old_version, permit_downgrade, take, take_from, ignore, ignore_from,
                       favour, avoid, no_dependencies_from, no_blockers_from, not_usable_specs;
    bool allowed_to_break_system(false);

    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_uninstall.begin_args()),
            i_end(resolution_options.a_permit_uninstall.end_args()) ;
            i != i_end ; ++i)
        allowed_to_remove_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_uninstalls_may_break.begin_args()),
            i_end(resolution_options.a_uninstalls_may_break.end_args()) ;
            i != i_end ; ++i)
        if (*i == "system")
            allowed_to_break_system = true;
        else
            allowed_to_break_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                        UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_remove_if_dependent.begin_args()),
            i_end(resolution_options.a_remove_if_dependent.end_args()) ;
            i != i_end ; ++i)
        remove_if_dependent_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_less_restrictive_remove_blockers.begin_args()),
            i_end(resolution_options.a_less_restrictive_remove_blockers.end_args()) ;
            i != i_end ; ++i)
        less_restrictive_remove_blockers_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_purge.begin_args()),
            i_end(resolution_options.a_purge.end_args()) ;
            i != i_end ; ++i)
        purge_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_without.begin_args()),
            i_end(resolution_options.a_without.end_args()) ;
            i != i_end ; ++i)
        without.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_with.begin_args()),
            i_end(resolution_options.a_with.end_args()) ;
            i != i_end ; ++i)
        with.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_take.begin_args()),
            i_end(resolution_options.a_take.end_args()) ;
            i != i_end ; ++i)
        take.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_take_from.begin_args()),
            i_end(resolution_options.a_take_from.end_args()) ;
            i != i_end ; ++i)
        take_from.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_ignore.begin_args()),
            i_end(resolution_options.a_ignore.end_args()) ;
            i != i_end ; ++i)
        ignore.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_ignore_from.begin_args()),
            i_end(resolution_options.a_ignore_from.end_args()) ;
            i != i_end ; ++i)
        ignore_from.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_favour.begin_args()),
            i_end(resolution_options.a_favour.end_args()) ;
            i != i_end ; ++i)
        favour.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_avoid.begin_args()),
            i_end(resolution_options.a_avoid.end_args()) ;
            i != i_end ; ++i)
        avoid.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_no_dependencies_from.begin_args()),
            i_end(resolution_options.a_no_dependencies_from.end_args()) ;
            i != i_end ; ++i)
        no_dependencies_from.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_no_blockers_from.begin_args()),
            i_end(resolution_options.a_no_blockers_from.end_args()) ;
            i != i_end ; ++i)
        no_blockers_from.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_downgrade.begin_args()),
            i_end(resolution_options.a_permit_downgrade.end_args()) ;
            i != i_end ; ++i)
        permit_downgrade.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_old_version.begin_args()),
            i_end(resolution_options.a_permit_old_version.end_args()) ;
            i != i_end ; ++i)
        permit_old_version.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_not_usable.begin_args()),
            i_end(resolution_options.a_not_usable.end_args()) ;
            i != i_end ; ++i)
        not_usable_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    std::tr1::shared_ptr<Generator> all_binary_repos_generator;
    for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
            r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->destination_interface() && ! (*r)->installed_root_key())
        {
            if (all_binary_repos_generator)
                all_binary_repos_generator.reset(new generator::Intersection(*all_binary_repos_generator,
                            generator::InRepository((*r)->name())));
            else
                all_binary_repos_generator.reset(new generator::InRepository((*r)->name()));
        }

    if (! all_binary_repos_generator)
        all_binary_repos_generator.reset(new generator::Nothing());

    for (args::StringSetArg::ConstIterator i(resolution_options.a_preset.begin_args()),
            i_end(resolution_options.a_preset.end_args()) ;
            i != i_end ; ++i)
    {
        const std::tr1::shared_ptr<const Reason> reason(new PresetReason("preset", make_null_shared_ptr()));
        PackageDepSpec spec(parse_user_package_dep_spec(*i, env.get(), UserPackageDepSpecOptions()));
        const std::tr1::shared_ptr<const Resolvents> resolvents(get_resolvents_for_fn(
                    env.get(), resolution_options, spec, make_null_shared_ptr(), reason));

        if (resolvents->empty())
            throw args::DoHelp("Preset '" + *i + "' has no resolvents");

        for (Resolvents::ConstIterator r(resolvents->begin()), r_end(resolvents->end()) ;
                r != r_end ; ++r)
        {
            const std::tr1::shared_ptr<Constraint> constraint(new Constraint(make_named_values<Constraint>(
                            n::destination_type() = r->destination_type(),
                            n::nothing_is_fine_too() = true,
                            n::reason() = reason,
                            n::spec() = spec,
                            n::untaken() = false,
                            n::use_existing() = ue_if_possible
                            )));
            initial_constraints.insert(std::make_pair(*r, make_initial_constraints_for(
                            env.get(), resolution_options, all_binary_repos_generator, without, *r))).first->second->add(
                    constraint);
        }
    }

    using std::tr1::placeholders::_1;
    using std::tr1::placeholders::_2;
    using std::tr1::placeholders::_3;
    using std::tr1::placeholders::_4;

    ResolverFunctions resolver_functions(make_named_values<ResolverFunctions>(
                n::allowed_to_remove_fn() = std::tr1::bind(&allowed_to_remove_fn,
                        env.get(), std::tr1::cref(allowed_to_remove_specs), _1, _2),
                n::can_use_fn() = std::tr1::bind(&can_use_fn,
                    env.get(), std::tr1::cref(not_usable_specs), _1),
                n::confirm_fn() = std::tr1::bind(&confirm_fn,
                        env.get(), std::tr1::cref(resolution_options), std::tr1::cref(permit_downgrade),
                        std::tr1::cref(permit_old_version), std::tr1::cref(allowed_to_break_specs),
                        allowed_to_break_system, _1, _2),
                n::find_repository_for_fn() = std::tr1::bind(&find_repository_for_fn,
                        env.get(), std::tr1::cref(resolution_options), _1, _2),
                n::get_constraints_for_dependent_fn() = std::tr1::bind(&get_constraints_for_dependent_fn,
                        env.get(), std::tr1::cref(less_restrictive_remove_blockers_specs), _1, _2, _3),
                n::get_constraints_for_purge_fn() = std::tr1::bind(&get_constraints_for_purge_fn,
                        env.get(), std::tr1::cref(purge_specs), _1, _2, _3),
                n::get_destination_types_for_fn() = std::tr1::bind(&get_destination_types_for_fn,
                        env.get(), std::tr1::cref(resolution_options), _1, _2, _3),
                n::get_initial_constraints_for_fn() = std::tr1::bind(&initial_constraints_for_fn,
                        env.get(), std::tr1::cref(resolution_options), std::tr1::cref(without),
                        std::tr1::cref(initial_constraints), all_binary_repos_generator, _1),
                n::get_resolvents_for_fn() = std::tr1::bind(&get_resolvents_for_fn,
                        env.get(), std::tr1::cref(resolution_options), _1, _2, _3),
                n::get_use_existing_fn() = std::tr1::bind(&use_existing_fn,
                        env.get(), std::tr1::cref(resolution_options), std::tr1::cref(without), std::tr1::cref(with), _1, _2, _3),
                n::interest_in_spec_fn() = std::tr1::bind(&interest_in_spec_fn,
                        env.get(), std::tr1::cref(resolution_options), std::tr1::cref(take), std::tr1::cref(take_from),
                        std::tr1::cref(ignore), std::tr1::cref(ignore_from), std::tr1::cref(no_blockers_from),
                        std::tr1::cref(no_dependencies_from), _1, _2),
                n::make_destination_filtered_generator_fn() = std::tr1::bind(&make_destination_filtered_generator,
                        env.get(), std::tr1::cref(resolution_options), all_binary_repos_generator, _1, _2),
                n::prefer_or_avoid_fn() = std::tr1::bind(&prefer_or_avoid_fn,
                        env.get(), std::tr1::cref(resolution_options), std::tr1::cref(favour), std::tr1::cref(avoid), _1),
                n::remove_if_dependent_fn() = std::tr1::bind(&remove_if_dependent_fn,
                        env.get(), std::tr1::cref(remove_if_dependent_specs), _1)
                ));

    ScopedSelectionCache selection_cache(env.get());
    std::tr1::shared_ptr<Resolver> resolver(new Resolver(env.get(), resolver_functions));
    bool is_set(false);
    std::list<SuggestRestart> restarts;

    try
    {
        {
            DisplayCallback display_callback("Resolving: ");
            ScopedNotifierCallback display_callback_holder(env.get(),
                    NotifierCallbackFunction(std::tr1::cref(display_callback)));

            while (true)
            {
                try
                {
                    if (purge)
                        resolver->purge();
                    else
                        add_resolver_targets(env, resolver, resolution_options, targets_if_not_purge, is_set);
                    resolver->resolve();
                    break;
                }
                catch (const SuggestRestart & e)
                {
                    restarts.push_back(e);
                    display_callback(ResolverRestart());
                    initial_constraints.insert(std::make_pair(e.resolvent(), make_initial_constraints_for(
                                    env.get(), resolution_options, all_binary_repos_generator, without, e.resolvent()))).first->second->add(
                            e.suggested_preset());
                    resolver = make_shared_ptr(new Resolver(env.get(), resolver_functions));

                    if (restarts.size() > 9000)
                        throw InternalError(PALUDIS_HERE, "Restarted over nine thousand times. Something's "
                                "probably gone horribly wrong. Consider using --dump-restarts and having "
                                "a look.");
                }
            }
        }

        if (! restarts.empty())
            display_restarts_if_requested(restarts, resolution_options);

        dump_if_requested(env, resolver, resolution_options);

        retcode |= display_resolution(env, resolver->resolved(), resolution_options,
                display_options, program_options, keys_if_import,
                purge ? make_shared_ptr(new const Sequence<std::string>) : targets_if_not_purge);

        if (! resolver->resolved()->taken_unable_to_make_decisions()->empty())
            retcode |= 1;

        if (! resolver->resolved()->taken_unconfirmed_decisions()->empty())
            retcode |= 2;

        if (! resolver->resolved()->taken_unorderable_decisions()->empty())
            retcode |= 4;

        if (0 == retcode)
            return perform_resolution(env, resolver->resolved(), resolution_options,
                    execution_options, program_options, keys_if_import,
                    purge ? make_shared_ptr(new const Sequence<std::string>) : targets_if_not_purge,
                    is_set);
    }
    catch (...)
    {
        if (! restarts.empty())
            display_restarts_if_requested(restarts, resolution_options);

        dump_if_requested(env, resolver, resolution_options);
        throw;
    }

    return retcode | 1;
}

