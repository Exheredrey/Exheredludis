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

#include <paludis/repositories/e/metadata_xml.hh>
#include <paludis/repositories/e/xml_things_handle.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/validated.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/log.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/choice.hh>
#include <tr1/unordered_map>

using namespace paludis;
using namespace paludis::erepository;

typedef std::tr1::unordered_map<FSEntry, std::tr1::shared_ptr<MetadataXML>, Hash<FSEntry> > Store;

namespace paludis
{
    template <>
    struct Implementation<MetadataXMLPool>
    {
        mutable Mutex mutex;
        mutable Store store;
    };
}

MetadataXMLPool::MetadataXMLPool() :
    PrivateImplementationPattern<MetadataXMLPool>(new Implementation<MetadataXMLPool>)
{
}

MetadataXMLPool::~MetadataXMLPool()
{
}

const std::tr1::shared_ptr<const MetadataXML>
MetadataXMLPool::metadata_if_exists(const FSEntry & f) const
{
    Context context("When handling metadata.xml file '" + stringify(f) + "':");

    FSEntry f_real(f.realpath_if_exists());
    Lock lock(_imp->mutex);
    Store::const_iterator i(_imp->store.find(f_real));
    if (i != _imp->store.end())
        return i->second;
    else
    {
        std::tr1::shared_ptr<MetadataXML> metadata_xml;
        if (f_real.is_regular_file_or_symlink_to_regular_file())
        {
            try
            {
                if (XMLThingsHandle::get_instance()->available())
                    metadata_xml = XMLThingsHandle::get_instance()->create_metadata_xml_from_xml_file()(f);
            }
            catch (const Exception & e)
            {
                Log::get_instance()->message("e.metadata_xml.bad", ll_warning, lc_context) << "Got exception '"
                    << e.message() << "' (" << e.what() << "), ignoring metadata.xml file '" << f_real << "'";
            }
        }
        return _imp->store.insert(std::make_pair(f_real, metadata_xml)).first->second;
    }
}

template class Map<ChoiceNameWithPrefix, std::string>;
template class PrivateImplementationPattern<MetadataXMLPool>;
template class InstantiationPolicy<MetadataXMLPool, instantiation_method::SingletonTag>;

