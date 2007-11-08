/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/util/instantiation_policy-impl.hh>

using namespace paludis;

template class paludis::InstantiationPolicy<CommandLine, paludis::instantiation_method::SingletonTag>;

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(this, "Actions",
            "Selects which basic action to perform. Exactly one action should "
            "be specified."),
    a_search(&action_args,     "search",        's',  "Search for a package"),
    a_version(&action_args,   "version",      'V',  "Display program version"),
    a_help(&action_args,      "help",         'h',  "Display program help"),

    general_args(this, "General options",
            "Options that are relevant for most or all actions."),
    a_log_level(&general_args, "log-level",  '\0'),
    a_no_colour(&general_args, "no-colour", '\0', "Do not use colour"),
    a_no_color(&a_no_colour, "no-color"),
    a_environment(&general_args, "environment", 'E', "Environment specification (class:suffix, both parts optional)"),

    match_args(this, "Matching options",
            "Options that control which packages are matched."),
    a_keys(&match_args, "keys", 'k', "Match using listed metadata keys, rather than name and description"),
    a_matcher(&match_args, "matcher", 'm', "Which match algorithm to use",
            paludis::args::EnumArg::EnumArgOptions
            ("text",        "Simple text match")
            ("pcre",        "Regular expression match using pcre")
            ("exact",       "Exact text match"),
            "text"),
    a_flatten(&match_args, "flatten", 'f', "Flatten spec trees, rather than matching against individual items"),
    a_enabled_only(&match_args, "enabled-only", 'e', "When searching spec trees, only look in enabled subtrees"),
    a_not(&match_args, "not", 'n', "Select packages that do not match"),

    filter_args(this, "Filter options",
            "Options that control whether or not a package is considered for matching."),

    a_repository(&filter_args, "repository", 'r', "Matches with this repository name only",
            paludis::args::StringSetArg::StringSetArgOptions(), &paludis::RepositoryNameValidator::validate),
    a_repository_format(&filter_args, "repository-format", '\0', "Matches with this repository format only"),
    a_category(&filter_args,   "category",   '\0', "Matches with this category name only",
            paludis::args::StringSetArg::StringSetArgOptions(), &paludis::CategoryNamePartValidator::validate),
    a_package(&filter_args,    "package",    '\0', "Matches with this package name only",
            paludis::args::StringSetArg::StringSetArgOptions(), &paludis::PackageNamePartValidator::validate),
    a_visible_only(&filter_args, "visible-only", 'v', "Only consider visible packages"),
    a_all_versions(&filter_args, "all-versions", 'a', "Check all versions, rather than only one (slower)"),
    a_kind(&filter_args, "kind", 'K', "Packages of this kind only",
            paludis::args::EnumArg::EnumArgOptions
            ("installable",        "Installable packages")
            ("installed",          "Installed packages")
            ("all",                "All packages"),
            "installable"),

    output_args(this, "Output options",
            "Options that control how output is generated."),

    a_list(&output_args, "list", 'l', "Output a package name list only"),
    a_show_dependencies(&output_args, "show-dependencies", 'D', "Show dependencies"),
    a_show_metadata(&output_args, "show-metadata", 'M', "Show raw metadata"),

    deprecated_args(this, "Deprecated options", "Deprecated options."),
    a_config_suffix(&deprecated_args, "config-suffix", 'c',
            "Replaced by --environment")
{
    add_usage_line("[ --search ] [search options] pattern ...");
    add_usage_line("--help");

    add_environment_variable("INQUISITIO_OPTIONS", "Default command-line options.");
}

std::string
CommandLine::app_name() const
{
    return "inquisitio";
}

std::string
CommandLine::app_synopsis() const
{
    return "A search client for Paludis, the other package mangler";
}

std::string
CommandLine::app_description() const
{
    return
        "inquisitio is a search client for Paludis. It can find packages based upon a "
        "number of different criteria including package name, description and homepage, "
        "and using a number of different match techniques including simple text match, "
        "approximate match and regular expression.";
}

CommandLine::~CommandLine()
{
}


