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
#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/resolver_lists.hh>
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

    bool is_buildish_dep(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->empty())
            throw InternalError(PALUDIS_HERE, "not implemented");

        LabelTypesVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->end()),
                accept_visitor(v));
        return v.seen_buildish_dep;
    }

    bool is_runish_dep(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->empty())
            throw InternalError(PALUDIS_HERE, "not implemented");

        LabelTypesVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->end()),
                accept_visitor(v));
        return v.seen_runish_dep;
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
            DestinationTypes result;

            if (resolution_options.a_create_binaries.specified())
            {
                bool b(true);

                if (resolution_options.a_no_binaries_for.specified() && package_id)
                {
                    for (args::StringSetArg::ConstIterator a(resolution_options.a_no_binaries_for.begin_args()),
                            a_end(resolution_options.a_no_binaries_for.end_args()) ;
                            a != a_end ; ++a)
                        if (match_package(*env,
                                    parse_user_package_dep_spec(*a, env, UserPackageDepSpecOptions() + updso_allow_wildcards),
                                    *package_id, MatchPackageOptions()))
                        {
                            b = false;
                            break;
                        }
                }

                if (b)
                    result += dt_create_binary;
            }

            if (resolution_options.a_install_to_root.specified())
                result += dt_install_to_slash;

            if (result.none())
                result += dt_install_to_slash;

            return result;
        }

        DestinationTypes visit(const DependencyReason & reason) const
        {
            DestinationTypes result;

            bool is_buildish(is_buildish_dep(reason.sanitised_dependency())),
                 is_runish(is_runish_dep(reason.sanitised_dependency()));

            if ((! is_buildish) && (! is_runish))
                throw InternalError(PALUDIS_HERE, "not buildish or runish. eek. labels are { "
                        + join(indirect_iterator(reason.sanitised_dependency().active_dependency_labels()->begin()),
                            indirect_iterator(reason.sanitised_dependency().active_dependency_labels()->end()), ", ")
                        + " }");

            if (is_buildish)
                result += dt_install_to_slash;
            if (is_runish)
                result |= visit(TargetReason());

            return result;
        }

        DestinationTypes visit(const PresetReason &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "not sure what to do here yet");
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
            const ResolveCommandLineResolutionOptions & resolution_options,
            const Generator & g,
            const Resolvent & r)
    {
        switch (r.destination_type())
        {
            case dt_install_to_slash:
                return g | filter::InstalledAtRoot(FSEntry("/"));

            case dt_create_binary:
                {
                    std::tr1::shared_ptr<Generator> generator;
                    for (args::StringSetArg::ConstIterator a(resolution_options.a_create_binaries.begin_args()),
                            a_end(resolution_options.a_create_binaries.end_args()) ;
                            a != a_end ; ++a)
                    {
                        if (! generator)
                            generator.reset(new generator::InRepository(RepositoryName(*a)));
                        else
                            generator.reset(new generator::Intersection(*generator, generator::InRepository(RepositoryName(*a))));
                    }

                    if (! generator)
                        throw args::DoHelp("No binary destinations were specified");
                    else
                        return g & *generator;
                }

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
            PackageDepSpecList & allowed_to_remove_specs,
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
                    allowed_to_remove_specs.push_back(s);
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

    UseExisting use_existing_fn(const ResolveCommandLineResolutionOptions & resolution_options,
            const Resolvent &,
            const PackageDepSpec &,
            const std::tr1::shared_ptr<const Reason> & reason)
    {
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

    bool installed_is_scm_older_than(const Environment * const env, const ResolveCommandLineResolutionOptions & resolution_options,
            const Resolvent & q, const int n)
    {
        Context context("When working out whether '" + stringify(q) + "' has installed SCM packages:");

        const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsUnsorted(
                    make_destination_filtered_generator(env, resolution_options, generator::Package(q.package()), q) |
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
            const Resolvent & resolvent)
    {
        const std::tr1::shared_ptr<Constraints> result(new Constraints);

        int n(reinstall_scm_days(resolution_options));
        if ((-1 != n) && installed_is_scm_older_than(env, resolution_options, resolvent, n))
        {
            result->add(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                                value_for<n::destination_type>(resolvent.destination_type()),
                                value_for<n::nothing_is_fine_too>(false),
                                value_for<n::reason>(make_shared_ptr(new PresetReason)),
                                value_for<n::spec>(make_package_dep_spec(PartiallyMadePackageDepSpecOptions()).package(resolvent.package())),
                                value_for<n::untaken>(false),
                                value_for<n::use_existing>(ue_only_if_transient)
                                ))));
        }

        return result;
    }

    const std::tr1::shared_ptr<Constraints> initial_constraints_for_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const InitialConstraints & initial_constraints,
            const Resolvent & resolvent)
    {
        InitialConstraints::const_iterator i(initial_constraints.find(resolvent));
        if (i == initial_constraints.end())
            return make_initial_constraints_for(env, resolution_options, resolvent);
        else
            return i->second;
    }

    struct IsTargetVisitor
    {
        bool visit(const DependencyReason &) const
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

    struct CareAboutDepFnVisitor
    {
        const ResolveCommandLineResolutionOptions & resolution_options;
        const SanitisedDependency dep;

        CareAboutDepFnVisitor(const ResolveCommandLineResolutionOptions & c, const SanitisedDependency & d) :
            resolution_options(c),
            dep(d)
        {
        }

        bool visit(const ExistingNoChangeDecision &) const
        {
            if (! is_enabled_dep(dep))
                return false;

            if (! resolution_options.a_follow_installed_build_dependencies.specified())
                if (is_just_build_dep(dep))
                    return false;
            if (resolution_options.a_ignore_installed_dependencies.specified())
                if (! is_compiled_against_dep(dep))
                    return false;

            if (is_suggestion(dep) || is_recommendation(dep))
            {
                /* should only return false if the dep's not already installedish */
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

        bool visit(const ChangesToMakeDecision &) const
        {
            if (is_enabled_dep(dep))
                return true;

            return false;
        }
    };

    bool care_about_dep_fn(const Environment * const, const ResolveCommandLineResolutionOptions & resolution_options,
            const Resolvent &, const std::tr1::shared_ptr<const Resolution> & resolution,
            const SanitisedDependency & dep)
    {
        CareAboutDepFnVisitor v(resolution_options, dep);
        return resolution->decision()->accept_returning<bool>(v);
    }

    bool
    take_dependency_fn(const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const Resolvent & resolvent,
            const SanitisedDependency & dep,
            const std::tr1::shared_ptr<const Reason> &)
    {
        bool suggestion(is_suggestion(dep)), recommendation(is_recommendation(dep));

        if (suggestion || recommendation)
        {
            for (args::StringSetArg::ConstIterator a(resolution_options.a_take.begin_args()),
                    a_end(resolution_options.a_take.end_args()) ;
                    a != a_end ; ++a)
            {
                PackageDepSpec user_spec(parse_user_package_dep_spec(*a, env, UserPackageDepSpecOptions() + updso_allow_wildcards));
                PackageDepSpec spec(*dep.spec().if_package());
                if (match_qpns(*env, user_spec, *spec.package_ptr()))
                    return true;
            }

            for (args::StringSetArg::ConstIterator a(resolution_options.a_take_from.begin_args()),
                    a_end(resolution_options.a_take_from.end_args()) ;
                    a != a_end ; ++a)
            {
                PackageDepSpec user_spec(parse_user_package_dep_spec(*a, env, UserPackageDepSpecOptions() + updso_allow_wildcards));
                if (match_qpns(*env, user_spec, resolvent.package()))
                    return true;
            }

            for (args::StringSetArg::ConstIterator a(resolution_options.a_discard.begin_args()),
                    a_end(resolution_options.a_discard.end_args()) ;
                    a != a_end ; ++a)
            {
                PackageDepSpec user_spec(parse_user_package_dep_spec(*a, env, UserPackageDepSpecOptions() + updso_allow_wildcards));
                PackageDepSpec spec(*dep.spec().if_package());
                if (match_qpns(*env, user_spec, *spec.package_ptr()))
                    return false;
            }

            for (args::StringSetArg::ConstIterator a(resolution_options.a_discard_from.begin_args()),
                    a_end(resolution_options.a_discard_from.end_args()) ;
                    a != a_end ; ++a)
            {
                PackageDepSpec user_spec(parse_user_package_dep_spec(*a, env, UserPackageDepSpecOptions() + updso_allow_wildcards));
                if (match_qpns(*env, user_spec, resolvent.package()))
                    return false;
            }
        }
        if (suggestion)
        {
            if (resolution_options.a_suggestions.argument() == "take")
            {
                return true;
            }
            return false;
        }
        if (recommendation)
        {
            if (resolution_options.a_recommendations.argument() == "take")
            {
                return true;
            }
            return false;
        }

        return true;
    }

    const std::tr1::shared_ptr<const Repository>
    find_repository_for_fn(const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const Resolvent & resolvent,
            const std::tr1::shared_ptr<const Resolution> &,
            const ChangesToMakeDecision & decision)
    {
        std::tr1::shared_ptr<const Repository> result;
        for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
                r_end(env->package_database()->end_repositories()) ;
                r != r_end ; ++r)
        {
            switch (resolvent.destination_type())
            {
                case dt_install_to_slash:
                    if ((! (*r)->installed_root_key()) || ((*r)->installed_root_key()->value() != FSEntry("/")))
                        continue;
                    break;

                case dt_create_binary:
                    if (resolution_options.a_create_binaries.end_args() == std::find(
                                resolution_options.a_create_binaries.begin_args(),
                                resolution_options.a_create_binaries.end_args(),
                                stringify((*r)->name())))
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
                            + "' with destination type " + stringify(resolvent.destination_type())
                            + ", don't know whether to install to ::" + stringify(result->name())
                            + " or ::" + stringify((*r)->name()));
                else
                    result = *r;
            }
        }

        if (! result)
            throw ConfigurationError("No repository suitable for '" + stringify(*decision.origin_id())
                    + "' with destination type " + stringify(resolvent.destination_type()) + " has been configured");
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

    bool allowed_to_remove_fn(
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

    void ser_thread_func(StringListStream & ser_stream, const ResolverLists & resolution_lists)
    {
        Serialiser ser(ser_stream);
        resolution_lists.serialise(ser);
        ser_stream.nothing_more_to_write();
    }

    int display_resolution(
            const std::tr1::shared_ptr<Environment> &,
            const ResolverLists & resolution_lists,
            const ResolveCommandLineResolutionOptions &,
            const ResolveCommandLineDisplayOptions & display_options,
            const ResolveCommandLineProgramOptions & program_options,
            const std::tr1::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
            const std::tr1::shared_ptr<const Sequence<std::string> > & targets)
    {
        Context context("When displaying chosen resolution:");

        StringListStream ser_stream;
        Thread ser_thread(std::tr1::bind(&ser_thread_func,
                    std::tr1::ref(ser_stream),
                    std::tr1::cref(resolution_lists)));

        std::string command(program_options.a_display_resolution_program.argument());
        if (command.empty())
            command = "$CAVE display-resolution";

        for (args::ArgsSection::GroupsConstIterator g(display_options.begin()), g_end(display_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                    command = command + " " + (*o)->forwardable_string();
        }

        for (Sequence<std::string>::ConstIterator p(targets->begin()), p_end(targets->end()) ;
                p != p_end ; ++p)
            command = command + " " + args::escape(*p);

        if (keys_if_import)
            for (Map<std::string, std::string>::ConstIterator k(keys_if_import->begin()),
                    k_end(keys_if_import->end()) ;
                    k != k_end ; ++k)
                command = command + " --unpackaged-repository-params '" + k->first + "=" + k->second + "'";

        paludis::Command cmd(command);
        cmd
            .with_input_stream(&ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

        return run_command(cmd);
    }

    void perform_resolution(
            const std::tr1::shared_ptr<Environment> &,
            const ResolverLists & resolution_lists,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const ResolveCommandLineExecutionOptions & execution_options,
            const ResolveCommandLineProgramOptions & program_options,
            const std::tr1::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
            const std::tr1::shared_ptr<const Sequence<std::string> > & targets,
            const bool is_set) PALUDIS_ATTRIBUTE((noreturn));

    void perform_resolution(
            const std::tr1::shared_ptr<Environment> &,
            const ResolverLists & resolution_lists,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const ResolveCommandLineExecutionOptions & execution_options,
            const ResolveCommandLineProgramOptions & program_options,
            const std::tr1::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
            const std::tr1::shared_ptr<const Sequence<std::string> > & targets,
            const bool is_set)
    {
        Context context("When performing chosen resolution:");

        StringListStream ser_stream;
        Serialiser ser(ser_stream);
        resolution_lists.serialise(ser);

        /* backgrounding this barfs with become_command. working out why could
         * be a fun exercise for someone with way too much time on their hands.
         * */
        ser_thread_func(ser_stream, resolution_lists);

        std::string command(program_options.a_execute_resolution_program.argument());
        if (command.empty())
            command = "$CAVE execute-resolution";

        for (args::ArgsSection::GroupsConstIterator g(execution_options.begin()), g_end(execution_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                    command = command + " " + (*o)->forwardable_string();
        }

        if (is_set)
            command.append(" --set");

        for (args::ArgsSection::GroupsConstIterator g(program_options.begin()), g_end(program_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                    command = command + " " + (*o)->forwardable_string();
        }

        if (! resolution_options.a_execute.specified())
            command = command + " --pretend";

        for (Sequence<std::string>::ConstIterator p(targets->begin()), p_end(targets->end()) ;
                p != p_end ; ++p)
            command = command + " " + args::escape(*p);

        if (keys_if_import)
            for (Map<std::string, std::string>::ConstIterator k(keys_if_import->begin()),
                    k_end(keys_if_import->end()) ;
                    k != k_end ; ++k)
                command = command + " --unpackaged-repository-params '" + k->first + "=" + k->second + "'";

        paludis::Command cmd(command);
        cmd
            .with_input_stream(&ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

        become_command(cmd);
    }

    struct ChosenIDVisitor
    {
        const std::tr1::shared_ptr<const PackageID> visit(const ChangesToMakeDecision & decision) const
        {
            return decision.origin_id();
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

    struct KindNameVisitor
    {
        const std::string visit(const RemoveDecision &) const
        {
            return "remove_decision";
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
        const std::tr1::shared_ptr<const Sequence<std::string> > & targets)
{
    int retcode(0);

    InitialConstraints initial_constraints;
    PackageDepSpecList allowed_to_remove_specs;

    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_uninstall.begin_args()),
            i_end(resolution_options.a_permit_uninstall.end_args()) ;
            i != i_end ; ++i)
        allowed_to_remove_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                    UserPackageDepSpecOptions() + updso_allow_wildcards));

    ResolverFunctions resolver_functions(make_named_values<ResolverFunctions>(
                value_for<n::allowed_to_remove_fn>(std::tr1::bind(&allowed_to_remove_fn,
                        env.get(),
                        std::tr1::cref(allowed_to_remove_specs),
                        std::tr1::placeholders::_1)),
                value_for<n::care_about_dep_fn>(std::tr1::bind(&care_about_dep_fn,
                        env.get(), std::tr1::cref(resolution_options), std::tr1::placeholders::_1,
                        std::tr1::placeholders::_2, std::tr1::placeholders::_3)),
                value_for<n::find_repository_for_fn>(std::tr1::bind(&find_repository_for_fn,
                        env.get(), std::tr1::cref(resolution_options), std::tr1::placeholders::_1, std::tr1::placeholders::_2,
                        std::tr1::placeholders::_3)),
                value_for<n::get_destination_types_for_fn>(std::tr1::bind(&get_destination_types_for_fn,
                        env.get(), std::tr1::cref(resolution_options), std::tr1::placeholders::_1, std::tr1::placeholders::_2,
                        std::tr1::placeholders::_3)),
                value_for<n::get_initial_constraints_for_fn>(std::tr1::bind(&initial_constraints_for_fn,
                        env.get(), std::tr1::cref(resolution_options), std::tr1::cref(initial_constraints), std::tr1::placeholders::_1)),
                value_for<n::get_resolvents_for_fn>(std::tr1::bind(&get_resolvents_for_fn,
                        env.get(), std::tr1::cref(resolution_options), std::tr1::placeholders::_1, std::tr1::placeholders::_2,
                        std::tr1::placeholders::_3)),
                value_for<n::get_use_existing_fn>(std::tr1::bind(&use_existing_fn,
                        std::tr1::cref(resolution_options), std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3)),
                value_for<n::make_destination_filtered_generator_fn>(std::tr1::bind(&make_destination_filtered_generator,
                        env.get(), std::tr1::cref(resolution_options), std::tr1::placeholders::_1, std::tr1::placeholders::_2)),
                value_for<n::take_dependency_fn>(std::tr1::bind(&take_dependency_fn, env.get(),
                        std::tr1::cref(resolution_options), std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3))

                ));

    ScopedSelectionCache selection_cache(env.get());
    std::tr1::shared_ptr<Resolver> resolver(new Resolver(env.get(), resolver_functions));
    bool is_set(false);
    std::list<SuggestRestart> restarts;

    try
    {
        {
            DisplayCallback display_callback;
            ScopedNotifierCallback display_callback_holder(env.get(),
                    NotifierCallbackFunction(std::tr1::cref(display_callback)));

            while (true)
            {
                try
                {
                    add_resolver_targets(env, resolver, resolution_options, targets,
                            allowed_to_remove_specs, is_set);
                    resolver->resolve();
                    break;
                }
                catch (const SuggestRestart & e)
                {
                    restarts.push_back(e);
                    display_callback(ResolverRestart());
                    initial_constraints.insert(std::make_pair(e.resolvent(), make_initial_constraints_for(
                                    env.get(), resolution_options, e.resolvent()))).first->second->add(
                            e.suggested_preset());
                    resolver = make_shared_ptr(new Resolver(env.get(), resolver_functions));
                }
            }
        }

        if (! restarts.empty())
            display_restarts_if_requested(restarts, resolution_options);

        dump_if_requested(env, resolver, resolution_options);

        retcode |= display_resolution(env, *resolver->lists(), resolution_options,
                display_options, program_options, keys_if_import, targets);

        if (! resolver->lists()->taken_error_job_ids()->empty())
            retcode |= 1;

        if (0 == retcode)
            perform_resolution(env, *resolver->lists(), resolution_options,
                    execution_options, program_options, keys_if_import, targets, is_set);
    }
    catch (...)
    {
        if (! restarts.empty())
            display_restarts_if_requested(restarts, resolution_options);

        dump_if_requested(env, resolver, resolution_options);
        throw;
    }

    return EXIT_FAILURE;
}

