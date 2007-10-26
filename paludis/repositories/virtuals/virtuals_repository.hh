/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_VIRTUALS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_VIRTUALS_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/map-fwd.hh>

namespace paludis
{
    /**
     * A repository holding packages representing virtuals.
     *
     * \ingroup grpvirtualsrepository
     */
    class PALUDIS_VISIBLE VirtualsRepository :
        public Repository,
        public RepositoryMakeVirtualsInterface,
        private PrivateImplementationPattern<VirtualsRepository>,
        public tr1::enable_shared_from_this<VirtualsRepository>
    {
        private:
            void need_names() const;
            void need_ids() const;

        public:
            ///\name Basic operations
            ///\{

            VirtualsRepository(const Environment * const env);

            virtual ~VirtualsRepository();

            ///\}

            /**
             * Create a VirtualsRepository instance.
             */
            static tr1::shared_ptr<Repository> make_virtuals_repository(
                    Environment * const env,
                    tr1::shared_ptr<const Map<std::string, std::string> >);

            virtual tr1::shared_ptr<const CategoryNamePartSet> unimportant_category_names() const;

            virtual void invalidate();

            virtual void invalidate_masks();

            virtual bool can_be_favourite_repository() const;

            /* Repository */

            virtual tr1::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartSet> category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

            /* RepositoryMakeVirtualsInterface */

            virtual const tr1::shared_ptr<const PackageID> make_virtual_package_id(
                    const QualifiedPackageName & virtual_name, const tr1::shared_ptr<const PackageID> & provider) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

    };
}

#endif
