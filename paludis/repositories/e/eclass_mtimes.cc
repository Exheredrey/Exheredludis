/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
 * Copyright (c) 2008 David Leverton
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

#include "eclass_mtimes.hh"
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/layout.hh>
#include <paludis/name.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/hashes.hh>
#include <tr1/unordered_map>

using namespace paludis;

namespace
{
    struct Cache
    {
        std::tr1::shared_ptr<const FSEntrySequence> dirs;
        std::tr1::unordered_map<std::string, FSEntry, Hash<std::string> > files;

        Cache(const std::tr1::shared_ptr<const FSEntrySequence> & d) :
            dirs(d)
        {
        }
    };

    const FSEntry *
    lookup(const std::string & e, Cache & c)
    {
        std::tr1::unordered_map<std::string, FSEntry, Hash<std::string> >::const_iterator i(c.files.find(e));
        if (i != c.files.end())
            return & i->second;

        for (FSEntrySequence::ReverseConstIterator d(c.dirs->rbegin()),
                 d_end(c.dirs->rend()) ; d != d_end ; ++d)
        {
            FSEntry f(*d / e);
            if (f.exists())
                return & c.files.insert(std::make_pair(e, f)).first->second;
        }

        return 0;
    }
}

namespace paludis
{
    template<>
    struct Implementation<EclassMtimes>
    {
        const ERepository * repo;
        mutable Cache eclasses;
        mutable std::tr1::unordered_map<QualifiedPackageName, Cache, Hash<QualifiedPackageName> > exlibs;
        mutable Mutex mutex;

        Implementation(const ERepository * r, const std::tr1::shared_ptr<const FSEntrySequence> & d) :
            repo(r),
            eclasses(d)
        {
        }
    };
}

EclassMtimes::EclassMtimes(const ERepository * r, const std::tr1::shared_ptr<const FSEntrySequence> & d) :
    PrivateImplementationPattern<EclassMtimes>(new Implementation<EclassMtimes>(r, d))
{
}

EclassMtimes::~EclassMtimes()
{
}

const FSEntry *
EclassMtimes::eclass(const std::string & e) const
{
    Lock l(_imp->mutex);
    return lookup(e + ".eclass", _imp->eclasses);
}

const FSEntry *
EclassMtimes::exlib(const std::string & e, const QualifiedPackageName & qpn) const
{
    Lock l(_imp->mutex);
    std::tr1::unordered_map<QualifiedPackageName, Cache, Hash<QualifiedPackageName> >::iterator cache(_imp->exlibs.find(qpn));
    if (_imp->exlibs.end() == cache)
        cache = _imp->exlibs.insert(std::make_pair(qpn, Cache(_imp->repo->layout()->exlibsdirs(qpn)))).first;
    return lookup(e + ".exlib", cache->second);
}

