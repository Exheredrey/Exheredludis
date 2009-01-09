/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_DEP_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_DEP_PRINTER_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/spec_tree.hh>
#include <paludis/formatter.hh>
#include <paludis/environment-fwd.hh>

namespace paludis
{
    namespace unpackaged_repositories
    {
        class PALUDIS_VISIBLE DepPrinter :
            private PrivateImplementationPattern<DepPrinter>
        {
            public:
                DepPrinter(const Environment * const, const DependencySpecTree::ItemFormatter &, const bool);
                ~DepPrinter();

                const std::string result() const;

                void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node);
                void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node);
                void visit(const DependencySpecTree::NodeType<DependencyLabelsDepSpec>::Type & node);
                void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node);
                void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node);
                void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node);
                void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node);
        };
    }
}

#endif
