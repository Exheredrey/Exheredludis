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

#ifndef PALUDIS_GUARD_PALUDIS_METADATA_KEY_HH
#define PALUDIS_GUARD_PALUDIS_METADATA_KEY_HH 1

#include <paludis/metadata_key-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/dep_tree.hh>
#include <paludis/contents-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/formatter-fwd.hh>
#include <paludis/metadata_key_holder.hh>
#include <paludis/choice-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/remove_shared_ptr.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/type_list.hh>
#include <tr1/type_traits>
#include <string>

/** \file
 * Declarations for metadata key classes.
 *
 * \ingroup g_metadata_key
 *
 * \section Examples
 *
 * - \ref example_metadata_key.cc "example_metadata_key.cc" (for metadata keys)
 */

namespace paludis
{
    /**
     * A MetadataKey is a generic key that contains a particular piece of
     * information about a PackageID or Repository instance.
     *
     * A basic MetadataKey has:
     *
     * - A raw name. This is in a repository-defined format designed to closely
     *   represent the internal name. For example, ebuilds and VDB IDs use
     *   raw names like 'DESCRIPTION' and 'KEYWORDS', whereas CRAN uses names
     *   like 'Title' and 'BundleDescription'. The raw name is unique in a
     *   PackageID or Repository.
     *
     * - A human name. This is the name that should be used when outputting
     *   normally for a human to read.
     *
     * - A MetadataKeyType. This is a hint to clients as to whether the key
     *   should be displayed when outputting information about a package ID
     *   or Repository.
     *
     * Subclasses provide additional information, including the 'value' of the
     * key. A ConstVisitor using MetadataKeyVisitorTypes can be used to get more
     * detail.
     *
     * The header \ref paludis/literal_metadata_key.hh "literal_metadata_key.hh"
     * contains various concrete implementations of MetadataKey subclasses.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataKey :
        private InstantiationPolicy<MetadataKey, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<MetadataKey>,
        public virtual DeclareAbstractAcceptMethods<MetadataKey, MakeTypeList<
                MetadataCollectionKey<KeywordNameSet>,
                MetadataCollectionKey<Set<std::string> >,
                MetadataCollectionKey<Sequence<std::string> >,
                MetadataCollectionKey<PackageIDSequence>,
                MetadataCollectionKey<FSEntrySequence>,
                MetadataSpecTreeKey<DependencySpecTree>,
                MetadataSpecTreeKey<LicenseSpecTree>,
                MetadataSpecTreeKey<FetchableURISpecTree>,
                MetadataSpecTreeKey<SimpleURISpecTree>,
                MetadataSpecTreeKey<ProvideSpecTree>,
                MetadataSpecTreeKey<PlainTextSpecTree>,
                MetadataValueKey<std::string>,
                MetadataValueKey<long>,
                MetadataValueKey<bool>,
                MetadataValueKey<FSEntry>,
                MetadataValueKey<std::tr1::shared_ptr<const PackageID> >,
                MetadataValueKey<std::tr1::shared_ptr<const Contents> >,
                MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> >,
                MetadataValueKey<std::tr1::shared_ptr<const Choices> >,
                MetadataTimeKey,
                MetadataSectionKey
                >::Type>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataKey(const std::string & raw_name, const std::string & human_name, const MetadataKeyType);

        public:
            virtual ~MetadataKey() = 0;

            ///\}

            /**
             * Fetch our raw name.
             */
            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our human name.
             */
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our key type.
             */
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A MetadataSectionKey is a MetadataKey that holds a number of other
     * MetadataKey instances.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataSectionKey :
        public MetadataKey,
        public ImplementAcceptMethods<MetadataKey, MetadataSectionKey>,
        public MetadataKeyHolder
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataSectionKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~MetadataSectionKey();

