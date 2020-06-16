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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_SPEC_DATA_HH
#define PALUDIS_GUARD_PALUDIS_DEP_SPEC_DATA_HH 1

#include <paludis/dep_spec_data-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/changed_choices-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/version_operator-fwd.hh>
#include <paludis/version_requirements-fwd.hh>
#include <paludis/slot_requirement-fwd.hh>
#include <paludis/additional_package_dep_spec_requirement-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/partially_made_package_dep_spec-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <string>
#include <memory>

namespace paludis
{
    /**
     * Data for a ConditionalDepSpec.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE ConditionalDepSpecData
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~ConditionalDepSpecData();

            ///\}

            /**
             * Fetch ourself as a string.
             */
            virtual std::string as_string() const = 0;

            /**
             * Fetch the result for condition_met.
             *
             * \since 0.58 takes env, package_id
             */
            virtual bool condition_met(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Fetch the result for condition_would_be_met_when.
             *
             * \since 0.58 takes env, package_id
             */
            virtual bool condition_would_be_met_when(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &,
                    const ChangedChoices &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Fetch the result for condition_meetable.
             *
             * \since 0.58 takes env, package_id
             */
            virtual bool condition_meetable(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * Data for a PackageDepSpec.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE PackageDepSpecData
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~PackageDepSpecData();

            ///\}

            /**
             * Fetch ourself as a string.
             */
            virtual std::string as_string() const = 0;

            /**
             * Fetch the package name (may be a zero pointer).
             */
            virtual std::shared_ptr<const QualifiedPackageName> package_ptr() const = 0;

            /**
             * Fetch the package name part, if wildcarded, or a zero pointer otherwise.
             */
            virtual std::shared_ptr<const PackageNamePart> package_name_part_ptr() const = 0;

            /**
             * Fetch the category name part, if wildcarded, or a zero pointer otherwise.
             */
            virtual std::shared_ptr<const CategoryNamePart> category_name_part_ptr() const = 0;

            /**
             * Fetch the version requirements (may be a zero pointer).
             */
            virtual std::shared_ptr<const VersionRequirements> version_requirements_ptr() const = 0;

            /**
             * Fetch the version requirements mode.
             */
            virtual VersionRequirementsMode version_requirements_mode() const = 0;

            /**
             * Fetch the slot name (may be a zero pointer).
             */
            virtual std::shared_ptr<const SlotRequirement> slot_requirement_ptr() const = 0;

            /**
             * Fetch the from-repository requirement (may be a zero pointer).
             */
            virtual std::shared_ptr<const RepositoryName> in_repository_ptr() const = 0;

            /**
             * Fetch the installable-to-repository requirement (may be a zero pointer).
             *
             * \since 0.32
             */
            virtual std::shared_ptr<const InstallableToRepository> installable_to_repository_ptr() const = 0;

            /**
             * Fetch the from-repository requirement (may be a zero pointer).
             */
            virtual std::shared_ptr<const RepositoryName> from_repository_ptr() const = 0;

            /**
             * Fetch the installed-at-path requirement (may be a zero pointer).
             *
             * \since 0.32
             */
            virtual std::shared_ptr<const FSPath> installed_at_path_ptr() const = 0;

            /**
             * Fetch the installable-to-path requirement (may be a zero pointer).
             *
             * \since 0.32
             */
            virtual std::shared_ptr<const InstallableToPath> installable_to_path_ptr() const = 0;

            /**
             * Fetch the additional requirements (may be a zero pointer).
             */
            virtual std::shared_ptr<const AdditionalPackageDepSpecRequirements> additional_requirements_ptr() const = 0;

            /**
             * Fetch options if we're being used to construct a new PartiallyMadePackageDepSpec.
             *
             * \since 0.38
             */
            virtual const PartiallyMadePackageDepSpecOptions options_for_partially_made_package_dep_spec() const = 0;
    };
}

#endif
