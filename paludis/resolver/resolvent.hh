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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVENT_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVENT_HH 1

#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/destination_types-fwd.hh>
#include <paludis/resolver/destination_types-fwd.hh>
#include <paludis/resolver/slot_name_or_null.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/serialise-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct destination_type_name> destination_type;
        typedef Name<struct package_name> package;
        typedef Name<struct slot_name> slot;
    }

    namespace resolver
    {
        struct Resolvent
        {
            NamedValue<n::destination_type, DestinationType> destination_type;
            NamedValue<n::package, QualifiedPackageName> package;
            NamedValue<n::slot, SlotNameOrNull> slot;

            Resolvent(const Resolvent &);

            Resolvent(const QualifiedPackageName &, const SlotName &, const DestinationType);
            Resolvent(const QualifiedPackageName &, const SlotNameOrNull &, const DestinationType);

            Resolvent(const PackageDepSpec &, const bool, const DestinationType);
            Resolvent(const PackageDepSpec &, const SlotName &, const DestinationType);

            Resolvent(const std::tr1::shared_ptr<const PackageID> &, const DestinationType);

            void serialise(Serialiser &) const;
            static const Resolvent deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));

            std::size_t hash() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class Sequence<resolver::Resolvent>;
    extern template class WrappedForwardIterator<Sequence<resolver::Resolvent>::ConstIteratorTag, const resolver::Resolvent>;
#endif
}

#endif
