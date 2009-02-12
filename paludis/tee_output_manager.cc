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

#include <paludis/tee_output_manager.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tee_output_stream.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/tokeniser.hh>
#include <vector>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<TeeOutputManager>
    {
        const std::tr1::shared_ptr<const Sequence<std::tr1::shared_ptr<OutputManager> > > streams;
        TeeOutputStream stdout_stream;
        TeeOutputStream stderr_stream;

        Implementation(
                const std::tr1::shared_ptr<const Sequence<std::tr1::shared_ptr<OutputManager> > > & s) :
            streams(s)
        {
        }
    };
}

TeeOutputManager::TeeOutputManager(
        const std::tr1::shared_ptr<const OutputManagerSequence> & s) :
    PrivateImplementationPattern<TeeOutputManager>(new Implementation<TeeOutputManager>(s))
{
    for (OutputManagerSequence::ConstIterator i(s->begin()), i_end(s->end()) ;
            i != i_end ; ++i)
    {
        _imp->stdout_stream.add_stream(&(*i)->stdout_stream());
        _imp->stderr_stream.add_stream(&(*i)->stderr_stream());
    }
}

TeeOutputManager::~TeeOutputManager()
{
}

std::ostream &
TeeOutputManager::stdout_stream()
{
    return _imp->stdout_stream;
}

std::ostream &
TeeOutputManager::stderr_stream()
{
    return _imp->stderr_stream;
}

void
TeeOutputManager::succeeded()
{
    for (OutputManagerSequence::ConstIterator i(_imp->streams->begin()), i_end(_imp->streams->end()) ;
            i != i_end ; ++i)
        (*i)->succeeded();
}

void
TeeOutputManager::message(const MessageType t, const std::string & s)
{
    for (OutputManagerSequence::ConstIterator i(_imp->streams->begin()), i_end(_imp->streams->end()) ;
            i != i_end ; ++i)
        (*i)->message(t, s);
}

const std::tr1::shared_ptr<const Set<std::string> >
TeeOutputManager::factory_managers()
{
    std::tr1::shared_ptr<Set<std::string> > result(new Set<std::string>);
    result->insert("tee");
    return result;
}

const std::tr1::shared_ptr<OutputManager>
TeeOutputManager::factory_create(
        const OutputManagerFactory::KeyFunction & key_func,
        const OutputManagerFactory::CreateChildFunction & create_child)
{
    std::tr1::shared_ptr<OutputManagerSequence> children(new OutputManagerSequence);

    std::vector<std::string> children_keys;
    tokenise_whitespace(key_func("children"), std::back_inserter(children_keys));
    if (children_keys.empty())
        throw ConfigurationError("No children specified for TeeOutputManager");

    for (std::vector<std::string>::const_iterator c(children_keys.begin()), c_end(children_keys.end()) ;
            c != c_end ; ++c)
        children->push_back(create_child(*c));

    return make_shared_ptr(new TeeOutputManager(children));
}

template class PrivateImplementationPattern<TeeOutputManager>;

