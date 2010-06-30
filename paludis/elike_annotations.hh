/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ELIKE_ANNOTATIONS_HH
#define PALUDIS_GUARD_PALUDIS_ELIKE_ANNOTATIONS_HH 1

#include <paludis/elike_annotations-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/metadata_key.hh>
#include <tr1/memory>

namespace paludis
{
    class PALUDIS_VISIBLE ELikeAnnotations :
        private PrivateImplementationPattern<ELikeAnnotations>,
        public MetadataSectionKey
    {
        public:
            ELikeAnnotations(const std::tr1::shared_ptr<const Map<std::string, std::string> > &);
            ~ELikeAnnotations();

            void need_keys_added() const;
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class PrivateImplementationPattern<ELikeAnnotations>;
}

#endif
