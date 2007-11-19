/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#include "install_args_group.hh"

#include <paludis/environment-fwd.hh>
#include <paludis/args/do_help.hh>

using namespace paludis;
using namespace paludis::args;

InstallArgsGroup::InstallArgsGroup(ArgsHandler * h, const std::string & our_name,
                                   const std::string & our_description) :
    ArgsGroup(h, our_name, our_description),

    a_pretend(this, "pretend", 'p', "Pretend only", false),
    a_destinations(this, "destinations", 'd', "Use specified destinations instead of defaults"),
    a_preserve_world(this, "preserve-world", '1', "Don't modify the world file", true),
    a_add_to_world_spec(this, "add-to-world-spec", '\0',
            "Use this spec, rather than all targets, for updating world (for resume commands)"),
    a_no_config_protection(this, "no-config-protection", '\0', "Disable config file protection (dangerous)", false),
    a_debug_build(this, "debug-build", '\0'),
    a_checks(this, "checks", '\0'),
    a_fetch(this, "fetch", 'f', "Only fetch sources; don't install anything", false),
    a_no_safe_resume(this, "no-safe-resume", '\0', "Do not allow interrupted downloads to be resumed", false),
    a_show_reasons(this, "show-reasons", '\0', "Show why packages are being (un)installed",
            args::EnumArg::EnumArgOptions
            ("none",    "Don't show any information")
            ("summary", "Show a summary")
            ("full",    "Show full output (can be very verbose)"),
            "summary"),
    a_show_use_descriptions(this, "show-use-descriptions", '\0', "Show descriptions of USE flags",
            args::EnumArg::EnumArgOptions
            ("none",       "Don't show any descriptions")
            ("new",        "Show for new use flags")
            ("changed",    "Show for new and changed flags")
            ("all",        "Show for all flags"),
            "changed"),
    a_show_package_descriptions(this, "show-package-descriptions", '\0', "Show package descriptions",
            args::EnumArg::EnumArgOptions
            ("none",       "Don't show any descriptions")
            ("new",        "Show descriptions for new packages")
            ("all",        "Show descriptions for all packages"),
            "new"),
    a_continue_on_failure(this, "continue-on-failure", '\0', "Whether to continue after a fetch or install error",
            args::EnumArg::EnumArgOptions
            ("if-fetch-only",       "If fetching only")
            ("never",               "Never")
            ("if-satisfied",        "If remaining packages' dependencies are satisfied")
            ("if-independent",      "If independent of failed and skipped packages")
            ("always",              "Always (UNSAFE)"),
            "if-fetch-only")
{
}

InstallArgsGroup::~InstallArgsGroup()
{
}

void
InstallArgsGroup::populate_dep_list_options(const Environment *, DepListOptions & options) const
{
    options.dependency_tags = a_show_reasons.argument() == "summary" || a_show_reasons.argument() == "full";
}

tr1::shared_ptr<const DestinationsSet>
InstallArgsGroup::destinations(Environment * env) const
{
    if (a_destinations.specified())
    {
        Context local_context("When building destinations collection:");

        tr1::shared_ptr<DestinationsSet> d(new DestinationsSet);
        for (args::StringSetArg::ConstIterator i(a_destinations.begin_args()),
                i_end(a_destinations.end_args()) ;
                i != i_end ; ++i)
        {
            tr1::shared_ptr<Repository> repo(env->package_database()->fetch_repository(
                        RepositoryName(*i)));
            if (repo->destination_interface)
                d->insert(repo);
            else
                throw args::DoHelp("--destinations argument '" + *i + "' does not provide a destinations interface");
        }

        return d;
    }
    else
        return env->default_destinations();
}

