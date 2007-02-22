/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/name.hh>

CommandLine::CommandLine() :
    ArgsHandler(),

    tree_action_args(this, "Tree-Oriented Actions",
            "Selects which basic tree-oriented action to perform. Exactly one action should "
            "be specified."),

    a_find_stable_candidates(&tree_action_args,
            "find-stable-candidates", 's',  "Search for stable package candidates"),
    a_find_dropped_keywords(&tree_action_args,
            "find-dropped-keywords",  'd',  "Search for packages where keywords have been dropped"),
    a_find_insecure_packages(&tree_action_args,
            "find-insecure-packages", 'i',  "Search for packages marked as insecure by a GLSA"),
    a_find_unused_packages(&tree_action_args,
            "find-unused-packages",   'U',  "Search package versions that can probably safely be removed"),
    a_keywords_graph(&tree_action_args,
            "keyword-graph",          'k',  "Display keywords graphically"),
    a_reverse_deps(&tree_action_args,
            "reverse-deps",           'r',  "Find all package that depend on a given dep spec"),
    a_what_needs_keywording(&tree_action_args,
            "what-needs-keywording", 'w', "Display what needs to be done to keyword a target"),

    profile_action_args(this, "Profile-Oriented Actions",
            "Selects which basic profile-oriented action to perform. Exactly one action should "
            "be specified."),

    a_display_profiles_use(&profile_action_args,
            "display-profiles-use",   'u',  "Display USE information for all profiles"),
    a_display_default_system_resolution(&profile_action_args,
            "display-default-system-resolution", 'S', "Display package names and versions that are included in "
            "the default resolution of the system set"),

    downgrade_check_args(this, "Downgrade Check Actions",
            "Selects which downgrade check related action to perform. Exactly one action should "
            "be specified."),

    a_build_downgrade_check_list(&downgrade_check_args,
            "build-downgrade-check-list", '\0', "Build the downgrade check lists"),
    a_downgrade_check(&downgrade_check_args,
            "downgrade-check", '\0', "Perform the dowgrade check"),

    general_action_args(this, "General Actions",
            "Selects which basic general action to perform. Exactly one action should "
            "be specified."),

    a_version(&general_action_args,
            "version",                'V',  "Display program version"),
    a_help(&general_action_args,
            "help",                   'h',  "Display program help"),

    general_args(this, "General options",
            "Options which are relevant for most or all actions."),
    a_log_level(&general_args, "log-level",  '\0'),
    a_no_colour(&general_args, "no-colour", '\0', "Do not use colour"),
    a_no_color(&a_no_colour, "no-color"),

    a_repository_directory(&general_args, "repository-dir", 'D',
            "Where to find the repository (default: detected from ./ or ../ or ../..)"),

    tree_args(this, "Tree action options",
            "Options which are relevant for tree actions."),
    a_category(&tree_args,   "category",   'C',
            "Matches with this category name only (may be specified multiple times)",
            paludis::args::StringSetArg::StringSetArgOptions(), &paludis::CategoryNamePartValidator::validate),
    a_package(&tree_args,    "package",    'P',
            "Matches with this package name only (may be specified multiple times)",
            paludis::args::StringSetArg::StringSetArgOptions(), &paludis::PackageNamePartValidator::validate),

    profile_args(this, "Profile action options",
            "Options which are relevant for profile actions."),
    a_profile(&profile_args,    "profile",    '\0',
            "Display results for this profile path, rather than all profiles (may be specified multiple times)"),
    a_unstable(&profile_args,   "unstable",    '\0',
            "Accept ~arch as well as arch"),

    configuration_options(this, "Configuration options",
            "Options that control general configuration."),
    a_write_cache_dir(&configuration_options, "write-cache-dir", '\0',
            "Use a subdirectory named for the repository name under the specified directory for repository write cache")
{
    add_usage_line("--find-stable-candidates arch [ --repository-dir /path ] "
            "[ --category app-misc --category sys-apps ... ] "
            "[ --package foo --package fnord ... ] ");
    add_usage_line("--find-dropped-keywords arch [ --repository-dir /path ] "
            "[ --category app-misc --category sys-apps ... ] "
            "[ --package foo --package fnord ... ] ");
    add_usage_line("--find-insecure-packages [ --repository-dir /path ] "
            "[ --category app-misc --category sys-apps ... ] "
            "[ --package foo --package fnord ... ] ");
    add_usage_line("--find-unused-packages [ --repository-dir /path ] "
            "[ --category app-misc --category sys-apps ... ] "
            "[ --package foo --package fnord ... ] ");
    add_usage_line("--keywords-graph [ --repository-dir /path ] "
            "[ --category app-misc --category sys-apps ... ] "
            "[ --package foo --package fnord ... ]");
    add_usage_line("--reverse-deps [ --repository-dir /path ] "
            "[ --category app-misc --category sys-apps ... ] "
            "[ --package foo --package fnord ... ]"
            "target-spec");
    add_usage_line("--what-needs-keywording [ --repository-dir /path ] "
            "[ --category app-misc --category sys-apps ... ] "
            "[ --package foo --package fnord ... ]"
            "target-keyword target-spec");
    add_usage_line("--display-profiles-use [ --profile default-linux/x86/2006.0 "
            "--profile default-linux/x86/2006.1 ... ] [ --repository-dir /path ]");
    add_usage_line("--display-default-system-resolution [ --profile default-linux/x86/2006.0 "
            "--profile default-linux/x86/2006.1 ... ] [ --repository-dir /path ]");

    add_usage_line("--version");
    add_usage_line("--help");

    add_environment_variable("PALUDIS_EBUILD_DIR", "Where to look for ebuild.bash and related "
            "utilities.");
    add_environment_variable("PALUDIS_REPOSITORY_SO_DIR", "Where to look for repository .so "
            "files.");
    add_environment_variable("ADJUTRIX_OPTIONS", "Default command-line options.");
}

std::string
CommandLine::app_name() const
{
    return "adjutrix";
}

std::string
CommandLine::app_synopsis() const
{
    return "A tool for arch teams";
}

std::string
CommandLine::app_description() const
{
    return
        "adjutrix provides a number of utilities that may be useful for arch teams."
        "\n\n"
        "The --repository-dir switch can be used to tell adjutrix where to find "
        "the repository. If this switch is not used, adjutrix will check the current "
        "directory, the parent directory and the parent's parent directory for "
        "something resembling a profile root. If run inside a package or category "
        "directory, filtering in the style of --package and --category is carried "
        "out automatically for the current package or category.";
}

CommandLine::~CommandLine()
{
}

