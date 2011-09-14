/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_METADATA_XML_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_METADATA_XML_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/singleton.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/choice-fwd.hh>
#include <paludis/maintainer-fwd.hh>
#include <memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_herds> herds;
        typedef Name<struct name_long_description> long_description;
        typedef Name<struct name_maintainers> maintainers;
        typedef Name<struct name_uses> uses;
    }

    namespace erepository
    {
        struct MetadataXML
        {
            NamedValue<n::herds, std::shared_ptr<Sequence<std::string> > > herds;
            NamedValue<n::long_description, std::string> long_description;
            NamedValue<n::maintainers, std::shared_ptr<Maintainers> > maintainers;
            NamedValue<n::uses, std::shared_ptr<Map<ChoiceNameWithPrefix, std::string> > > uses;
        };

        class PALUDIS_VISIBLE MetadataXMLPool :
            public Singleton<MetadataXMLPool>
        {
            friend class Singleton<MetadataXMLPool>;

            private:
                Pimp<MetadataXMLPool> _imp;

                MetadataXMLPool();
                ~MetadataXMLPool();

            public:
                const std::shared_ptr<const MetadataXML> metadata_if_exists(const FSPath &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class Pimp<erepository::MetadataXMLPool>;
    extern template class Singleton<erepository::MetadataXMLPool>;
}

#endif
