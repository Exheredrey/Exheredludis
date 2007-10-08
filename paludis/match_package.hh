/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_MATCH_PACKAGE_HH
#define PALUDIS_GUARD_PALUDIS_MATCH_PACKAGE_HH 1

/** \file
 * Declarations for match_package and match_package_in_set.
 *
 * \ingroup g_query
 *
 * \section Examples
 *
 * - \ref example_match_package.cc "example_match_package.cc"
 */

#include <paludis/util/attributes.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/dep_tree.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>

namespace paludis
{
    /**
     * Return whether the specified PackageID matches the specified
     * PackageDepSpec.
     *
     * \ingroup g_query
     */
    bool match_package(
            const Environment & env,
            const PackageDepSpec & spec,
            const PackageID & target)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    /**
     * Return whether the specified PackageID matches any of the items in the
     * specified set.
     *
     * Named sets inside the set are expanded.
     *
     * \ingroup g_query
     */
    bool match_package_in_set(
            const Environment & env,
            const SetSpecTree::ConstItem & spec,
            const PackageID & target)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;
}

#endif
