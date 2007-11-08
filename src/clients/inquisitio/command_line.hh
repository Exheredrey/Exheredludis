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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_INQUISITIO_COMMAND_LINE_HH
#define PALUDIS_GUARD_SRC_CLIENTS_INQUISITIO_COMMAND_LINE_HH 1

#include <paludis/args/args.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/args/log_level_arg.hh>

class CommandLine :
    public paludis::args::ArgsHandler,
    public paludis::InstantiationPolicy<CommandLine, paludis::instantiation_method::SingletonTag>
{
    friend class paludis::InstantiationPolicy<CommandLine, paludis::instantiation_method::SingletonTag>;

    private:
        CommandLine();
        ~CommandLine();

    public:
        ///\name Program information
        ///\{

        virtual std::string app_name() const;
        virtual std::string app_synopsis() const;
        virtual std::string app_description() const;

        ///\}

        /// \name Action arguments
        ////{

        paludis::args::ArgsGroup action_args;

        paludis::args::SwitchArg a_search;
        paludis::args::SwitchArg a_version;
        paludis::args::SwitchArg a_help;

        ///\}

        /// \name General arguments
        ///{

        paludis::args::ArgsGroup general_args;
        paludis::args::LogLevelArg a_log_level;
        paludis::args::SwitchArg a_no_colour;
        paludis::args::AliasArg a_no_color;
        paludis::args::StringArg a_environment;

        ///}

        ///\name Matching arguments
        ///\{

        paludis::args::ArgsGroup match_args;
        paludis::args::StringSetArg a_keys;
        paludis::args::EnumArg a_matcher;
        paludis::args::SwitchArg a_flatten;
        paludis::args::SwitchArg a_enabled_only;
        paludis::args::SwitchArg a_not;

        ///\}

        ///\name Filter arguments
        ///\{

        paludis::args::ArgsGroup filter_args;
        paludis::args::StringSetArg a_repository;
        paludis::args::StringSetArg a_repository_format;
        paludis::args::StringSetArg a_category;
        paludis::args::StringSetArg a_package;
        paludis::args::SwitchArg a_visible_only;
        paludis::args::SwitchArg a_all_versions;
        paludis::args::EnumArg a_kind;

        ///\}

        ///\name Output arguments
        ///\{

        paludis::args::ArgsGroup output_args;
        paludis::args::SwitchArg a_compact;
        paludis::args::SwitchArg a_show_dependencies;
        paludis::args::SwitchArg a_show_metadata;

        ///\}

        ///\name Deprecated arguments
        ///\{

        paludis::args::ArgsGroup deprecated_args;
        paludis::args::StringArg a_config_suffix;

        ///\}
};


#endif
