/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_E_USE_DESC_HH
#define PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_E_USE_DESC_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/choice-fwd.hh>
#include <utility>

namespace paludis
{
    typedef std::pair<FSPath, ChoicePrefixName> UseDescFileInfo;
    typedef Sequence<UseDescFileInfo> UseDescFileInfoSequence;

    /**
     * Implements use.desc options for ERepository.
     *
     * \ingroup grperepository
     */
    class UseDesc
    {
        private:
            Pimp<UseDesc> _imp;

        public:
            ///\name Basic operations
            ///\{

            UseDesc(const std::shared_ptr<const UseDescFileInfoSequence> &);
            ~UseDesc();

            UseDesc(const UseDesc &) = delete;
            UseDesc & operator= (const UseDesc &) = delete;

            ///\}

            /**
             * Describe the given use flag.
             *
             * May return an empty string.
             */
            const std::string describe(
                    const QualifiedPackageName &,
                    const ChoicePrefixName & prefix,
                    const UnprefixedChoiceName & suffix) const;
    };
}

#endif
