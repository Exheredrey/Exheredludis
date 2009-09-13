/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_FILTER_HH
#define PALUDIS_GUARD_PALUDIS_FILTER_HH 1

#include <paludis/filter-fwd.hh>
#include <paludis/filter_handler-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/action-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    class PALUDIS_VISIBLE Filter :
        private PrivateImplementationPattern<Filter>
    {
        protected:
            Filter(const std::tr1::shared_ptr<const FilterHandler> &);

        public:
            Filter(const Filter &);
            Filter & operator= (const Filter &);
            ~Filter();

            std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<const RepositoryNameSet> repositories(
                    const Environment * const,
                    const std::tr1::shared_ptr<const RepositoryNameSet> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<const CategoryNamePartSet> categories(
                    const Environment * const,
                    const std::tr1::shared_ptr<const RepositoryNameSet> &,
                    const std::tr1::shared_ptr<const CategoryNamePartSet> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<const QualifiedPackageNameSet> packages(
                    const Environment * const,
                    const std::tr1::shared_ptr<const RepositoryNameSet> &,
                    const std::tr1::shared_ptr<const QualifiedPackageNameSet> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<const PackageIDSet> ids(
                    const Environment * const,
                    const std::tr1::shared_ptr<const PackageIDSet> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    namespace filter
    {
        class PALUDIS_VISIBLE All :
            public Filter
        {
            public:
                All();
        };

        template <typename>
        class PALUDIS_VISIBLE SupportsAction :
            public Filter
        {
            public:
                SupportsAction();
        };

        class PALUDIS_VISIBLE NotMasked :
            public Filter
        {
            public:
                NotMasked();
        };

        class PALUDIS_VISIBLE InstalledAtRoot :
            public Filter
        {
            public:
                InstalledAtRoot(const FSEntry &);
        };

        class PALUDIS_VISIBLE And :
            public Filter
        {
            public:
                And(const Filter &, const Filter &);
        };

        class PALUDIS_VISIBLE SameSlot :
            public Filter
        {
            public:
                SameSlot(const std::tr1::shared_ptr<const PackageID> &);
        };

        class PALUDIS_VISIBLE Slot :
            public Filter
        {
            public:
                Slot(const SlotName &);
        };

        class PALUDIS_VISIBLE NoSlot :
            public Filter
        {
            public:
                NoSlot();
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<Filter>;
    extern template class filter::SupportsAction<InstallAction>;
    extern template class filter::SupportsAction<UninstallAction>;
    extern template class filter::SupportsAction<PretendAction>;
    extern template class filter::SupportsAction<ConfigAction>;
    extern template class filter::SupportsAction<FetchAction>;
    extern template class filter::SupportsAction<InfoAction>;
    extern template class filter::SupportsAction<PretendFetchAction>;
#endif
}

#endif
