/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_FS_MERGER_FWD_HH
#define PALUDIS_GUARD_PALUDIS_FS_MERGER_FWD_HH 1

#include <iosfwd>
#include <paludis/util/attributes.hh>
#include <paludis/util/options-fwd.hh>

/** \file
 * Forward declarations for paludis/fs_merger.hh .
 *
 * \ingroup g_repository
 * \since 0.51
 */

namespace paludis
{
#include <paludis/fs_merger-se.hh>

    /**
     * Status flags for Merger.
     *
     * \ingroup g_repository
     * \since 0.26
     * \since 0.51 called FSMergerStatusFlags instead of MergeStatusFlags
     */
    typedef Options<FSMergerStatusFlag> FSMergerStatusFlags;

    /**
     * Options for FSMerger.
     *
     * \ingroup g_repository
     * \since 0.71
     */
    typedef Options<FSMergerOption> FSMergerOptions;

    struct FSMergerParams;
    class FSMergerError;
    class FSMerger;
}

#endif
