/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#include <paludis/forward_at_finish_output_manager.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<ForwardAtFinishOutputManager>
    {
        std::stringstream stdout_stream;
        std::stringstream stderr_stream;
        const bool if_success, if_failure;
        const std::tr1::shared_ptr<OutputManager> child;
        bool success;

        Implementation(
                const bool s,
                const bool f,
                const std::tr1::shared_ptr<OutputManager> & m
                ) :
            if_success(s),
            if_failure(f),
            child(m),
            success(false)
        {
        }
    };
}

ForwardAtFinishOutputManager::ForwardAtFinishOutputManager(
        const bool s,
        const bool f,
        const std::tr1::shared_ptr<OutputManager> & m) :
    PrivateImplementationPattern<ForwardAtFinishOutputManager>(
            new Implementation<ForwardAtFinishOutputManager>(s, f, m))
{
}

ForwardAtFinishOutputManager::~ForwardAtFinishOutputManager()
{
    if ((_imp->if_success && _imp->success) || (_imp->if_failure && ! _imp->success))
    {
        std::copy((std::istreambuf_iterator<char>(_imp->stdout_stream)),
                std::istreambuf_iterator<char>(),
                std::ostreambuf_iterator<char>(_imp->child->stdout_stream()));
        std::copy((std::istreambuf_iterator<char>(_imp->stderr_stream)),
                std::istreambuf_iterator<char>(),
                std::ostreambuf_iterator<char>(_imp->child->stderr_stream()));
    }
}

std::ostream &
ForwardAtFinishOutputManager::stdout_stream()
{
    return _imp->stdout_stream;
}

std::ostream &
ForwardAtFinishOutputManager::stderr_stream()
{
    return _imp->stderr_stream;
}

void
ForwardAtFinishOutputManager::succeeded()
{
    _imp->success = true;
}

void
ForwardAtFinishOutputManager::message(const MessageType, const std::string &)
{
}

void
ForwardAtFinishOutputManager::flush()
{
}

bool
ForwardAtFinishOutputManager::want_to_flush() const
{
    return false;
}

void
ForwardAtFinishOutputManager::nothing_more_to_come()
{
    if (! ((_imp->if_success && _imp->success) || (_imp->if_failure && ! _imp->success)))
    {
        _imp->stdout_stream.clear();
        _imp->stderr_stream.clear();
    }
}

const std::tr1::shared_ptr<const Set<std::string> >
ForwardAtFinishOutputManager::factory_managers()
{
    std::tr1::shared_ptr<Set<std::string> > result(new Set<std::string>);
    result->insert("forward_at_finish");
    return result;
}

const std::tr1::shared_ptr<OutputManager>
ForwardAtFinishOutputManager::factory_create(
        const OutputManagerFactory::KeyFunction & key_func,
        const OutputManagerFactory::CreateChildFunction & create_child_function,
        const OutputManagerFactory::ReplaceVarsFunc &)
{
    std::string child_s(key_func("child")), if_success_s(key_func("if_success")), if_failure_s(key_func("if_failure"));

    if (child_s.empty())
        throw ConfigurationError("Key 'child' not specified when creating a forward_on_failure output manager");

    std::tr1::shared_ptr<OutputManager> child(create_child_function(child_s));

    return make_shared_ptr(new ForwardAtFinishOutputManager(
                destringify<bool>(if_success_s),
                destringify<bool>(if_failure_s),
                child));
}

template class PrivateImplementationPattern<ForwardAtFinishOutputManager>;

