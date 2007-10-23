/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PALUDIS_ENVIRONMENTS_PALUDIS_LICENSES_CONF_HH
#define PALUDIS_GUARD_PALUDIS_PALUDIS_ENVIRONMENTS_PALUDIS_LICENSES_CONF_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>

namespace paludis
{
    class PaludisEnvironment;

    namespace paludis_environment
    {
        /**
         * Represents the licenses.conf file, which may be composed of multiple 'real' files.
         *
         * \ingroup grppaludisenvironment
         * \nosubgrouping
         */
        class LicensesConf :
            private PrivateImplementationPattern<LicensesConf>,
            private InstantiationPolicy<LicensesConf, instantiation_method::NonCopyableTag>
        {
            public:
                ///\name Basic operations
                ///\{

                LicensesConf(const PaludisEnvironment * const);
                ~LicensesConf();

                ///\}

                /**
                 * Add another file.
                 */
                void add(const FSEntry &);

                /**
                 * Query a particular license.
                 */
                bool query(const std::string &, const PackageID &) const;
        };
    }
}

#endif
