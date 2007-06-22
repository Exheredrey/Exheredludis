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
#include <paludis/name.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

void PALUDIS_VISIBLE expose_name()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<PackageNamePartError>
        ("PackageNamePartError", "NameError",
         "Thrown if an invalid value is assigned to a PackageNamePart.");
    ExceptionRegister::get_instance()->add_exception<CategoryNamePartError>
        ("CategoryNamePartError", "NameError",
         "Thrown if an invalid value is assigned to a CategoryNamePart.");
    ExceptionRegister::get_instance()->add_exception<QualifiedPackageNameError>
        ("QualifiedPackageNameError", "NameError",
         "May be thrown if an invalid name is assigned to a QualifiedPackageName "
         "(alternatively, the exception raised may be a PackageNamePartError or a CategoryNamePartError).");
    ExceptionRegister::get_instance()->add_exception<UseFlagNameError>
        ("UseFlagNameError", "NameError",
         "Thrown if an invalid value is assigned to a UseFlagName.");
    ExceptionRegister::get_instance()->add_exception<IUseFlagNameError>
        ("IUseFlagNameError", "NameError",
         "Thrown if an invalid value is assigned to a IUseFlagName.");
    ExceptionRegister::get_instance()->add_exception<SlotNameError>
        ("SlotNameError", "NameError",
         "Thrown if an invalid value is assigned to a SlotName.");
    ExceptionRegister::get_instance()->add_exception<RepositoryNameError>
        ("RepositoryNameError", "NameError",
         "Thrown if an invalid value is assigned to a RepositoryName.");
    ExceptionRegister::get_instance()->add_exception<KeywordNameError>
        ("KeywordNameError", "NameError",
         "Thrown if an invalid value is assigned to a KeywordName.");
    ExceptionRegister::get_instance()->add_exception<SetNameError>
        ("SetNameError", "NameError",
         "Thrown if an invalid value is assigned to a SetName.");

    /**
     * Enums
     */
    enum_auto("UseFlagState", last_use);
    enum_auto("IUseFlagParseMode", last_iuse_pm);

    /**
     * PackageNamePart
     */
    register_shared_ptrs_to_python<PackageNamePart>();
    class_validated<PackageNamePart>
        (
         "PackageNamePart",
         "Holds a string that is a valid name for the package part of a QualifiedPackageName."
        );

    /**
     * PackageNamePartCollection
     */
    class_collection<PackageNamePartCollection>
        (
         "PackageNamePartCollection",
         "Iterable of PackageNamePart.\n"
         "A collection of PackageNamePart instances."
        );

    /**
     * CategoryNamePart
     */
    register_shared_ptrs_to_python<CategoryNamePart>();
    class_validated<CategoryNamePart>
        (
         "CategoryNamePart",
         "Holds a string that is a valid name for the category part of a QualifiedPackageName."
        )
        // CategoryNamePart + PackageNamePart = QualifiedPackageName
        .def(bp::self + bp::other<PackageNamePart>())
        ;

    /**
     * CategoryNamePartCollection
     */
    class_collection<CategoryNamePartCollection>
        (
         "CategoryNamePartCollection",
         "Iterable of CategoryNamePart.\n"
         "A collection of CategoryNamePart instances."
        );

    /**
     * UseFlagName
     */
    class_validated<UseFlagName>
        (
         "UseFlagName",
         "Holds a string that is a valid name for a USE flag."
        );

    /**
     * UseFlagNameCollection
     */
    class_collection<UseFlagNameCollection>
        (
         "UseFlagNameCollection",
         "Iterable of UseFlagName.\n"
         "A collection of UseFlagName instances."
        );

    /**
     * SlotName
     */
    register_shared_ptrs_to_python<SlotName>();
    class_validated<SlotName>
        (
         "SlotName",
         "Holds a string that is a valid name for a SLOT."
        );

    /**
     * RepositoryName
     */
    register_shared_ptrs_to_python<RepositoryName>();
    class_validated<RepositoryName>
        (
         "RepositoryName",
         "Holds a string that is a valid name for a Repository."
        );

    /**
     * RepositoryNameCollection
     */
    class_collection<RepositoryNameCollection>
        (
         "RepositoryNameCollection",
         "Iterable of RepositoryName\n"
         "A collection of RepositoryName instances."
        );

    /**
     * KeywordName
     */
    class_validated<KeywordName>
        (
         "KeywordName",
         "Holds a string that is a valid name for a KEYWORD."
        );

    /**
     * KeywordNameCollect
     */
    class_collection<KeywordNameCollection>
        (
         "KeywordNameCollection",
         "Iterable of KeywordName\n"
         "A collection of KeywordName instances."
        );

    /**
     * SetName
     */
    class_validated<SetName>
        (
         "SetName",
         "Holds a string that is a valid name for a set."
        );

    /**
     * SetNameCollection
     */
    class_collection<SetNameCollection>
        (
         "SetNameCollection",
         "Iterable of SetName\n"
         "A collection of set names."
        );

    /**
     * InheritedCollection
     */
    class_collection<InheritedCollection>
        (
         "InheritedCollection",
         "Iterable of string\n"
         "A collection of inherited packages."
        );

    /**
     * QualifiedPackageName
     */
    register_shared_ptrs_to_python<QualifiedPackageName>();
    bp::implicitly_convertible<std::string, QualifiedPackageName>();
    bp::class_<QualifiedPackageName>
        (
         "QualifiedPackageName",
         "Represents a category plus package name.",
         bp::init<const std::string &>("__init__(string)")
        )
        .def(bp::init<const CategoryNamePart &, const PackageNamePart &>())

        .def_readwrite("category", &QualifiedPackageName::category)

        .def_readwrite("package", &QualifiedPackageName::package)

        .def("__cmp__", &py_cmp<QualifiedPackageName>)

        .def(bp::self_ns::str(bp::self))
        ;

    /**
     * QualifiedPackageNameCollection
     */
    class_collection<QualifiedPackageNameCollection>
        (
         "QualifiedPackageNameCollection",
         "Iterable of QualifiedPackageName\n"
         "A collection of QualifiedPackageName instances."
        );

    /**
     * IUseFlag
     */
    bp::class_<IUseFlag>
        (
         "IUseFlag",
         "Represents an IUse flag.",
         bp::init<const std::string &, IUseFlagParseMode>("__init__(string, IUseFlagParseMode")
        )
        .def(bp::init<const UseFlagName &, const UseFlagState &>("__init__(UseFlagName, UseFlagState)"))

        .def_readwrite("flag", &IUseFlag::flag,
                "[rw] UseFlagName"
                )

        .def_readwrite("state", &IUseFlag::state,
                "[rw] UseFlagState"
                )

        .def("__cmp__", &py_cmp<IUseFlag>)

        .def(bp::self_ns::str(bp::self))

        ;

    /**
     * IUseFlagNameCollection
     */
    class_collection<IUseFlagCollection>
        (
         "IUseFlagCollection",
         "Iterable of IUseFlag\n"
         "A collection of use flags."
        );
}
