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
#include <paludis/version_spec.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void PALUDIS_VISIBLE expose_version_spec()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<BadVersionSpecError>
        ("BadVersionSpecError", "BaseException",
         "Thrown if a VersionSpec is created from an invalid version string.");

    /**
     * VersionSpec
     */
    bp::implicitly_convertible<std::string, VersionSpec>();
    bp::class_<VersionSpec>
        (
         "VersionSpec",
         "A VersionSpec represents a version number (for example, 1.2.3b-r1).",
         bp::init<const std::string &>("__init__(string)")
        )
        .def("bump", &VersionSpec::bump,
                "bump() -> VersionSpec\n"
                "This is used by the ~> operator. It returns a VersionSpec where the next to last number "
                "is one greater (e.g. 5.3.1 => 5.4).\n"
                "Any non number parts are stripped (e.g. 1.2.3_alpha4-r5 => 1.3)."
            )

        .add_property("is_scm", &VersionSpec::is_scm,
                "[ro] bool\n"
                "Are we an -scm package, or something pretending to be one?"
                )

        .def("remove_revision", &VersionSpec::remove_revision,
                "remove_revision() -> VersionSpec\n"
                "Remove the revision part."
            )

        .def("revision_only", &VersionSpec::revision_only,
                "revision_only() -> string\n"
                "Revision part only (or \"r0\")."
            )

        .def("__cmp__", &VersionSpec::compare)

        .def(bp::self_ns::str(bp::self))
        ;
}