            ///\}
    };

    /**
     * Extra methods for MetadataValueKey with certain item types.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     */
    template <typename C_>
    class ExtraMetadataValueKeyMethods
    {
    };

    /**
     * Extra methods for MetadataValueKey with long value type.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     */
    template <>
    class PALUDIS_VISIBLE ExtraMetadataValueKeyMethods<long>
    {
        public:
            virtual ~ExtraMetadataValueKeyMethods() = 0;

            /**
             * Return a formatted version of our value.
             */
            virtual std::string pretty_print() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * Extra methods for MetadataValueKey with bool value type.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     */
    template <>
    class PALUDIS_VISIBLE ExtraMetadataValueKeyMethods<bool>
    {
        public:
            virtual ~ExtraMetadataValueKeyMethods() = 0;

            /**
             * Return a formatted version of our value.
             */
            virtual std::string pretty_print() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * Extra methods for MetadataValueKey with PackageID value type.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     */
    template <>
    class PALUDIS_VISIBLE ExtraMetadataValueKeyMethods<std::tr1::shared_ptr<const PackageID> >
    {
        public:
            virtual ~ExtraMetadataValueKeyMethods() = 0;

            /**
             * Return a formatted version of our value, using the supplied Formatter to
             * format the item.
             */
            virtual std::string pretty_print(const Formatter<PackageID> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataValueKey is a MetadataKey that holds some simple type
     * as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <typename C_>
    class PALUDIS_VISIBLE MetadataValueKey :
        public MetadataKey,
        public ImplementAcceptMethods<MetadataKey, MetadataValueKey<C_> >,
        public virtual ExtraMetadataValueKeyMethods<C_>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataValueKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const C_ value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataTimeKey is a MetadataKey that has a time_t as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataTimeKey :
        public MetadataKey,
        public ImplementAcceptMethods<MetadataKey, MetadataTimeKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataTimeKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual time_t value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataCollectionKey is a MetadataKey that holds a Set of some kind of item
     * as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <typename C_>
    class PALUDIS_VISIBLE MetadataCollectionKey :
        public MetadataKey,
        public ImplementAcceptMethods<MetadataKey, MetadataCollectionKey<C_> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataCollectionKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const std::tr1::shared_ptr<const C_> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const Formatter<
                    typename std::tr1::remove_const<typename RemoveSharedPtr<typename C_::value_type>::Type>::type> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataSpecTreeKey<> is a MetadataKey that holds a spec tree of some
     * kind as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <typename C_>
    class PALUDIS_VISIBLE MetadataSpecTreeKey :
        public MetadataKey,
        public ImplementAcceptMethods<MetadataKey, MetadataSpecTreeKey<C_> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataSpecTreeKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const std::tr1::shared_ptr<const typename C_::ConstItem> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a multiline-line indented and formatted version of our
             * value, using the supplied Formatter to format individual items.
             */
            virtual std::string pretty_print(const typename C_::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const typename C_::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataSpecTreeKey<FetchableURISpecTree> is a MetadataKey that holds
     * a FetchableURISpecTree as its value.
     *
     * This specialisation of MetadataSpecTreeKey provides an additional
     * initial_label method.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <>
    class PALUDIS_VISIBLE MetadataSpecTreeKey<FetchableURISpecTree> :
        public MetadataKey,
        public ImplementAcceptMethods<MetadataKey, MetadataSpecTreeKey<FetchableURISpecTree> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataSpecTreeKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const std::tr1::shared_ptr<const FetchableURISpecTree::ConstItem> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a multiline-line indented and formatted version of our
             * value, using the supplied Formatter to format individual items.
             */
            virtual std::string pretty_print(const FetchableURISpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const FetchableURISpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a URILabel that represents the initial label to use when
             * deciding the behaviour of individual items in the heirarchy.
             */
            virtual const std::tr1::shared_ptr<const URILabel> initial_label() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataSpecTreeKey<DependencySpecTree> is a MetadataKey that holds
     * a FetchableURISpecTree as its value.
     *
     * This specialisation of MetadataSpecTreeKey provides an additional
     * initial_label method.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <>
    class PALUDIS_VISIBLE MetadataSpecTreeKey<DependencySpecTree> :
        public MetadataKey,
        public ImplementAcceptMethods<MetadataKey, MetadataSpecTreeKey<DependencySpecTree> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataSpecTreeKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const std::tr1::shared_ptr<const DependencySpecTree::ConstItem> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a multiline-line indented and formatted version of our
             * value, using the supplied Formatter to format individual items.
             */
            virtual std::string pretty_print(const DependencySpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const DependencySpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a DependencyLabelSequence that represents the initial labels to use when
             * deciding the behaviour of individual items in the heirarchy.
             */
            virtual const std::tr1::shared_ptr<const DependencyLabelSequence> initial_labels() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };
}

#endif
