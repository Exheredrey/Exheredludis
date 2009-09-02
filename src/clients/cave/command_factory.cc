/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#include "command_factory.hh"
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <tr1/functional>
#include <map>

#include "cmd_display_resolution.hh"
#include "cmd_help.hh"
#include "cmd_perform.hh"
#include "cmd_print_categories.hh"
#include "cmd_print_commands.hh"
#include "cmd_print_environment_metadata.hh"
#include "cmd_print_id_contents.hh"
#include "cmd_print_id_executables.hh"
#include "cmd_print_id_metadata.hh"
#include "cmd_print_ids.hh"
#include "cmd_print_owners.hh"
#include "cmd_print_packages.hh"
#include "cmd_print_repositories.hh"
#include "cmd_print_repository_formats.hh"
#include "cmd_print_sets.hh"
#include "cmd_print_sync_protocols.hh"
#include "cmd_resolve.hh"
#include "cmd_show.hh"
#include "cmd_sync.hh"
#include "cmd_update_world.hh"

using namespace paludis;
using namespace cave;

typedef std::map<std::string, std::tr1::function<const std::tr1::shared_ptr<Command> ()> > Handlers;

namespace paludis
{
    template <>
    struct Implementation<CommandFactory>
    {
        Handlers handlers;
    };
}

namespace
{
    template <typename T_>
    const std::tr1::shared_ptr<T_> make_command()
    {
        return make_shared_ptr(new T_);
    }
}

CommandFactory::CommandFactory() :
    PrivateImplementationPattern<CommandFactory>(new Implementation<CommandFactory>)
{
    _imp->handlers.insert(std::make_pair("display-resolution", make_command<DisplayResolutionCommand>));
    _imp->handlers.insert(std::make_pair("help", make_command<HelpCommand>));
    _imp->handlers.insert(std::make_pair("perform", make_command<PerformCommand>));
    _imp->handlers.insert(std::make_pair("print-categories", make_command<PrintCategoriesCommand>));
    _imp->handlers.insert(std::make_pair("print-commands", make_command<PrintCommandsCommand>));
    _imp->handlers.insert(std::make_pair("print-environment-metadata", make_command<PrintEnvironmentMetadataCommand>));
    _imp->handlers.insert(std::make_pair("print-id-contents", make_command<PrintIDContentsCommand>));
    _imp->handlers.insert(std::make_pair("print-id-executables", make_command<PrintIDExecutablesCommand>));
    _imp->handlers.insert(std::make_pair("print-id-metadata", make_command<PrintIDMetadataCommand>));
    _imp->handlers.insert(std::make_pair("print-ids", make_command<PrintIDsCommand>));
    _imp->handlers.insert(std::make_pair("print-owners", make_command<PrintOwnersCommand>));
    _imp->handlers.insert(std::make_pair("print-packages", make_command<PrintPackagesCommand>));
    _imp->handlers.insert(std::make_pair("print-repositories", make_command<PrintRepositoriesCommand>));
    _imp->handlers.insert(std::make_pair("print-repository-formats", make_command<PrintRepositoryFormatsCommand>));
    _imp->handlers.insert(std::make_pair("print-sets", make_command<PrintSetsCommand>));
    _imp->handlers.insert(std::make_pair("print-sync-protocols", make_command<PrintSyncProtocolsCommand>));
    _imp->handlers.insert(std::make_pair("resolve", make_command<ResolveCommand>));
    _imp->handlers.insert(std::make_pair("show", make_command<ShowCommand>));
    _imp->handlers.insert(std::make_pair("sync", make_command<SyncCommand>));
    _imp->handlers.insert(std::make_pair("update-world", make_command<UpdateWorldCommand>));
}

CommandFactory::~CommandFactory()
{
}

const std::tr1::shared_ptr<Command>
CommandFactory::create(const std::string & s) const
{
    Handlers::const_iterator i(_imp->handlers.find(s));
    if (i == _imp->handlers.end())
        throw UnknownCommand(s);
    else
        return i->second();
}

CommandFactory::ConstIterator
CommandFactory::begin() const
{
    return first_iterator(_imp->handlers.begin());
}

CommandFactory::ConstIterator
CommandFactory::end() const
{
    return first_iterator(_imp->handlers.end());
}

UnknownCommand::UnknownCommand(const std::string & s) throw () :
    Exception("Unknown command '" + s + "'")
{
}

template class InstantiationPolicy<CommandFactory, instantiation_method::SingletonTag>;
template class PrivateImplementationPattern<CommandFactory>;
template class WrappedForwardIterator<CommandFactory::ConstIteratorTag, const std::string>;

