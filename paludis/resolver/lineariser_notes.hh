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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_LINEARISER_NOTES_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_LINEARISER_NOTES_HH 1

#include <paludis/resolver/lineariser_notes-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/named_value.hh>
#include <paludis/serialise-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct cycle_breaking_name> cycle_breaking;
    }

    namespace resolver
    {
        struct LineariserNotes
        {
            NamedValue<n::cycle_breaking, std::string> cycle_breaking;

            void serialise(Serialiser &) const;
            static const std::tr1::shared_ptr<LineariserNotes> deserialise(
                    Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
