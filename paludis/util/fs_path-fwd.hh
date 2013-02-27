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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_FS_PATH_FWD_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_FS_PATH_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/options-fwd.hh>
#include <iosfwd>

namespace paludis
{
    class FSPath;
    class FSPathComparator;

    typedef Sequence<FSPath> FSPathSequence;

    typedef Set<FSPath, FSPathComparator> FSPathSet;

    /**
     * An FSPath can be written to an ostream.
     *
     * \ingroup g_fs
     */
    std::ostream & operator<< (std::ostream & s, const FSPath & f) PALUDIS_VISIBLE;

    bool operator== (const FSPath &, const FSPath &) PALUDIS_VISIBLE;
    bool operator!= (const FSPath &, const FSPath &) PALUDIS_VISIBLE;

#include <paludis/util/fs_path-se.hh>

    typedef Options<FSPathMkdirOption> FSPathMkdirOptions;
}

#endif
