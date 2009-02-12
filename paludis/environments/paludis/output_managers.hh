/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_OUTPUT_MANAGERS_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_OUTPUT_MANAGERS_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/create_output_manager_info-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    class PaludisEnvironment;

    namespace paludis_environment
    {
        class OutputManagers :
            private PrivateImplementationPattern<OutputManagers>,
            private InstantiationPolicy<OutputManagers, instantiation_method::NonCopyableTag>
        {
            public:
                ///\name Basic operations
                ///\{

                OutputManagers(const PaludisEnvironment * const);
                ~OutputManagers();

                ///\}

                /**
                 * Add another file.
                 */
                void add(const FSEntry &);

                const std::tr1::shared_ptr<OutputManager> create_named_output_manager(
                        const std::string &,
                        const CreateOutputManagerInfo &) const;

                std::string replace_vars(
                        const std::string &,
                        const CreateOutputManagerInfo &) const;
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<paludis_environment::OutputManagers>;
#endif
}

#endif
