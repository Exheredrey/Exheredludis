/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#include "command_line.hh"

#include <paludis/util/instantiation_policy-impl.hh>

using namespace paludis;

template class InstantiationPolicy<CommandLine, instantiation_method::SingletonTag>;

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(this, "Actions",
            "Selects which basic action to perform. At most one action should "
            "be specified."),
    a_fix_linkage(&action_args, "fix-linkage", '\0', "Search for and rebuild packages linked against non-existant libraries (default)"),
    a_version(&action_args,     "version",     'V',  "Display program version"),
    a_help(&action_args,        "help",        'h',  "Display program help"),

    general_args(this, "General options",
            "Options which are relevant for most or all actions."),
    a_log_level(&general_args,      "log-level",      '\0'),
    a_no_colour(&general_args, "no-colour", '\0', "Do not use colour"),
    a_no_color(&a_no_colour,   "no-color"),
    a_environment(&general_args,    "environment",    'E',  "Environment specification (class:suffix, both parts optional)"),
    a_exact(&general_args,     "exact",               '\0', "Rebuild the same package version that is currently installed"),
    a_verbose(&general_args,        "verbose",        'v',  "Display more detailed output"),
    a_resume_command_template(&general_args, "resume-command-template", '\0', "Save the resume command to a file. If the filename contains 'XXXXXX', use mkstemp(3) to generate the filename"),

    fix_linkage_args(this, "Fix Linkage options",
            "Options which are relevant for --fix-linkage."),
    a_library(&fix_linkage_args, "library", '\0', "Only rebuild packages linked against this library, even if it exists"),

    install_args(this, "Install options",
            "Options which are relevant for the install process."),
    dl_args(this)
{
    add_usage_line("[ --fix-linkage ] [fix linkage options]");
    add_usage_line("--help");

    // XXX destinations support
    install_args.a_destinations.remove();
    install_args.a_preserve_world.remove();
    install_args.a_preserve_world.set_specified(true);
    install_args.a_add_to_world_spec.remove();

    dl_args.dl_reinstall_targets.remove();
    dl_args.dl_upgrade.set_default_arg("as-needed");
    dl_args.dl_new_slots.set_default_arg("as-needed");

    add_environment_variable("RECONCILIO_OPTIONS", "Default command-line options.");
}

std::string
CommandLine::app_name() const
{
    return "reconcilio";
}

std::string
CommandLine::app_synopsis() const
{
    return "A broken package rebuilder for Paludis, the other package mangler";
}

std::string
CommandLine::app_description() const
{
    return
        "reconcilio searches for and rebuilds packages that are linked against "
        "libraries that are not present on the system, or a specific library "
        "named by the user.";
}

CommandLine::~CommandLine()
{
}


