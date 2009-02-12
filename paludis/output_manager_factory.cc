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

#include <paludis/output_manager_factory.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/system.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/about.hh>
#include <tr1/unordered_map>
#include <list>

#include <paludis/standard_output_manager.hh>

using namespace paludis;

namespace paludis
{
    namespace n
    {
        struct create_function;
    }
}

namespace
{
    struct Funcs
    {
        NamedValue<n::create_function, OutputManagerFactory::CreateFunction> create_function;
    };

    typedef std::tr1::unordered_map<std::string, Funcs> Keys;

    const Funcs & fetch(const Keys & keys, const std::string & key)
    {
        if (key.empty())
            throw ConfigurationError("Key 'handler' not specified when creating an output manager");

        Keys::const_iterator i(keys.find(key));
        if (i == keys.end())
            throw ConfigurationError("Format '" + key + "' not supported when creating an output manager (known formats are { "
                    + join(first_iterator(keys.begin()), first_iterator(keys.end()), ", ") + "})");

        return i->second;
    }
}

namespace paludis
{
    template <>
    struct Implementation<OutputManagerFactory>
    {
        Keys keys;
        std::list<void *> dl_opened;
    };
}

OutputManagerFactory::OutputManagerFactory() :
    PrivateImplementationPattern<OutputManagerFactory>(new Implementation<OutputManagerFactory>)
{
    /* we might want to make this plugin loadable at some point */
    add_manager(StandardOutputManager::factory_managers(), StandardOutputManager::factory_create);
}

OutputManagerFactory::~OutputManagerFactory()
{
}

const std::tr1::shared_ptr<OutputManager>
OutputManagerFactory::create(
        const KeyFunction & key_function
        ) const
{
    Context context("When creating output manager:");
    return fetch(_imp->keys, key_function("handler")).create_function()(key_function);
}

OutputManagerFactory::ConstIterator
OutputManagerFactory::begin_keys() const
{
    return first_iterator(_imp->keys.begin());
}

OutputManagerFactory::ConstIterator
OutputManagerFactory::end_keys() const
{
    return first_iterator(_imp->keys.end());
}

void
OutputManagerFactory::add_manager(
        const std::tr1::shared_ptr<const Set<std::string> > & formats,
        const CreateFunction & create_function
        )
{
    for (Set<std::string>::ConstIterator f(formats->begin()), f_end(formats->end()) ;
            f != f_end ; ++f)
    {
        if (! _imp->keys.insert(std::make_pair(*f, make_named_values<Funcs>(
                            value_for<n::create_function>(create_function)
                            ))).second)
            throw ConfigurationError("Handler for output manager format '" + stringify(*f) + "' already exists");
    }
}

template class PrivateImplementationPattern<OutputManagerFactory>;
template class InstantiationPolicy<OutputManagerFactory, instantiation_method::SingletonTag>;
template class WrappedForwardIterator<OutputManagerFactory::ConstIteratorTag, const std::string>;

