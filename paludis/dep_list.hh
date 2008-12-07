/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_HH 1

#include <paludis/dep_spec-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/dep_tag.hh>
#include <paludis/dep_list_options.hh>
#include <paludis/dep_list-fwd.hh>
#include <paludis/handled_information-fwd.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/options.hh>
#include <paludis/version_spec.hh>
#include <tr1/functional>
#include <iosfwd>

/** \file
 * Declarations for DepList and related classes.
 *
 * \ingroup g_dep_list
 *
 * \section Examples
 *
 * - None at this time. Use InstallTask if you need to install things.
 */

namespace paludis
{
    namespace n
    {
        struct associated_entry;
        struct blocks;
        struct circular;
        struct dependency_tags;
        struct destination;
        struct downgrade;
        struct fall_back;
        struct generation;
        struct handled;
        struct installed_deps_post;
        struct installed_deps_pre;
        struct installed_deps_runtime;
        struct kind;
        struct match_package_options;
        struct new_slots;
        struct override_masks;
        struct package_id;
        struct reinstall;
        struct reinstall_scm;
        struct state;
        struct suggested;
        struct tags;
        struct target_type;
        struct uninstalled_deps_post;
        struct uninstalled_deps_pre;
        struct uninstalled_deps_runtime;
        struct uninstalled_deps_suggested;
        struct upgrade;
        struct use;
    }

    /**
     * A sequence of functions to try, in order, when overriding masks.
     *
     * \ingroup g_dep_list
     */
    typedef Sequence<std::tr1::function<bool (const PackageID &, const Mask &)> > DepListOverrideMasksFunctions;

    /**
     * An entry in a DepList.
     *
     * \see DepList
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    struct DepListEntry
    {
        NamedValue<n::associated_entry, const DepListEntry *> associated_entry;
        NamedValue<n::destination, std::tr1::shared_ptr<Repository> > destination;
        NamedValue<n::generation, long> generation;
        NamedValue<n::handled, std::tr1::shared_ptr<const DepListEntryHandled> > handled;
        NamedValue<n::kind, DepListEntryKind> kind;
        NamedValue<n::package_id, std::tr1::shared_ptr<const PackageID> > package_id;
        NamedValue<n::state, DepListEntryState> state;
        NamedValue<n::tags, std::tr1::shared_ptr<DepListEntryTags> > tags;
    };

    /**
     * Parameters for a DepList.
     *
     * \see DepList
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    struct PALUDIS_VISIBLE DepListOptions
    {
        DepListOptions();

        NamedValue<n::blocks, DepListBlocksOption> blocks;
        NamedValue<n::circular, DepListCircularOption> circular;
        NamedValue<n::dependency_tags, bool> dependency_tags;
        NamedValue<n::downgrade, DepListDowngradeOption> downgrade;
        NamedValue<n::fall_back, DepListFallBackOption> fall_back;
        NamedValue<n::installed_deps_post, DepListDepsOption> installed_deps_post;
        NamedValue<n::installed_deps_pre, DepListDepsOption> installed_deps_pre;
        NamedValue<n::installed_deps_runtime, DepListDepsOption> installed_deps_runtime;
        NamedValue<n::match_package_options, MatchPackageOptions> match_package_options;
        NamedValue<n::new_slots, DepListNewSlotsOption> new_slots;
        NamedValue<n::override_masks, std::tr1::shared_ptr<DepListOverrideMasksFunctions> > override_masks;
        NamedValue<n::reinstall, DepListReinstallOption> reinstall;
        NamedValue<n::reinstall_scm, DepListReinstallScmOption> reinstall_scm;
        NamedValue<n::suggested, DepListSuggestedOption> suggested;
        NamedValue<n::target_type, DepListTargetType> target_type;
        NamedValue<n::uninstalled_deps_post, DepListDepsOption> uninstalled_deps_post;
        NamedValue<n::uninstalled_deps_pre, DepListDepsOption> uninstalled_deps_pre;
        NamedValue<n::uninstalled_deps_runtime, DepListDepsOption> uninstalled_deps_runtime;
        NamedValue<n::uninstalled_deps_suggested, DepListDepsOption> uninstalled_deps_suggested;
        NamedValue<n::upgrade, DepListUpgradeOption> upgrade;
        NamedValue<n::use, DepListUseOption> use;
    };

    /**
     * Holds a list of dependencies in merge order.
     *
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepList :
        private InstantiationPolicy<DepList, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<DepList>
    {
        protected:
            class AddVisitor;
            friend class AddVisitor;

            /**
             * Find an appropriate destination for a package.
             */
            std::tr1::shared_ptr<Repository> find_destination(const PackageID &,
                    const std::tr1::shared_ptr<const DestinationsSet> &);

