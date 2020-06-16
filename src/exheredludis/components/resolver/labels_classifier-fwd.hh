/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_LABELS_CLASSIFIER_FWD_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_LABELS_CLASSIFIER_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        struct LabelsClassifier;

        bool is_suggestion(
                const Environment * const,
                const std::shared_ptr<const PackageID> &,
                const SanitisedDependency &) PALUDIS_ATTRIBUTE((warn_unused_result));

        bool is_recommendation(
                const Environment * const,
                const std::shared_ptr<const PackageID> &,
                const SanitisedDependency &) PALUDIS_ATTRIBUTE((warn_unused_result));

        bool is_just_build_dep(
                const Environment * const,
                const std::shared_ptr<const PackageID> &,
                const SanitisedDependency &) PALUDIS_ATTRIBUTE((warn_unused_result));

        bool is_just_fetch_dep(
                const Environment * const,
                const std::shared_ptr<const PackageID> &,
                const SanitisedDependency &) PALUDIS_ATTRIBUTE((warn_unused_result));

        bool is_run_or_post_dep(
                const Environment * const,
                const std::shared_ptr<const PackageID> &,
                const SanitisedDependency &) PALUDIS_ATTRIBUTE((warn_unused_result));

        bool is_compiled_against_dep(
                const Environment * const,
                const std::shared_ptr<const PackageID> &,
                const SanitisedDependency &) PALUDIS_ATTRIBUTE((warn_unused_result));

        bool is_enabled_dep(
                const Environment * const,
                const std::shared_ptr<const PackageID> &,
                const SanitisedDependency &) PALUDIS_ATTRIBUTE((warn_unused_result));
    }
}

#endif
