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

#include "cmd_resolve.hh"
#include "cmd_resolve_cmdline.hh"
#include "cmd_resolve_display_callback.hh"
#include "cmd_resolve_dump.hh"
#include "exceptions.hh"
#include "command_command_line.hh"

#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/system.hh>
#include <paludis/args/do_help.hh>
#include <paludis/args/escape.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/resolver/qpn_s.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/destinations.hh>
#include <paludis/resolver/serialise-impl.hh>
#include <paludis/resolver/resolutions.hh>
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

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <map>

using namespace paludis;
using namespace paludis::resolver;
using namespace cave;

using std::cout;
using std::endl;

namespace
{
    void add_resolver_targets(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<Resolver> & resolver,
            const ResolveCommandLine & cmdline)
    {
        Context context("When adding targets from commandline:");

        if (cmdline.begin_parameters() == cmdline.end_parameters())
            throw args::DoHelp("Must specify at least one target");

        bool seen_sets(false), seen_packages(false);
        for (ResolveCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
                p != p_end ; ++p)
        {
            try
            {
                resolver->add_target(parse_user_package_dep_spec(*p, env.get(),
                            UserPackageDepSpecOptions() + updso_throw_if_set));
                if (seen_sets)
                    throw args::DoHelp("Cannot specify both set and package targets");
                seen_packages = true;
            }
            catch (const GotASetNotAPackageDepSpec &)
            {
                if (seen_packages)
                    throw args::DoHelp("Cannot specify both set and package targets");
                if (seen_sets)
                    throw args::DoHelp("Cannot specify multiple set targets");

                resolver->add_target(SetName(*p));
                seen_sets = true;
            }
        }
    }

    UseInstalled use_installed_from_cmdline(const args::EnumArg & a, const bool is_set)
    {
        if (a.argument() == "auto")
            return is_set ? ui_if_same : ui_never;
        else if (a.argument() == "never")
            return ui_never;
        else if (a.argument() == "if-transient")
            return ui_only_if_transient;
        else if (a.argument() == "if-same")
            return ui_if_same;
        else if (a.argument() == "if-same-version")
            return ui_if_same_version;
        else if (a.argument() == "if-possible")
            return ui_if_possible;
        else
            throw args::DoHelp("Don't understand argument '" + a.argument() + "' to '--" + a.long_name() + "'");
    }

    struct UseInstalledVisitor
    {
        const ResolveCommandLine & cmdline;
        const bool from_set;

        UseInstalledVisitor(const ResolveCommandLine & c, const bool f) :
            cmdline(c),
            from_set(f)
        {
        }

        UseInstalled visit(const DependencyReason &) const
        {
            return use_installed_from_cmdline(cmdline.resolution_options.a_keep, false);
        }

        UseInstalled visit(const TargetReason &) const
        {
            return use_installed_from_cmdline(cmdline.resolution_options.a_keep_targets, from_set);
        }

        UseInstalled visit(const PresetReason &) const
        {
            return ui_if_possible;
        }

        UseInstalled visit(const SetReason & r) const
        {
            UseInstalledVisitor v(cmdline, true);
            return r.reason_for_set()->accept_returning<UseInstalled>(v);
        }
    };

    UseInstalled use_installed_fn(const ResolveCommandLine & cmdline,
            const QPN_S &,
            const PackageDepSpec &,
            const std::tr1::shared_ptr<const Reason> & reason)
    {
        UseInstalledVisitor v(cmdline, false);
        return reason->accept_returning<UseInstalled>(v);
    }

