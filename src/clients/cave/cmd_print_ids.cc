/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include "cmd_print_ids.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintIDsCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-ids";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of known IDs.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of known IDs. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }

        args::ArgsGroup g_filters;
        args::StringSetArg a_matching;
        args::StringSetArg a_supporting;
        args::StringSetArg a_with_mask;

        PrintIDsCommandLine() :
            g_filters(this, "Filters", "Filter the output. Each filter may be specified more than once."),
            a_matching(&g_filters, "matching", '\0', "Show only IDs matching this spec. If specified multiple "
                    "times, only IDs matching every spec are selected.",
                    args::StringSetArg::StringSetArgOptions()),
            a_supporting(&g_filters, "supporting", '\0', "Show only IDs supporting this action. If specified "
                    "multiple times, all listed actions must be supported.",
                    args::StringSetArg::StringSetArgOptions
                    ("install",       "able to be installed")
                    ("installed",     "installed")
                    ("uninstall",     "able to be uninstalled")
                    ("pretend",       "has pretend-install-time checks")
                    ("config",        "supports post-install configuration")
                    ("fetch",         "able to have sources fetched")
                    ("pretend-fetch", "able to have sources pretend-fetched")
                    ("info",          "provides extra pre- or post-install information")
                    ),
            a_with_mask(&g_filters, "with-mask", '\0', "Show only IDs with this kind of mask. If specified "
                    "multiple times, all listed masks must be present.",
                    args::StringSetArg::StringSetArgOptions
                    ("none",          "no mask")
                    ("any",           "any kind of mask")
                    ("user",          "masked by user")
                    ("unaccepted",    "masked by unaccepted key")
                    ("repository",    "masked by repository")
                    ("unsupported",   "masked because it is unsupported")
                    ("association",   "masked by association")
                   )
        {
            add_usage_line("[ --matching spec ] [ --supporting action ] [ --with-mask mask-kind ]");
        }
    };

    struct WithMaskFilterHandler :
        FilterHandler
    {
        const PrintIDsCommandLine & cmdline;
        const std::string mask;

        WithMaskFilterHandler(const PrintIDsCommandLine & c, const std::string & f) :
            cmdline(c),
            mask(f)
        {
        }

        virtual std::string as_string() const
        {
            return "with mask '" + mask + "'";
        }

        virtual std::tr1::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const,
                const std::tr1::shared_ptr<const RepositoryNameSet> & r) const
        {
            return r;
        }

        virtual std::tr1::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const,
                const std::tr1::shared_ptr<const RepositoryNameSet> &,
                const std::tr1::shared_ptr<const CategoryNamePartSet> & c) const
        {
            return c;
        }

        virtual std::tr1::shared_ptr<const QualifiedPackageNameSet> packages(
                const Environment * const,
                const std::tr1::shared_ptr<const RepositoryNameSet> &,
                const std::tr1::shared_ptr<const QualifiedPackageNameSet> & q) const
        {
            return q;
        }

        virtual std::tr1::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::tr1::shared_ptr<const PackageIDSet> & c) const
        {
            std::tr1::shared_ptr<PackageIDSet> result(new PackageIDSet);

            for (PackageIDSet::ConstIterator i(c->begin()), i_end(c->end()) ;
                    i != i_end ; ++i)
            {
                if (mask == "none")
                {
                    if (! (*i)->masked())
                        result->insert(*i);
                }
                else if (mask == "any")
                {
                    if ((*i)->masked())
                        result->insert(*i);
                }
                else if (mask == "user")
                {
                    for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                            m != m_end ; ++m)
                        if (simple_visitor_cast<const UserMask>(**m))
                        {
                            result->insert(*i);
                            break;
                        }
                }
                else if (mask == "unaccepted")
                {
                    for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                            m != m_end ; ++m)
                        if (simple_visitor_cast<const UnacceptedMask>(**m))
                        {
                            result->insert(*i);
                            break;
                        }
                }
                else if (mask == "repository")
                {
                    for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                            m != m_end ; ++m)
                        if (simple_visitor_cast<const RepositoryMask>(**m))
                        {
                            result->insert(*i);
                            break;
                        }
                }
                else if (mask == "unsupported")
                {
                    for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                            m != m_end ; ++m)
                        if (simple_visitor_cast<const UnsupportedMask>(**m))
                        {
                            result->insert(*i);
                            break;
                        }
                }
                else if (mask == "association")
                {
                    for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                            m != m_end ; ++m)
                        if (simple_visitor_cast<const AssociationMask>(**m))
                        {
                            result->insert(*i);
                            break;
                        }
                }
                else
                    throw args::DoHelp("Unknown --" + cmdline.a_with_mask.long_name() + " value '" + mask + "'");
            }

            return result;
        }
    };

    struct WithMaskFilter :
        Filter
    {
        WithMaskFilter(const PrintIDsCommandLine & c, const std::string & f) :
            Filter(make_shared_ptr(new WithMaskFilterHandler(c, f)))
        {
        }
    };
}

int
PrintIDsCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintIDsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_IDS_OPTIONS", "CAVE_PRINT_IDS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-ids takes no parameters");

    Generator g((generator::All()));
    if (cmdline.a_matching.specified())
    {
        for (args::StringSetArg::ConstIterator m(cmdline.a_matching.begin_args()),
                m_end(cmdline.a_matching.end_args()) ;
                m != m_end ; ++m)
        {
            PackageDepSpec s(parse_user_package_dep_spec(*m, env.get(), UserPackageDepSpecOptions() + updso_allow_wildcards));
            g = g & generator::Matches(s, MatchPackageOptions());
        }
    }

    FilteredGenerator fg(g);

    if (cmdline.a_supporting.specified())
    {
        for (args::StringSetArg::ConstIterator m(cmdline.a_supporting.begin_args()),
                m_end(cmdline.a_supporting.end_args()) ;
                m != m_end ; ++m)
        {
            if (*m == "install")
                fg = fg | filter::SupportsAction<InstallAction>();
            else if (*m == "installed")
                fg = fg | filter::SupportsAction<InstalledAction>();
            else if (*m == "uninstall")
                fg = fg | filter::SupportsAction<UninstallAction>();
            else if (*m == "pretend")
                fg = fg | filter::SupportsAction<PretendAction>();
            else if (*m == "config")
                fg = fg | filter::SupportsAction<ConfigAction>();
            else if (*m == "fetch")
                fg = fg | filter::SupportsAction<FetchAction>();
            else if (*m == "pretend-fetch")
                fg = fg | filter::SupportsAction<PretendFetchAction>();
            else if (*m == "info")
                fg = fg | filter::SupportsAction<InfoAction>();
            else
                throw args::DoHelp("Unknown --" + cmdline.a_supporting.long_name() + " value '" + *m + "'");
        }
    }

    if (cmdline.a_with_mask.specified())
    {
        for (args::StringSetArg::ConstIterator m(cmdline.a_with_mask.begin_args()),
                m_end(cmdline.a_with_mask.end_args()) ;
                m != m_end ; ++m)
        {
            fg = fg | WithMaskFilter(cmdline, *m);
        }
    }

    const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(fg)]);
    std::copy(indirect_iterator(ids->begin()), indirect_iterator(ids->end()),
            std::ostream_iterator<PackageID>(cout, "\n"));

    return EXIT_SUCCESS;
}

std::tr1::shared_ptr<args::ArgsHandler>
PrintIDsCommand::make_doc_cmdline()
{
    return make_shared_ptr(new PrintIDsCommandLine);
}

