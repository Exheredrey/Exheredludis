/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_ACCOUNTS_ACCOUNTS_REPOSITORY_STORE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_ACCOUNTS_ACCOUNTS_REPOSITORY_STORE_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/metadata_key-fwd.hh>
#include <memory>

namespace paludis
{
    namespace accounts_repository
    {
        class AccountsRepository;

        class PALUDIS_VISIBLE AccountsRepositoryStore
        {
            private:
                Pimp<AccountsRepositoryStore> _imp;

                void _load(const RepositoryName &);

                void _load_one(
                        const RepositoryName &,
                        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & from_repo,
                        const FSPath & dir);

                void _load_one_users(
                        const RepositoryName &,
                        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & from_repo,
                        const FSPath & dir);

                void _load_one_user(
                        const RepositoryName &,
                        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & from_repo,
                        const FSPath & file);

                void _load_one_groups(
                        const RepositoryName &,
                        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & from_repo,
                        const FSPath & dir);

                void _load_one_group(
                        const RepositoryName &,
                        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & from_repo,
                        const FSPath & file);

            public:
                AccountsRepositoryStore(
                        const Environment * const,
                        const RepositoryName &,
                        const bool installed);

                ~AccountsRepositoryStore();

                bool has_category_named(const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                bool has_package_named(const QualifiedPackageName & q) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const CategoryNamePartSet> category_names() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const CategoryNamePartSet> unimportant_category_names() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const QualifiedPackageNameSet> package_names(
                        const CategoryNamePart & c) const PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const PackageIDSequence> package_ids(
                        const QualifiedPackageName & p) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class Pimp<accounts_repository::AccountsRepositoryStore>;
}

#endif
