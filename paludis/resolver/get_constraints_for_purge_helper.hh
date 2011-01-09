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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_GET_CONSTRAINTS_FOR_PURGE_HELPER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_GET_CONSTRAINTS_FOR_PURGE_HELPER_HH 1

#include <paludis/resolver/get_constraints_for_purge_helper-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/change_by_resolvent-fwd.hh>
#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE GetConstraintsForPurgeHelper
        {
            private:
                Pimp<GetConstraintsForPurgeHelper> _imp;

            public:
                explicit GetConstraintsForPurgeHelper(const Environment * const);
                ~GetConstraintsForPurgeHelper();

                void add_purge_spec(const PackageDepSpec &);

                const std::shared_ptr<ConstraintSequence> operator() (
                        const std::shared_ptr<const Resolution> &,
                        const std::shared_ptr<const PackageID> &,
                        const std::shared_ptr<const ChangeByResolventSequence> &) const;
        };
    }

    extern template class Pimp<resolver::GetConstraintsForPurgeHelper>;
}

#endif
