/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SHOW_SUGGEST_VISITOR_HH
#define PALUDIS_GUARD_PALUDIS_SHOW_SUGGEST_VISITOR_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/legacy/dep_list-fwd.hh>
#include <paludis/dep_spec-fwd.hh>

/** \file
 * Declarations for ShowSuggestVisitor, which is used internally by DepList.
 *
 * \ingroup g_dep_list
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Used by DepList to add suggested deps.
     *
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    class ShowSuggestVisitor :
        private Pimp<ShowSuggestVisitor>
    {
        public:
            ///\name Basic operations
            ///\{

            ShowSuggestVisitor(DepList * const dd, const std::shared_ptr<const DestinationsSet> & ddd,
                    const Environment * const, const std::shared_ptr<const PackageID> &, bool, bool);
            ~ShowSuggestVisitor();

            ///\}

            ///\name Visitor operations
            ///\{

            void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node);

            ///\}
    };
}

#endif
