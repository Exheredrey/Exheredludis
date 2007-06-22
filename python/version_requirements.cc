/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#include <paludis_python.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <paludis/version_requirements.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void PALUDIS_VISIBLE expose_version_requirements()
{
    /**
     * Enums
     */
    enum_auto("VersionRequirementsMode", last_vr);

    /**
     * VersionRequirement
     */
    bp::class_<VersionRequirement>
        (
         "VersionRequirement",
         bp::init<const VersionOperator &, const VersionSpec &>(
             "__init__(VersionOperator, VersionSpec)"
             )
        )
        .def_readwrite("version_operator", &VersionRequirement::version_operator,
                "[rw] VersionOperator"
                )

        .def_readwrite("version_spec", &VersionRequirement::version_spec,
                "[rw] VersionSpec"
                )

        .def("__eq__", &VersionRequirement::operator==)

        .def("__ne__", &py_ne<VersionRequirement>)
        ;

    /**
     * VersionRequirements
     */
    class_collection<VersionRequirements>
        (
         "VersionRequirements",
         "Iterable collection of VersionRequirement instances, usually for a PackageDepSpec."
        );
}
