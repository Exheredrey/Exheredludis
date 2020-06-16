/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2014 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_PREFER_OR_AVOID_HELPER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_PREFER_OR_AVOID_HELPER_HH 1

#include <paludis/resolver/prefer_or_avoid_helper-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/tribool-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE PreferOrAvoidHelper
        {
            private:
                Pimp<PreferOrAvoidHelper> _imp;

            public:
                explicit PreferOrAvoidHelper(const Environment * const);
                ~PreferOrAvoidHelper();

                void add_prefer_name(const QualifiedPackageName &);
                void add_avoid_name(const QualifiedPackageName &);

                void add_prefer_matching(const std::shared_ptr<const PackageIDSequence> &);
                void add_avoid_matching(const std::shared_ptr<const PackageIDSequence> &);

                Tribool operator() (const PackageDepSpec &, const std::shared_ptr<const PackageID> &) const;
        };
    }

    extern template class Pimp<resolver::PreferOrAvoidHelper>;
}

#endif
