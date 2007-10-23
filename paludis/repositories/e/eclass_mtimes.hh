/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_ECLASS_MTIMES_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_ECLASS_MTIMES_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/tr1_memory.hh>

namespace paludis
{
    /**
     * Holds an eclass mtimes cache for a ERepository.
     *
     * \see ERepository
     * \ingroup grperepository
     * \nosubgrouping
     */
    class EclassMtimes :
        private PrivateImplementationPattern<EclassMtimes>
    {
        public:
            ///\name Basic operations
            ///\{

            EclassMtimes(tr1::shared_ptr<const FSEntrySequence>);
            ~EclassMtimes();

            ///\}

            /**
             * Fetch the mtime for a given eclass.
             */
            time_t mtime(const std::string &) const;
    };
}

#endif
