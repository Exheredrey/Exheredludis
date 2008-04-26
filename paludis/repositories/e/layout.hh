/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
 * Copyright (c) 2006 Danny van Dyk
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_LAYOUT_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_LAYOUT_HH 1

#include <paludis/name-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/util/map-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    class ERepository;

    namespace erepository
    {
        class ERepositoryEntries;

        /**
         * Manages the layout of a ERepository.
         *
         * \ingroup grperepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE Layout
        {
            private:
                std::tr1::shared_ptr<const FSEntry> _master_repository_location;

            protected:
                ///\name Basic operations
                ///\{

                Layout(std::tr1::shared_ptr<const FSEntry> master_repository_location);

                ///\}

            public:
                ///\name Basic operations
                ///\{

                virtual ~Layout() = 0;

                ///\}

                ///\name Configuration information
                ///\{

                std::tr1::shared_ptr<const FSEntry> master_repository_location() const;

                ///\}

                ///\name Layout operations
                ///\{

                virtual bool has_category_named(const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual bool has_package_named(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSEntry categories_file() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const CategoryNamePartSet> category_names() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                        const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const PackageIDSequence> package_ids(
                        const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSEntry info_packages_file(const FSEntry &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSEntry info_variables_file(const FSEntry &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSEntry package_directory(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSEntry category_directory(const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSEntry binary_ebuild_location(const QualifiedPackageName &, const VersionSpec &,
                        const std::string & eapi) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const FSEntrySequence> arch_list_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const FSEntrySequence> repository_mask_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const FSEntrySequence> profiles_desc_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const FSEntrySequence> mirror_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const FSEntrySequence> use_desc_dirs() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSEntry profiles_base_dir() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs_global() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs_category(const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs_package(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<const FSEntrySequence> licenses_dirs() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::tr1::shared_ptr<Map<FSEntry, std::string> > manifest_files(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSEntry sync_filter_file() const;

                virtual void invalidate_masks() = 0;

                ///\}
        };

        /**
         * Thrown if a layout of the specified type does not exist.
         *
         * \ingroup grpexceptions
         * \ingroup grperepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE NoSuchLayoutType :
            public ConfigurationError
        {
            public:
                /**
                 * Constructor.
                 */
                NoSuchLayoutType(const std::string & format) throw ();
        };

        /**
         * Virtual constructor for Layout.
         *
         * \ingroup grperepository
         */
        class PALUDIS_VISIBLE LayoutMaker :
            public VirtualConstructor<std::string,
                std::tr1::shared_ptr<Layout> (*) (const ERepository * const, const FSEntry &,
                        std::tr1::shared_ptr<const ERepositoryEntries>,
                        std::tr1::shared_ptr<const FSEntry>),
                virtual_constructor_not_found::ThrowException<NoSuchLayoutType> >,
            public InstantiationPolicy<LayoutMaker, instantiation_method::SingletonTag>
        {
            friend class InstantiationPolicy<LayoutMaker, instantiation_method::SingletonTag>;

            private:
                LayoutMaker();
        };
    }
}

#endif
