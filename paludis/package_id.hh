/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_ID_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_ID_HH 1

#include <paludis/package_id-fwd.hh>

#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/operators.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/tr1_memory.hh>

#include <paludis/action-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/dep_tree-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/version_spec-fwd.hh>

#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>

/** \file
 * Declarations for PackageID classes.
 *
 * \ingroup g_package_id
 *
 * \section Examples
 *
 * - \ref example_package_id.cc "example_package_id.cc" (for package IDs)
 * - \ref example_action.cc "example_action.cc" (for actions)
 * - \ref example_metadata_key.cc "example_metadata_key.cc" (for metadata keys)
 * - \ref example_mask.cc "example_mask.cc" (for masks)
 */

namespace paludis
{
    class PackageDatabase;

    /**
     * A PackageID represents a unique package version in a particular
     * Repository.
     *
     * All PackageID instances have some basic identification data:
     *
     * - A name
     * - A version
     * - A slot (which will be '0' if slots aren't used)
     * - An owning repository
     *
     * It should be noted that the above together are not sufficient to uniquely
     * identify a package. Some repositories support multiple slots per version
     * of a package, and some repositories support additional affixes that
     * prevent a package from being uniquely identifiable merely by the above.
     *
     * PackageID instances also have:
     *
     * - A collection of metadata keys. Some of these keys have a particular
     *   specific role in certain places. These can be fetched via blah_key()
     *   methods, all of which may return an empty pointer. Other keys have no
     *   meaningful global role, and can only be fetched using the iteration
     *   methods.
     *
     * - A collection (often empty) of masks. A masked package cannot be
     *   installed.
     *
     * A PackageID instance may support certain actions, which are represented
     * via an Action subclass instance.
     */
    class PALUDIS_VISIBLE PackageID :
        private InstantiationPolicy<PackageID, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<PackageID>,
        public equality_operators::HasEqualityOperators
    {
        protected:
            /**
             * Add a new MetadataKey, which must not use the same raw name as
             * any previous MetadataKey added to this ID.
             */
            virtual void add_metadata_key(const tr1::shared_ptr<const MetadataKey> &) const;

            /**
             * Add a new Mask.
             */
            virtual void add_mask(const tr1::shared_ptr<const Mask> &) const;

            /**
             * This method will be called before any of the metadata key
             * iteration methods does its work. It can be used by subclasses to
             * implement as-needed loading of keys.
             */
            virtual void need_keys_added() const = 0;

            /**
             * This method will be called before any of the mask iteration
             * methods does its work. It can be used by subclasses to implement
             * as-needed loading of masks.
             */
            virtual void need_masks_added() const = 0;

        public:
            ///\name Basic operations
            ///\{

            PackageID();
            virtual ~PackageID() = 0;

            ///\}

            /**
             * Return our canonical string representation.
             *
             * This method (which is called by paludis::stringify()) should be
             * used when outputting a PackageID or a PackageID's version. Some
             * repositories need to display additional information to identify a
             * package, so outputting merely the name and / or version may be
             * insufficient.
             */
            virtual const std::string canonical_form(const PackageIDCanonicalForm) const = 0;

            /**
             * What is our package name?
             *
             * Use canonical_form instead for outputtting.
             */
            virtual const QualifiedPackageName name() const = 0;

            /**
             * What is our package version?
             *
             * Use canonical_form instead for outputtting.
             */
            virtual const VersionSpec version() const = 0;

            /**
             * What is our package's slot?
             */
            virtual const SlotName slot() const = 0;

            /**
             * What is our owning repository?
             */
            virtual const tr1::shared_ptr<const Repository> repository() const = 0;

            ///\name Specific metadata keys
            ///\{

            /**
             * The virtual_for_key, if non-zero, indicates that we are an
             * (old-style) virtual for another package. This affects dependency
             * resolution.
             */
            virtual const tr1::shared_ptr<const MetadataPackageIDKey> virtual_for_key() const = 0;

            /**
             * The keywords_key, if non-zero, is used by FindUnusedPackagesTask
             * to determine whether a package is unused.
             */
            virtual const tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> > keywords_key() const = 0;

            /**
             * The iuse_key, if non-zero, is used when displaying single-line
             * install-pretend output, and when resolving where DepList's
             * DepListReinstallOption is dl_reinstall_if_use_changed.
             */
            virtual const tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> > iuse_key() const = 0;

            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license_key() const = 0;

            /**
             * The provide_key, if non-zero, indicates that a package provides
             * certain old-style virtuals. This affects dependency resolution.
             */
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > provide_key() const = 0;

            /**
             * The contains_key, if non-zero, indicates that a package contains
             * other packages. This affects dependency resolution. */
            virtual const tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> > contains_key() const = 0;

            /**
             * The contained_in_key, if non-zero, indicates that a package is
             * contained in another package. This affects dependency resolution.
             */
            virtual const tr1::shared_ptr<const MetadataPackageIDKey> contained_in_key() const = 0;

            /**
             * The build_dependencies_key, if non-zero, indicates a package's
             * build-time dependencies.
             */
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key() const = 0;

            /**
             * The run_dependencies_key, if non-zero, indicates a package's
             * run-time dependencies.
             */
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key() const = 0;

            /**
             * The post_dependencies_key, if non-zero, indicates a package's
             * post-merge dependencies.
             */
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies_key() const = 0;

            /**
             * The suggested_dependencies_key, if non-zero, indicates a package's
             * suggested post-merge dependencies.
             */
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > suggested_dependencies_key() const = 0;

            /**
             * The src_uri_key, if non-zero, indicates files that have to be fetched
             * in order to install a package.
             */
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > src_uri_key() const = 0;

            /**
             * The homepage_key, if non-zero, describes a package's homepages.
             */
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage_key() const = 0;

            /**
             * The short_description_key, if non-zero, provides a short (no more
             * than a few hundred characters) description of a package.
             */
            virtual const tr1::shared_ptr<const MetadataStringKey> short_description_key() const = 0;

            /**
             * The long_description_key, if non-zero, provides a long
             * description of a package.
             */
            virtual const tr1::shared_ptr<const MetadataStringKey> long_description_key() const = 0;

            /**
             * The contents_key, if non-zero, contains the contents of a
             * package. For installed packages, this means the files installed;
             * for installable packages, this means the files that will be
             * installed (if known, which it may be for some binary packages).
             */
            virtual const tr1::shared_ptr<const MetadataContentsKey> contents_key() const = 0;

            /**
             * The installed_time_key, if non-zero, contains the time a package
             * was installed. It affects dependency resolution if DepList is
             * using dl_reinstall_scm_daily or dl_reinstall_scm_weekly.
             */
            virtual const tr1::shared_ptr<const MetadataTimeKey> installed_time_key() const = 0;

            /**
             * The source_origin_key, if non-zero, contains a string describing
             * the source repository whence a package originated.
             */
            virtual const tr1::shared_ptr<const MetadataStringKey> source_origin_key() const = 0;

            /**
             * The binary_origin_key, if non-zero, contains a string describing
             * the binary repository whence a package originated.
             */
            virtual const tr1::shared_ptr<const MetadataStringKey> binary_origin_key() const = 0;

            /**
             * The fs_location_key, if non-zero, indicates the filesystem
             * location (for example, the ebuild file or VDB directory) that
             * best describes the location of a PackageID.
             */
            virtual const tr1::shared_ptr<const MetadataFSEntryKey> fs_location_key() const = 0;

            ///\}

            ///\name Finding and iterating over metadata keys
            ///\{

            typedef libwrapiter::ForwardIterator<PackageID, tr1::shared_ptr<const MetadataKey> > MetadataConstIterator;
            MetadataConstIterator begin_metadata() const PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataConstIterator end_metadata() const PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataConstIterator find_metadata(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Actions
            ///\{

            /**
             * Do we support a particular action?
             *
             * Attempting to call perform_action with an unsupported action type
             * will lead to an UnsupportedActionError. However, in performance
             * critical code and in situations where creating a full Action
             * subclass instance is non-trivial, supports_action is much
             * simpler.
             */
            virtual bool supports_action(const SupportsActionTestBase &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Perform the specified action.
             */
            virtual void perform_action(Action &) const = 0;

            ///\}

            ///\name Masks
            ///\{

            typedef libwrapiter::ForwardIterator<PackageID, tr1::shared_ptr<const Mask> > MasksConstIterator;
            MasksConstIterator begin_masks() const PALUDIS_ATTRIBUTE((warn_unused_result));
            MasksConstIterator end_masks() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Do we have any masks? Equivalent to begin_masks() != end_masks().
             */
            bool masked() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Invalidate any masks.
             *
             * PackageID implementations may cache masks. This can cause
             * problems if the operating environment changes. Calling this
             * method will clear any masks held by the PackageID.
             */
            virtual void invalidate_masks() const;

            /**
             * Do we break Portage?
             *
             * This method may be used by Environment implementations to apply a "we don't
             * want packages that break Portage" mask.
             */
            virtual bool breaks_portage() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Extra comparisons
            ///\{

            /**
             * Perform an arbitrary less than comparison on another PackageID.
             *
             * Used by PackageIDSetComparator and operator==. This function
             * should not be used by anything else.
             *
             * This function will only be called if the other PackageID has the
             * same name, version and repository as this. If this is not enough
             * to uniquely identify an ID (e.g. if there is an affix, or if multiple
             * slots per version are allowed), then this function's
             * implementation must differentiate appropriately.
             */
            virtual bool arbitrary_less_than_comparison(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Provide any additional hash information for a PackageID.
             *
             * The standard PackageID hash incorporates the repository name, the
             * package name and the version of the package. If this function is
             * defined, its value is also used when determining a hash. This can
             * provide increased performance if a repository uses affixes or
             * multiple slots per version.
             */
            virtual std::size_t extra_hash_value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}
    };

    /**
     * PackageID instances are equality-comparable.
     *
     * \ingroup g_package_id
     * \since 0.26
     */
    bool operator== (const PackageID &, const PackageID &) PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    /**
     * A comparison functor that uses PackageID::arbitrary_less_than_comparison
     * to provide a meaningless but stable ordering for PackageIDSet.
     *
     * \ingroup g_package_id
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PackageIDSetComparator
    {
        public:
            /**
             * Perform an arbitrary less-than comparison.
             */
            bool operator() (const tr1::shared_ptr<const PackageID> &,
                    const tr1::shared_ptr<const PackageID> &) const;
    };

    /**
     * A comparison functor that provides a less-than comparison on PackageID
     * instances according to, in order, their name, their version, their
     * repository's importance according to the supplied PackageDatabase,
     * and PackageID::arbitrary_less_than_comparison.
     *
     * \ingroup g_package_id
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PackageIDComparator :
        private PrivateImplementationPattern<PackageIDComparator>
    {
        public:
            ///\name Standard library typedefs
            ///\{

            typedef bool result_type;

            ///\}

            ///\name Basic operations
            ///\{

            PackageIDComparator(const PackageDatabase * const);
            ~PackageIDComparator();

            ///\}

            /**
             * Perform the less-than comparison.
             */
            bool operator() (const tr1::shared_ptr<const PackageID> &,
                    const tr1::shared_ptr<const PackageID> &) const;
    };
}

#endif