void
InstallArgsGroup::populate_install_task(const Environment *, InstallTask & task) const
{
    task.set_no_config_protect(a_no_config_protection.specified());
    task.set_fetch_only(a_fetch.specified());
    task.set_pretend(a_pretend.specified());
    task.set_preserve_world(a_preserve_world.specified());
    task.set_safe_resume(! a_no_safe_resume.specified());

    if (a_add_to_world_spec.specified())
        task.set_add_to_world_spec(a_add_to_world_spec.argument());

    task.set_debug_mode(a_debug_build.option());
    task.set_checks_mode(a_checks.option());

    if (a_continue_on_failure.argument() == "if-fetch-only")
        task.set_continue_on_failure(itcof_if_fetch_only);
    else if (a_continue_on_failure.argument() == "never")
        task.set_continue_on_failure(itcof_never);
    else if (a_continue_on_failure.argument() == "if-satisfied")
        task.set_continue_on_failure(itcof_if_satisfied);
    else if (a_continue_on_failure.argument() == "if-independent")
        task.set_continue_on_failure(itcof_if_independent);
    else if (a_continue_on_failure.argument() == "always")
        task.set_continue_on_failure(itcof_always);
    else
        throw args::DoHelp("bad value for --continue-on-failure");
}

bool
InstallArgsGroup::want_full_install_reasons() const
{
    return "full" == a_show_reasons.argument();
}

bool
InstallArgsGroup::want_tags_summary() const
{
    return a_pretend.specified();
}

bool
InstallArgsGroup::want_install_reasons() const
{
    if (! a_pretend.specified())
        return false;

    return "full" == a_show_reasons.argument() || "summary" == a_show_reasons.argument();
}

bool
InstallArgsGroup::want_unchanged_use_flags() const
{
    return "none" != a_show_use_descriptions.argument() &&
        "new" != a_show_use_descriptions.argument() &&
        "changed" != a_show_use_descriptions.argument();
}

bool
InstallArgsGroup::want_changed_use_flags() const
{
    return "none" != a_show_use_descriptions.argument() && "new" != a_show_use_descriptions.argument();
}

bool
InstallArgsGroup::want_new_use_flags() const
{
    return "none" != a_show_use_descriptions.argument();
}

bool
InstallArgsGroup::want_use_summary() const
{
    return "none" != a_show_use_descriptions.argument();
}

bool
InstallArgsGroup::want_new_descriptions() const
{
    return "none" != a_show_package_descriptions.argument();
}

bool
InstallArgsGroup::want_existing_descriptions() const
{
    return "all" == a_show_package_descriptions.argument();
}

std::string
InstallArgsGroup::paludis_command_fragment() const
{
    std::string paludis_command;

    if (a_no_config_protection.specified())
        paludis_command.append(" --" + a_no_config_protection.long_name());

    if (a_preserve_world.specified())
        paludis_command.append(" --" + a_preserve_world.long_name());

    if (a_debug_build.specified())
        paludis_command.append(" --" + a_debug_build.long_name() + " " + a_debug_build.argument());

    if (a_no_safe_resume.specified())
        paludis_command.append(" --" + a_no_safe_resume.long_name());

    return paludis_command;
}

std::string
InstallArgsGroup::resume_command_fragment(const InstallTask & task) const
{
    std::string resume_command;

    if (a_add_to_world_spec.specified())
        resume_command = resume_command + " --" + a_add_to_world_spec.long_name()
                + " '" + a_add_to_world_spec.argument() + "'";
    else if (! a_preserve_world.specified())
    {
        if (task.had_set_targets())
            resume_command = resume_command + " --" + a_add_to_world_spec.long_name()
                + " '( )'";
        else
            resume_command = resume_command + " --" + a_add_to_world_spec.long_name()
                + " '( " + join(task.begin_targets(), task.end_targets(), " ") + " )'";
    }

    if (a_destinations.specified())
        for (args::StringSetArg::ConstIterator i(a_destinations.begin_args()),
                i_end(a_destinations.end_args()) ; i != i_end ; ++i)
            resume_command = resume_command + " --" + a_destinations.long_name() + " '" + *i + "'";

    return resume_command;
}

