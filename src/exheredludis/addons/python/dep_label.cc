/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include <python/paludis_python.hh>
#include <python/exception.hh>
#include <python/iterable.hh>

#include <paludis/dep_label.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/always_enabled_dependency_label.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

namespace
{
    template <typename T_>
    T_ * make_concrete_dependencies_label(const std::string & t)
    {
        return new T_(t);
    }
}

template <typename L_>
struct class_concrete_uri_label :
    bp::class_<L_, std::shared_ptr<L_>, bp::bases<URILabel>, boost::noncopyable>
{
    class_concrete_uri_label(const std::string & name) :
        bp::class_<L_, std::shared_ptr<L_>, bp::bases<URILabel>, boost::noncopyable>
        (
         name.c_str(),
         "A concrete URI label class.",
         bp::init<const std::string &>("__init__(string)")
        )
    {
        bp::implicitly_convertible<std::shared_ptr<L_>, std::shared_ptr<URILabel> >();
    }
};

template <typename L_>
struct class_concrete_dependencies_label :
    bp::class_<L_, std::shared_ptr<L_>, bp::bases<DependenciesLabel>, boost::noncopyable>
{
    class_concrete_dependencies_label(const std::string & name) :
        bp::class_<L_, std::shared_ptr<L_>, bp::bases<DependenciesLabel>, boost::noncopyable>
        (
         name.c_str(),
         "A concrete dependencies label class.",
         bp::no_init
        )
    {
        this->def("__init__",
                bp::make_constructor(&make_concrete_dependencies_label<L_>),
                "__init__(String)"
           );

        bp::implicitly_convertible<std::shared_ptr<L_>, std::shared_ptr<DependenciesLabel> >();
    }
};

void expose_dep_label()
{
    /**
     * URILabel
     */
    register_shared_ptrs_to_python<URILabel>();
    bp::class_<URILabel, boost::noncopyable>
        (
         "URILabel",
         "URI label base class.",
         bp::no_init
        )
        .add_property("text", &URILabel::text,
                "[ro] string\n"
                "Our text."
                )

        .def("__str__", &URILabel::text)
        ;

    /**
     * ConcreteURILabels
     */
    class_concrete_uri_label<URIMirrorsThenListedLabel>("URIMirrorsThenListedLabel");
    class_concrete_uri_label<URIMirrorsOnlyLabel>("URIMirrorsOnlyLabel");
    class_concrete_uri_label<URIListedOnlyLabel>("URIListedOnlyLabel");
    class_concrete_uri_label<URIListedThenMirrorsLabel>("URIListedThenMirrorsLabel");
    class_concrete_uri_label<URILocalMirrorsOnlyLabel>("URILocalMirrorsOnlyLabel");
    class_concrete_uri_label<URIManualOnlyLabel>("URIManualOnlyLabel");

    /**
     * DependenciesLabel
     */
    register_shared_ptrs_to_python<DependenciesLabel>();
    bp::class_<DependenciesLabel, boost::noncopyable>
        (
         "DependenciesLabel",
         "Dependencies label base class.",
         bp::no_init
        )
        .add_property("text", &DependenciesLabel::text,
                "[ro] string\n"
                "Our text."
                )

        .def("__str__", &DependenciesLabel::text)
        ;

    /**
     * ConcreteDependenciesLabels
     */
    class_concrete_dependencies_label<AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag> >("DependenciesBuildLabel");
    class_concrete_dependencies_label<AlwaysEnabledDependencyLabel<DependenciesRunLabelTag> >("DependenciesRunLabel");
    class_concrete_dependencies_label<AlwaysEnabledDependencyLabel<DependenciesPostLabelTag> >("DependenciesPostLabel");
    class_concrete_dependencies_label<AlwaysEnabledDependencyLabel<DependenciesCompileAgainstLabelTag> >("DependenciesCompileAgainstLabel");
    class_concrete_dependencies_label<AlwaysEnabledDependencyLabel<DependenciesInstallLabelTag> >("DependenciesInstallLabel");
    class_concrete_dependencies_label<AlwaysEnabledDependencyLabel<DependenciesFetchLabelTag> >("DependenciesFetchLabel");
    class_concrete_dependencies_label<AlwaysEnabledDependencyLabel<DependenciesSuggestionLabelTag> >("DependenciesSuggestionLabel");
    class_concrete_dependencies_label<AlwaysEnabledDependencyLabel<DependenciesRecommendationLabelTag> >("DependenciesRecommendationLabel");
}