            /**
             * Add a DepSpec with role context.
             */
            void add_in_role(const bool only_if_not_suggested_label, DependencySpecTree::ConstItem &, const std::string & role,
                    const std::tr1::shared_ptr<const DestinationsSet> &);

            /**
             * Return whether we prefer the first parameter, which is installed,
             * over the second, which isn't.
             */
            bool prefer_installed_over_uninstalled(const PackageID &,
                    const PackageID &);

            /**
             * Add a package to the list.
             */
            void add_package(const std::tr1::shared_ptr<const PackageID> &, const std::tr1::shared_ptr<const DepTag> &,
                    const PackageDepSpec &, const std::tr1::shared_ptr<DependencySpecTree::ConstItem> &,
                    const std::tr1::shared_ptr<const DestinationsSet> & destinations);

            /**
             * Add an already installed package to the list.
             */
            void add_already_installed_package(const std::tr1::shared_ptr<const PackageID> &, const std::tr1::shared_ptr<const DepTag> &,
                    const PackageDepSpec &, const std::tr1::shared_ptr<DependencySpecTree::ConstItem> &,
                    const std::tr1::shared_ptr<const DestinationsSet> & destinations);

            /**
             * Add an error package to the list.
             */
            void add_error_package(const std::tr1::shared_ptr<const PackageID> &, const DepListEntryKind,
                    const PackageDepSpec &, const std::tr1::shared_ptr<DependencySpecTree::ConstItem> &);

            /**
             * Add predependencies.
             */
            void add_predeps(DependencySpecTree::ConstItem &, const DepListDepsOption, const std::string &,
                    const std::tr1::shared_ptr<const DestinationsSet> & destinations, const bool only_if_not_suggested_label);

            /**
             * Add postdependencies.
             */
            void add_postdeps(DependencySpecTree::ConstItem &, const DepListDepsOption, const std::string &,
                    const std::tr1::shared_ptr<const DestinationsSet> & destinations, const bool only_if_not_suggested_label);

            /**
             * Return whether the specified PackageID is matched by
             * the top level target.
             */
            bool is_top_level_target(const PackageID &) const;

            void add_not_top_level(
                    const bool only_if_not_suggested_label,
                    DependencySpecTree::ConstItem &,
                    const std::tr1::shared_ptr<const DestinationsSet> & target_destinations,
                    const std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > & conditions);

        public:
            ///\name Basic operations
            ///\{

            DepList(const Environment * const, const DepListOptions &);

            virtual ~DepList();

            ///\}

            ///\name Iterate over our dependency list entries.
            ///\{

            struct IteratorTag;
            typedef WrappedForwardIterator<IteratorTag, DepListEntry> Iterator;

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const DepListEntry> ConstIterator;

            Iterator begin();
            Iterator end();

            ConstIterator begin() const;
            ConstIterator end() const;

            ///\}

            /**
             * Our options.
             */
            std::tr1::shared_ptr<DepListOptions> options();

            /**
             * Our options.
             */
            const std::tr1::shared_ptr<const DepListOptions> options() const;

            /**
             * Add the packages required to resolve an additional dependency
             * spec.
             */
            void add(SetSpecTree::ConstItem &,
                    const std::tr1::shared_ptr<const DestinationsSet> & target_destinations);

            /**
             * Add the packages required to resolve an additional dependency
             * spec.
             */
            void add(const PackageDepSpec &,
                    const std::tr1::shared_ptr<const DestinationsSet> & target_destinations);

            /**
             * Manually add a DepListEntry to the list.
             *
             * Does not work well with ordered resolution, and does not do much
             * sanity checking. This is used by InstallTask to implement resume
             * commands and the exec command.
             */
            Iterator push_back(const DepListEntry &);

            /**
             * Clear the list.
             */
            void clear();

            /**
             * Return whether a spec structure is already installed.
             */
            bool already_installed(DependencySpecTree::ConstItem &,
                    const std::tr1::shared_ptr<const DestinationsSet> & target_destinations) const;

            /**
             * Return whether a PackageID has been replaced.
             */
            bool replaced(const PackageID &) const;

            /**
             * Return whether a spec matches an item in the list.
             */
            bool match_on_list(const PackageDepSpec &) const;

            /**
             * Whether we have any errors.
             */
            bool has_errors() const;

            /**
             * Add a suggested package to the list.
             */
            void add_suggested_package(const std::tr1::shared_ptr<const PackageID> &,
                    const PackageDepSpec &, const std::tr1::shared_ptr<DependencySpecTree::ConstItem> &,
                    const std::tr1::shared_ptr<const DestinationsSet> & destinations);
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<DepList>;
#endif
}

#endif
