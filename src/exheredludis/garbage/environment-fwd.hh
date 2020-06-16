/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENT_FWD_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENT_FWD_HH 1

#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/attributes.hh>
#include <string>

/** \file
 * Forward declarations for paludis/environment.hh .
 *
 * \ingroup g_environment
 */

namespace paludis
{
    /**
     * A collection of mirror prefixes.
     *
     * \see Environment
     * \ingroup g_environment
     */
    typedef Sequence<std::string> MirrorsSequence;

    class Environment;

    class CreateOutputManagerInfo;
    class CreateOutputManagerForPackageIDActionInfo;
    class CreateOutputManagerForRepositorySyncInfo;
}

#endif
