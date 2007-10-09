/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_FWD_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_FWD_HH 1

#include <paludis/util/tr1_memory.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/dep_tree.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>

/** \file
 * Forward declarations for paludis/dep_list.hh .
 *
 * \ingroup g_dep_list
 */

namespace paludis
{
    class DepListOptions;
    class DepListEntryDestination;
    class DepListEntry;
    class DepList;

    /**
     * Is an item a valid child in an AnyDepSpec?
     *
     * \ingroup g_dep_list
     */
    bool is_viable_any_child(const Environment & env, const tr1::shared_ptr<const PackageID> &,
            const DependencySpecTree::ConstItem & i);
}

#endif
