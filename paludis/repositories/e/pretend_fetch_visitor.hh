/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_PRETEND_FETCH_VISITOR_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_PRETEND_FETCH_VISITOR_HH 1

#include <paludis/action-fwd.hh>
#include <paludis/dep_label-fwd.hh>
#include <paludis/dep_tree.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    namespace erepository
    {
        class PALUDIS_VISIBLE PretendFetchVisitor :
            private PrivateImplementationPattern<PretendFetchVisitor>,
            public ConstVisitor<FetchableURISpecTree>
        {
            public:
                PretendFetchVisitor(
                        const Environment * const,
                        const std::tr1::shared_ptr<const PackageID> &,
                        const EAPI & eapi,
                        const FSEntry & distdir,
                        const bool fetch_unneeded,
                        const std::tr1::shared_ptr<const URILabel> & initial_label,
                        PretendFetchAction & action);

                ~PretendFetchVisitor();

                void visit_sequence(const AllDepSpec &,
                        FetchableURISpecTree::ConstSequenceIterator,
                        FetchableURISpecTree::ConstSequenceIterator);

                void visit_sequence(const ConditionalDepSpec &,
                        FetchableURISpecTree::ConstSequenceIterator,
                        FetchableURISpecTree::ConstSequenceIterator);

                void visit_leaf(const URILabelsDepSpec &);

                void visit_leaf(const FetchableURIDepSpec &);
        };

    }
}

#endif
