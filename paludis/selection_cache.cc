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

#include <paludis/selection_cache.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/environment.hh>
#include <paludis/selection.hh>
#include <tr1/unordered_map>
#include <algorithm>

using namespace paludis;

typedef std::tr1::unordered_map<std::string, std::tr1::shared_ptr<const PackageIDSequence> > Cache;

namespace paludis
{
    template <>
    struct Implementation<SelectionCache>
    {
        mutable Mutex mutex;
        mutable Cache cache;
    };

    template <>
    struct Implementation<ScopedSelectionCache>
    {
        Environment * const environment;
        const std::tr1::shared_ptr<SelectionCache> selection_cache;

        Implementation(Environment * const e) :
            environment(e),
            selection_cache(new SelectionCache)
        {
        }
    };
}

SelectionCache::SelectionCache() :
    PrivateImplementationPattern<SelectionCache>(new Implementation<SelectionCache>)
{
}

SelectionCache::~SelectionCache()
{
}

const std::tr1::shared_ptr<PackageIDSequence>
SelectionCache::perform_select(const Environment * const env, const Selection & s) const
{
    std::string ss(s.as_string());
    Cache::const_iterator i(_imp->cache.end());

    {
        Lock lock(_imp->mutex);
        i = _imp->cache.find(ss);

        if (_imp->cache.end() == i)
            i = _imp->cache.insert(std::make_pair(ss, s.perform_select(env))).first;
    }

    std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);
    std::copy(i->second->begin(), i->second->end(), result->back_inserter());
    return result;
}

ScopedSelectionCache::ScopedSelectionCache(Environment * const e) :
    PrivateImplementationPattern<ScopedSelectionCache>(new Implementation<ScopedSelectionCache>(e))
{
    _imp->environment->add_selection_cache(_imp->selection_cache);
}

ScopedSelectionCache::~ScopedSelectionCache()
{
    _imp->environment->remove_selection_cache(_imp->selection_cache);
}

template class PrivateImplementationPattern<SelectionCache>;
template class PrivateImplementationPattern<ScopedSelectionCache>;

