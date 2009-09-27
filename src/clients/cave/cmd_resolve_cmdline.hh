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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_CMD_RESOLVE_CMDLINE_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_CMD_RESOLVE_CMDLINE_HH 1

#include "command_command_line.hh"

namespace paludis
{
    namespace cave
    {
        struct ResolveCommandLineResolutionOptions :
            args::ArgsSection
        {
            ResolveCommandLineResolutionOptions(args::ArgsHandler * const);

            args::ArgsGroup g_execution_options;
            args::SwitchArg a_execute;

            args::ArgsGroup g_convenience_options;
            args::SwitchArg a_lazy;
            args::SwitchArg a_complete;
            args::SwitchArg a_everything;

    //        args::ArgsGroup g_resolution_options;
    //        args::SwitchArg a_permit_slot_uninstalls;
    //        args::SwitchArg a_permit_uninstalls;
    //        args::SwitchArg a_permit_downgrades;
    //        args::SwitchArg a_permit_unsafe_uninstalls;

    //        args::ArgsGroup g_cleanup_options;
    //        args::SwitchArg a_purge_unused_slots;
    //        args::SwitchArg a_purge_unused_packages;

            args::ArgsGroup g_keep_options;
            args::EnumArg a_keep_targets;
            args::EnumArg a_keep;
            args::EnumArg a_reinstall_scm;
    //        args::SwitchArg a_reinstall_for_removals;

            args::ArgsGroup g_slot_options;
            args::EnumArg a_target_slots;
            args::EnumArg a_slots;

            args::ArgsGroup g_dependency_options;
            args::SwitchArg a_follow_installed_build_dependencies;
            args::SwitchArg a_ignore_installed_dependencies;

    //        args::ArgsGroup g_suggestion_options;
    //        args::EnumArg a_suggestions;
    //        args::EnumArg a_recommendations;
    //        args::StringSetArg a_take;
    //        args::StringSetArg a_take_from;
    //        args::StringSetArg a_discard;
    //        args::StringSetArg a_discard_from;

    //        args::ArgsGroup g_package_options;
    //        args::StringSetArg a_prefer;
    //        args::StringSetArg a_avoid;

    //        args::ArgsGroup g_ordering_options;
    //        args::StringSetArg a_early;
    //        args::StringSetArg a_late;

    //        args::ArgsGroup g_preset_options;
    //        args::StringSetArg a_soft_preset;
    //        args::StringSetArg a_fixed_preset;

            args::ArgsGroup g_destination_options;
    //        args::SwitchArg a_fetch;
            args::StringSetArg a_create_binaries;
            args::StringSetArg a_no_binaries_for;
    //        args::SwitchArg a_install_to_chroot;
            args::SwitchArg a_install_to_root;

    //        args::ArgsGroup g_interactivity_options;
    //        args::SwitchArg a_interactive;
    //        args::SwitchArg a_interactive_slots;
    //        args::SwitchArg a_interactive_decisions;
    //        args::SwitchArg a_interactive_ordering;

            args::ArgsGroup g_dump_options;
            args::SwitchArg a_dump;
            args::SwitchArg a_dump_dependencies;
            args::SwitchArg a_dump_restarts;
        };

        struct ResolveCommandLineExecutionOptions :
            args::ArgsSection
        {
            ResolveCommandLineExecutionOptions(args::ArgsHandler * const);

            args::ArgsGroup g_world_options;
            args::SwitchArg a_preserve_world;

            args::ArgsGroup g_failure_options;
            args::EnumArg a_continue_on_failure;

            args::ArgsGroup g_phase_options;
            args::StringSetArg a_skip_phase;
            args::StringSetArg a_abort_at_phase;
            args::StringSetArg a_skip_until_phase;
            args::EnumArg a_change_phases_for;
        };

        struct ResolveCommandLineDisplayOptions :
            args::ArgsSection
        {
            ResolveCommandLineDisplayOptions(args::ArgsHandler * const);

            args::ArgsGroup g_display_options;
//            args::EnumArg a_show_option_descriptions;
            args::EnumArg a_show_descriptions;

            args::ArgsGroup g_explanations;
            args::StringSetArg a_explain;
        };

        struct ResolveCommandLineProgramOptions :
            args::ArgsSection
        {
            ResolveCommandLineProgramOptions(args::ArgsHandler * const);

            args::ArgsGroup g_program_options;
            args::StringArg a_display_resolution_program;
            args::StringArg a_execute_resolution_program;
            args::StringArg a_perform_program;
            args::StringArg a_update_world_program;
        };

        struct ResolveCommandLine :
            CaveCommandCommandLine
        {
            virtual std::string app_name() const;
            virtual std::string app_synopsis() const;
            virtual std::string app_description() const;

            ResolveCommandLine();

            ResolveCommandLineResolutionOptions resolution_options;
            ResolveCommandLineExecutionOptions execution_options;
            ResolveCommandLineDisplayOptions display_options;
            ResolveCommandLineProgramOptions program_options;
        };
    }
}

#endif
