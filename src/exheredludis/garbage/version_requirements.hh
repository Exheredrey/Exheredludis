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

#ifndef PALUDIS_GUARD_PALUDIS_VERSION_REQUIREMENTS_HH
#define PALUDIS_GUARD_PALUDIS_VERSION_REQUIREMENTS_HH 1

#include <paludis/version_requirements-fwd.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>

/** \file
 * Declarations for version requirements classes.
 *
 * \ingroup g_dep_spec
 *
 * \section Examples
 *
 * - \ref example_dep_spec.cc "example_dep_spec.cc" (for specifications)
 */

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_version_operator> version_operator;
        typedef Name<struct name_version_spec> version_spec;
    }

    /**
     * A requirement for a version, consisting of a VersionOperator and an associated
     * VersionSpec.
     *
     * \ingroup g_dep_spec
     * \see PackageDepSpec
     * \see VersionSpec
     * \see VersionOperator
     * \see VersionRequirements
     * \nosubgrouping
     */
    struct VersionRequirement
    {
        NamedValue<n::version_operator, VersionOperator> version_operator;
        NamedValue<n::version_spec, VersionSpec> version_spec;
    };

    extern template class PALUDIS_VISIBLE Sequence<VersionRequirement>;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<Sequence<VersionRequirement>::ConstIteratorTag, const VersionRequirement>;
    extern template class PALUDIS_VISIBLE WrappedOutputIterator<Sequence<VersionRequirement>::InserterTag, VersionRequirement>;
}

#endif
