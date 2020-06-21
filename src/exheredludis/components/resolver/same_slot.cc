/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/resolver/same_slot.hh>
#include <paludis/util/wrapped_value.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/slot.hh>

using namespace paludis;
using namespace paludis::resolver;

bool
paludis::resolver::same_slot(
        const std::shared_ptr<const PackageID> & a,
        const std::shared_ptr<const PackageID> & b)
{
    if (a->slot_key())
        return b->slot_key() && a->slot_key()->parse_value().parallel_value() == b->slot_key()->parse_value().parallel_value();
    else
        return ! b->slot_key();
}

