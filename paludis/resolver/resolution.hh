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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLUTION_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLUTION_HH 1

#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/arrow-fwd.hh>
#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/util/named_value.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        struct already_ordered;
        struct arrows;
        struct constraints;
        struct decision;
        struct resolvent;
    }

    namespace resolver
    {
        struct Resolution
        {
            NamedValue<n::already_ordered, bool> already_ordered;
            NamedValue<n::arrows, std::tr1::shared_ptr<ArrowSequence> > arrows;
            NamedValue<n::constraints, std::tr1::shared_ptr<Constraints> > constraints;
            NamedValue<n::decision, std::tr1::shared_ptr<Decision> > decision;
            NamedValue<n::resolvent, Resolvent> resolvent;

            void serialise(Serialiser &) const;

            static const std::tr1::shared_ptr<Resolution> deserialise(
                    Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
