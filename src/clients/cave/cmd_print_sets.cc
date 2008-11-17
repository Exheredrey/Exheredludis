/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Alexander Færøy
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

#include "cmd_print_sets.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <cstdlib>
#include <iostream>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintSetsCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-sets";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of sets.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of sets. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }
    };
}

int
PrintSetsCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintSetsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_SETS_OPTIONS", "CAVE_PRINT_SETS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-sets takes no parameters");

    std::set<SetName> sets;

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories());
            r != r_end; ++r)
    {
        if ((*r).sets_interface() == 0)
            continue;

        std::tr1::shared_ptr<const SetNameSet> set_names((*r).sets_interface()->sets_list());

        std::copy(set_names->begin(), set_names->end(), std::inserter(sets, sets.begin()));
    }

    std::copy(sets.begin(), sets.end(), std::ostream_iterator<SetName>(cout, "\n"));

    return EXIT_SUCCESS;
}

std::tr1::shared_ptr<args::ArgsHandler>
PrintSetsCommand::make_doc_cmdline()
{
    return make_shared_ptr(new PrintSetsCommandLine);
}
