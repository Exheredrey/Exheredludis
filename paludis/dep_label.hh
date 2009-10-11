/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LABEL_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LABEL_HH 1

#include <paludis/dep_label-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/type_list.hh>

/** \file
 * Declarations for dependency label-related classes.
 *
 * \ingroup g_dep_spec
 *
 * \section Examples
 *
 * - \ref example_dep_label.cc "example_dep_label.cc"
 * - \ref example_dep_spec.cc "example_dep_spec.cc" (for specifications)
 * - \ref example_dep_tree.cc "example_dep_tree.cc" (for specification trees)
 * - \ref example_dep_tag.cc "example_dep_tag.cc" (for tags)
 */

namespace paludis
{
    /**
     * URI label base class.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE URILabel :
        private InstantiationPolicy<URILabel, instantiation_method::NonCopyableTag>,
        public virtual DeclareAbstractAcceptMethods<URILabel, MakeTypeList<
            URIMirrorsThenListedLabel, URIMirrorsOnlyLabel, URIListedOnlyLabel, URIListedThenMirrorsLabel,
            URILocalMirrorsOnlyLabel, URIManualOnlyLabel>::Type>
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~URILabel() = 0;

            ///\}

            /// Our text.
            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A concrete URI label class.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    template <typename T_>
    class PALUDIS_VISIBLE ConcreteURILabel :
        public URILabel,
        public ImplementAcceptMethods<URILabel, ConcreteURILabel<T_> >
    {
        private:
            const std::string _text;

        public:
            ///\name Basic operations
            ///\{

            ConcreteURILabel(const std::string &);
            ~ConcreteURILabel();

            ///\}

            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Convenience typedef alias to obtain our tag.
            typedef T_ Tag;
    };

    /**
     * Dependencies label base class.
     *
     * \since 0.42
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE DependenciesLabel :
        private InstantiationPolicy<DependenciesLabel, instantiation_method::NonCopyableTag>,
        public virtual DeclareAbstractAcceptMethods<DependenciesLabel, MakeTypeList<
            DependenciesBuildLabel, DependenciesRunLabel, DependenciesPostLabel, DependenciesCompileAgainstLabel,
            DependenciesFetchLabel, DependenciesInstallLabel, DependenciesSuggestionLabel,
            DependenciesRecommendationLabel, DependenciesTestLabel>::Type>
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~DependenciesLabel() = 0;

            ///\}

            /// Our text.
            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A concrete dependencies label class.
     *
     * \since 0.42
     * \ingroup g_dep_spec
     */
    template <typename T_>
    class PALUDIS_VISIBLE ConcreteDependenciesLabel :
        public DependenciesLabel,
        public ImplementAcceptMethods<DependenciesLabel, ConcreteDependenciesLabel<T_> >
    {
        private:
            const std::string _text;

        public:
            ///\name Basic operations
            ///\{

            ConcreteDependenciesLabel(const std::string &);
            ~ConcreteDependenciesLabel();

            ///\}

            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Convenience typedef alias to obtain our tag.
            typedef T_ Tag;
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class InstantiationPolicy<URILabel, instantiation_method::NonCopyableTag>;
    extern template class InstantiationPolicy<DependenciesLabel, instantiation_method::NonCopyableTag>;

    extern template class ConcreteURILabel<URIMirrorsThenListedLabelTag>;
    extern template class ConcreteURILabel<URIMirrorsOnlyLabelTag>;
    extern template class ConcreteURILabel<URIListedOnlyLabelTag>;
    extern template class ConcreteURILabel<URIListedThenMirrorsLabelTag>;
    extern template class ConcreteURILabel<URILocalMirrorsOnlyLabelTag>;
    extern template class ConcreteURILabel<URIManualOnlyLabelTag>;

    extern template class ConcreteDependenciesLabel<DependenciesBuildLabelTag>;
    extern template class ConcreteDependenciesLabel<DependenciesRunLabelTag>;
    extern template class ConcreteDependenciesLabel<DependenciesPostLabelTag>;
    extern template class ConcreteDependenciesLabel<DependenciesCompileAgainstLabelTag>;
    extern template class ConcreteDependenciesLabel<DependenciesFetchLabelTag>;
    extern template class ConcreteDependenciesLabel<DependenciesInstallLabelTag>;
    extern template class ConcreteDependenciesLabel<DependenciesSuggestionLabelTag>;
    extern template class ConcreteDependenciesLabel<DependenciesRecommendationLabelTag>;
    extern template class ConcreteDependenciesLabel<DependenciesTestLabelTag>;
#endif
}

#endif
