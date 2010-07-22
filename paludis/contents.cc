/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/contents.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/literal_metadata_key.hh>
#include <list>

using namespace paludis;

typedef std::list<std::shared_ptr<const ContentsEntry> > Entries;

namespace paludis
{
    template <>
    struct Implementation<ContentsEntry>
    {
        const std::shared_ptr<const MetadataValueKey<FSEntry> > location_key;

        Implementation(const FSEntry & n) :
            location_key(std::make_shared<LiteralMetadataValueKey<FSEntry>>("location", "location", mkt_significant, n))
        {
        }
    };
}

ContentsEntry::ContentsEntry(const FSEntry & n) :
    PrivateImplementationPattern<ContentsEntry>(new Implementation<ContentsEntry>(n)),
    _imp(PrivateImplementationPattern<ContentsEntry>::_imp)
{
    add_metadata_key(_imp->location_key);
}

ContentsEntry::~ContentsEntry()
{
}

void
ContentsEntry::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<FSEntry> >
ContentsEntry::location_key() const
{
    return _imp->location_key;
}

ContentsFileEntry::ContentsFileEntry(const FSEntry & our_name) :
    ContentsEntry(our_name)
{
}

ContentsDirEntry::ContentsDirEntry(const FSEntry & our_name) :
    ContentsEntry(our_name)
{
}

ContentsOtherEntry::ContentsOtherEntry(const FSEntry & our_name) :
    ContentsEntry(our_name)
{
}

namespace paludis
{
    template <>
    struct Implementation<ContentsSymEntry>
    {
        const std::shared_ptr<const MetadataValueKey<std::string> > target_key;

        Implementation(const std::string & t) :
            target_key(std::make_shared<LiteralMetadataValueKey<std::string>>("target", "target", mkt_normal, t))
        {
        }
    };
}

ContentsSymEntry::ContentsSymEntry(const FSEntry & our_name, const std::string & our_target) :
    PrivateImplementationPattern<ContentsSymEntry>(new Implementation<ContentsSymEntry>(our_target)),
    ContentsEntry(our_name),
    _imp(PrivateImplementationPattern<ContentsSymEntry>::_imp)
{
    add_metadata_key(_imp->target_key);
}

ContentsSymEntry::~ContentsSymEntry()
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
ContentsSymEntry::target_key() const
{
    return _imp->target_key;
}

namespace paludis
{
    template<>
    struct Implementation<Contents>
    {
        Entries c;
    };

    template <>
    struct WrappedForwardIteratorTraits<Contents::ConstIteratorTag>
    {
        typedef Entries::const_iterator UnderlyingIterator;
    };
}

Contents::Contents() :
    PrivateImplementationPattern<Contents>(new Implementation<Contents>())
{
}

Contents::~Contents()
{
}

void
Contents::add(const std::shared_ptr<const ContentsEntry> & c)
{
    _imp->c.push_back(c);
}

Contents::ConstIterator
Contents::begin() const
{
    return ConstIterator(_imp->c.begin());
}

Contents::ConstIterator
Contents::end() const
{
    return ConstIterator(_imp->c.end());
}

template class PrivateImplementationPattern<Contents>;
template class PrivateImplementationPattern<ContentsEntry>;
template class PrivateImplementationPattern<ContentsSymEntry>;

template class WrappedForwardIterator<Contents::ConstIteratorTag, const std::shared_ptr<const ContentsEntry> >;