    int reinstall_scm_days(const ResolveCommandLine & cmdline)
    {
        if (cmdline.resolution_options.a_reinstall_scm.argument() == "always")
            return 0;
        else if (cmdline.resolution_options.a_reinstall_scm.argument() == "daily")
            return 1;
        else if (cmdline.resolution_options.a_reinstall_scm.argument() == "weekly")
            return 7;
        else if (cmdline.resolution_options.a_reinstall_scm.argument() == "never")
            return -1;
        else
            throw args::DoHelp("Don't understand argument '" + cmdline.resolution_options.a_reinstall_scm.argument() + "' to '--"
                    + cmdline.resolution_options.a_reinstall_scm.long_name() + "'");
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
            static time_t current_time(time(0)); /* static to avoid weirdness */
            time_t installed_time(current_time);
            if (id->installed_time_key())
                installed_time = id->installed_time_key()->value();

            return (current_time - installed_time) > (24 * 60 * 60 * n);
        }
        else
            return false;
    }

    bool installed_is_scm_older_than(const Environment * const env, const QPN_S & q, const int n)
    {
        Context context("When working out whether '" + stringify(q) + "' has installed SCM packages:");

        const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsUnsorted(
                    generator::Package(q.package()) |
                    q.make_slot_filter() |
                    filter::SupportsAction<InstalledAction>())]);

        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            if (is_scm_older_than(*i, n))
                return true;
        }

        return false;
    }

    typedef std::map<QPN_S, std::tr1::shared_ptr<Constraints> > InitialConstraints;

    const std::tr1::shared_ptr<Constraints> make_initial_constraints_for(
            const Environment * const env,
            const ResolveCommandLine & cmdline,
            const QPN_S & qpn_s)
    {
        const std::tr1::shared_ptr<Constraints> result(new Constraints);

        int n(reinstall_scm_days(cmdline));
        if ((-1 != n) && installed_is_scm_older_than(env, qpn_s, n))
        {
            result->add(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                                value_for<n::nothing_is_fine_too>(false),
                                value_for<n::reason>(make_shared_ptr(new PresetReason)),
                                value_for<n::spec>(make_package_dep_spec(PartiallyMadePackageDepSpecOptions()).package(qpn_s.package())),
                                value_for<n::to_destinations>(DestinationTypes()),
                                value_for<n::untaken>(false),
                                value_for<n::use_installed>(ui_only_if_transient)
                                ))));
        }

        return result;
    }

    const std::tr1::shared_ptr<Constraints> initial_constraints_for_fn(
            const Environment * const env,
            const ResolveCommandLine & cmdline,
            const InitialConstraints & initial_constraints,
            const QPN_S & qpn_s)
    {
        InitialConstraints::const_iterator i(initial_constraints.find(qpn_s));
        if (i == initial_constraints.end())
            return make_initial_constraints_for(env, cmdline, qpn_s);
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

    const std::tr1::shared_ptr<QPN_S_Sequence>
    get_qpn_s_s_for_fn(const Environment * const env,
            const ResolveCommandLine & cmdline,
            const PackageDepSpec & spec,
            const std::tr1::shared_ptr<const Reason> & reason)
    {
        std::tr1::shared_ptr<QPN_S_Sequence> result(new QPN_S_Sequence);
        std::tr1::shared_ptr<QPN_S> best;
        std::list<QPN_S> installed;

        const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(
                    generator::Matches(spec, MatchPackageOptions() + mpo_ignore_additional_requirements) |
                    filter::SupportsAction<InstallAction>() |
                    filter::NotMasked())]);

        if (! ids->empty())
            best = make_shared_ptr(new QPN_S(*ids->begin()));

        const std::tr1::shared_ptr<const PackageIDSequence> installed_ids((*env)[selection::BestVersionInEachSlot(
                    generator::Matches(spec, MatchPackageOptions()) |
                    filter::SupportsAction<InstalledAction>())]);

        for (PackageIDSequence::ConstIterator i(installed_ids->begin()), i_end(installed_ids->end()) ;
                i != i_end ; ++i)
            installed.push_back(QPN_S(*i));

        const args::EnumArg & arg(is_target(reason) ? cmdline.resolution_options.a_target_slots : cmdline.resolution_options.a_slots);

        if (! best)
            std::copy(installed.begin(), installed.end(), result->back_inserter());
        else if (arg.argument() == "best-or-installed")
        {
            if (installed.end() == std::find(installed.begin(), installed.end(), *best))
                result->push_back(*best);
            else
                std::copy(installed.begin(), installed.end(), result->back_inserter());
        }
        else if (arg.argument() == "installed-or-best")
        {
            if (installed.empty())
                result->push_back(*best);
            else
                std::copy(installed.begin(), installed.end(), result->back_inserter());
        }
        else if (arg.argument() == "all")
        {
            if (installed.end() == std::find(installed.begin(), installed.end(), *best))
                result->push_back(*best);
            std::copy(installed.begin(), installed.end(), result->back_inserter());
        }
        else if (arg.argument() == "best")
            result->push_back(*best);
        else
            throw args::DoHelp("Don't understand argument '" + arg.argument() + "' to '--"
                    + arg.long_name() + "'");

        return result;
    }

    struct IsSuggestionVisitor
    {
        bool is_suggestion;

        IsSuggestionVisitor() :
            is_suggestion(true)
        {
        }

        void visit(const DependencyRequiredLabel &)
        {
            is_suggestion = false;
        }

        void visit(const DependencyRecommendedLabel &)
        {
            is_suggestion = false;
        }

        void visit(const DependencySuggestedLabel &)
        {
        }
    };

    struct IsTypeDepVisitor
    {
        bool is_build_dep;
        bool is_compiled_against_dep;

        IsTypeDepVisitor() :
            is_build_dep(true),
            is_compiled_against_dep(false)
        {
        }

        void visit(const DependencyBuildLabel &)
        {
        }

        void visit(const DependencyRunLabel &)
        {
            is_build_dep = false;
        }

        void visit(const DependencyPostLabel &)
        {
            is_build_dep = false;
        }

        void visit(const DependencyInstallLabel &)
        {
        }

        void visit(const DependencyCompileLabel &)
        {
            is_compiled_against_dep = true;
        }
    };

    bool is_suggestion(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->suggest_labels()->empty())
            return false;

        IsSuggestionVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->suggest_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->suggest_labels()->end()),
                accept_visitor(v));
        return v.is_suggestion;
    }

    bool is_just_build_dep(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->type_labels()->empty())
            throw InternalError(PALUDIS_HERE, "not implemented");

        IsTypeDepVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->type_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->type_labels()->end()),
                accept_visitor(v));
        return v.is_build_dep;
    }

    bool is_compiled_against_dep(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->type_labels()->empty())
            throw InternalError(PALUDIS_HERE, "not implemented");

        IsTypeDepVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->type_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->type_labels()->end()),
                accept_visitor(v));
        return v.is_compiled_against_dep;
    }

    bool care_about_dep_fn(const Environment * const, const ResolveCommandLine & cmdline,
            const QPN_S &, const std::tr1::shared_ptr<const Resolution> & resolution,
            const SanitisedDependency & dep)
    {
        if (dk_installed == resolution->decision()->kind())
        {
            if (! cmdline.resolution_options.a_follow_installed_build_dependencies.specified())
                if (is_just_build_dep(dep))
                    return false;
            if (cmdline.resolution_options.a_ignore_installed_dependencies.specified())
                if (! is_compiled_against_dep(dep))
                    return false;
        }

        return true;
    }

    bool
    take_dependency_fn(const Environment * const,
            const ResolveCommandLine &,
            const QPN_S &,
            const SanitisedDependency & dep,
            const std::tr1::shared_ptr<const Reason> &)
    {
        if (is_suggestion(dep))
            return false;

        return true;
    }


    int display_resolution(
            const std::tr1::shared_ptr<Environment> &,
            const ResolutionLists & resolution_lists,
            const ResolveCommandLine & cmdline)
    {
        Context context("When displaying chosen resolution:");

        std::stringstream ser_stream;
        Serialiser ser(ser_stream);
        resolution_lists.serialise(ser);

        std::string command(cmdline.program_options.a_display_resolution_program.argument());
        if (command.empty())
            command = "$CAVE display-resolution";

        for (args::ArgsSection::GroupsConstIterator g(cmdline.display_options.begin()), g_end(cmdline.display_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                    command = command + " " + (*o)->forwardable_string();
        }

        for (ResolveCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
                p != p_end ; ++p)
            command = command + " " + args::escape(*p);

        paludis::Command cmd(command);
        cmd
            .with_input_stream(&ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

        return run_command(cmd);
    }

    void perform_resolution(
            const std::tr1::shared_ptr<Environment> &,
            const ResolutionLists & resolution_lists,
            const ResolveCommandLine & cmdline) PALUDIS_ATTRIBUTE((noreturn));

    void perform_resolution(
            const std::tr1::shared_ptr<Environment> &,
            const ResolutionLists & resolution_lists,
            const ResolveCommandLine & cmdline)
    {
        Context context("When performing chosen resolution:");

        std::stringstream ser_stream;
        Serialiser ser(ser_stream);
        resolution_lists.serialise(ser);

        std::string command(cmdline.program_options.a_execute_resolution_program.argument());
        if (command.empty())
            command = "$CAVE execute-resolution";

        for (args::ArgsSection::GroupsConstIterator g(cmdline.execution_options.begin()),
                g_end(cmdline.execution_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                    command = command + " " + (*o)->forwardable_string();
        }

        for (args::ArgsSection::GroupsConstIterator g(cmdline.program_options.begin()),
                g_end(cmdline.program_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                    command = command + " " + (*o)->forwardable_string();
        }

        if (! cmdline.resolution_options.a_execute.specified())
            command = command + " --pretend";

        for (ResolveCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
                p != p_end ; ++p)
            command = command + " " + args::escape(*p);

        paludis::Command cmd(command);
        cmd
            .with_input_stream(&ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

        become_command(cmd);
    }
}


bool
ResolveCommand::important() const
{
    return true;
}

int
ResolveCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    ResolveCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_RESOLVE_OPTIONS", "CAVE_RESOLVE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.resolution_options.a_lazy.specified() +
            cmdline.resolution_options.a_complete.specified() +
            cmdline.resolution_options.a_everything.specified() > 1)
        throw args::DoHelp("At most one of '--" + cmdline.resolution_options.a_lazy.long_name() + "', '--" + cmdline.resolution_options.a_complete.long_name()
                + "' or '--" + cmdline.resolution_options.a_everything.long_name() + "' may be specified");

    if (cmdline.resolution_options.a_lazy.specified())
    {
        if (! cmdline.resolution_options.a_target_slots.specified())
            cmdline.resolution_options.a_target_slots.set_argument("best");
        if (! cmdline.resolution_options.a_slots.specified())
            cmdline.resolution_options.a_slots.set_argument("best");
        if (! cmdline.resolution_options.a_ignore_installed_dependencies.specified())
            cmdline.resolution_options.a_ignore_installed_dependencies.set_specified(true);
    }

    if (cmdline.resolution_options.a_complete.specified())
    {
        if (! cmdline.resolution_options.a_keep.specified())
            cmdline.resolution_options.a_keep.set_argument("if-same");
        if (! cmdline.resolution_options.a_target_slots.specified())
            cmdline.resolution_options.a_target_slots.set_argument("all");
        if (! cmdline.resolution_options.a_slots.specified())
            cmdline.resolution_options.a_slots.set_argument("all");
        if (! cmdline.resolution_options.a_follow_installed_build_dependencies.specified())
            cmdline.resolution_options.a_follow_installed_build_dependencies.set_specified(true);
        if (! cmdline.resolution_options.a_reinstall_scm.specified())
            cmdline.resolution_options.a_reinstall_scm.set_argument("weekly");
    }

    if (cmdline.resolution_options.a_everything.specified())
    {
        if (! cmdline.resolution_options.a_keep.specified())
            cmdline.resolution_options.a_keep.set_argument("if-transient");
        if (! cmdline.resolution_options.a_keep_targets.specified())
            cmdline.resolution_options.a_keep_targets.set_argument("if-transient");
        if (! cmdline.resolution_options.a_target_slots.specified())
            cmdline.resolution_options.a_target_slots.set_argument("all");
        if (! cmdline.resolution_options.a_slots.specified())
            cmdline.resolution_options.a_slots.set_argument("all");
        if (! cmdline.resolution_options.a_follow_installed_build_dependencies.specified())
            cmdline.resolution_options.a_follow_installed_build_dependencies.set_specified(true);
    }

    int retcode(0);

    InitialConstraints initial_constraints;

    ResolverFunctions resolver_functions(make_named_values<ResolverFunctions>(
                value_for<n::care_about_dep_fn>(std::tr1::bind(&care_about_dep_fn,
                        env.get(), std::tr1::cref(cmdline), std::tr1::placeholders::_1,
                        std::tr1::placeholders::_2, std::tr1::placeholders::_3)),
                value_for<n::get_initial_constraints_for_fn>(std::tr1::bind(&initial_constraints_for_fn,
                        env.get(), std::tr1::cref(cmdline), std::tr1::cref(initial_constraints), std::tr1::placeholders::_1)),
                value_for<n::get_qpn_s_s_for_fn>(std::tr1::bind(&get_qpn_s_s_for_fn,
                        env.get(), std::tr1::cref(cmdline), std::tr1::placeholders::_1, std::tr1::placeholders::_2)),
                value_for<n::get_use_installed_fn>(std::tr1::bind(&use_installed_fn,
                        std::tr1::cref(cmdline), std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3)),
                value_for<n::take_dependency_fn>(std::tr1::bind(&take_dependency_fn, env.get(),
                        std::tr1::cref(cmdline), std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3))

                ));
    std::tr1::shared_ptr<Resolver> resolver(new Resolver(env.get(), resolver_functions));
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
                    add_resolver_targets(env, resolver, cmdline);
                    resolver->resolve();
                    break;
                }
                catch (const SuggestRestart & e)
                {
                    display_callback(ResolverRestart());
                    initial_constraints.insert(std::make_pair(e.qpn_s(), make_initial_constraints_for(
                                    env.get(), cmdline, e.qpn_s()))).first->second->add(
                            e.suggested_preset());
                    resolver = make_shared_ptr(new Resolver(env.get(), resolver_functions));
                }
            }
        }

        dump_if_requested(env, resolver, cmdline);

        retcode |= display_resolution(env, *resolver->resolution_lists(), cmdline);

        if (! resolver->resolution_lists()->errors()->empty())
            retcode |= 1;

        if (0 == retcode)
            perform_resolution(env, *resolver->resolution_lists(), cmdline);
    }
    catch (...)
    {
        dump_if_requested(env, resolver, cmdline);
        throw;
    }

    return retcode;
}

std::tr1::shared_ptr<args::ArgsHandler>
ResolveCommand::make_doc_cmdline()
{
    return make_shared_ptr(new ResolveCommandLine);
}

