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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_XML_THINGS_HANDLE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_XML_THINGS_HANDLE_HH 1

#include <paludis/repositories/e/glsa.hh>
#include <paludis/repositories/e/metadata_xml.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/fs_entry-fwd.hh>

namespace paludis
{
    namespace erepository
    {
        class PALUDIS_VISIBLE XMLThingsHandle :
            private PrivateImplementationPattern<XMLThingsHandle>,
            public InstantiationPolicy<XMLThingsHandle, instantiation_method::SingletonTag>
        {
            friend class InstantiationPolicy<XMLThingsHandle, instantiation_method::SingletonTag>;

            private:
                XMLThingsHandle();
                ~XMLThingsHandle();

            public:
                typedef std::tr1::shared_ptr<GLSA> (* CreateGLSAFromXMLFilePtr) (const std::string &);
                typedef std::tr1::shared_ptr<MetadataXML> (* CreateMetadataXMLFromXMLFilePtr) (const FSEntry &);

                CreateGLSAFromXMLFilePtr create_glsa_from_xml_file() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                CreateMetadataXMLFromXMLFilePtr create_metadata_xml_from_xml_file() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<erepository::XMLThingsHandle>;
    extern template class InstantiationPolicy<erepository::XMLThingsHandle, instantiation_method::SingletonTag>;
#endif
}

#endif
