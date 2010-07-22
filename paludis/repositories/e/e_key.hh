/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_E_KEY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_E_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/repositories/e/eapi-fwd.hh>

namespace paludis
{
    class ERepository;

    namespace erepository
    {
        class ERepositoryID;

        class EMutableRepositoryMaskInfoKey :
            public MetadataValueKey<std::shared_ptr<const RepositoryMaskInfo> >
        {
            private:
                std::shared_ptr<const RepositoryMaskInfo> _value;
                const std::string _r;
                const std::string _h;
                const MetadataKeyType _t;

            public:
                EMutableRepositoryMaskInfoKey(const std::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::shared_ptr<const RepositoryMaskInfo> &, const MetadataKeyType);
                ~EMutableRepositoryMaskInfoKey();

                virtual const std::shared_ptr<const RepositoryMaskInfo> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                void set_value(const std::shared_ptr<const RepositoryMaskInfo> &);

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EDependenciesKey :
            public MetadataSpecTreeKey<DependencySpecTree>,
            private PrivateImplementationPattern<EDependenciesKey>
        {
            public:
                EDependenciesKey(
                        const Environment * const,
                        const std::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &,
                        const std::shared_ptr<const DependenciesLabelSequence> &,
                        const MetadataKeyType);
                ~EDependenciesKey();

                virtual const std::shared_ptr<const DependencySpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const DependencySpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const DependencySpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const DependenciesLabelSequence> initial_labels() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EFetchableURIKey :
            public MetadataSpecTreeKey<FetchableURISpecTree>,
            private PrivateImplementationPattern<EFetchableURIKey>
        {
            public:
                EFetchableURIKey(const Environment * const,
                        const std::shared_ptr<const ERepositoryID> &,
                        const std::shared_ptr<const EAPIMetadataVariable> &,
                        const std::string &,
                        const MetadataKeyType);
                ~EFetchableURIKey();

                virtual const std::shared_ptr<const FetchableURISpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const FetchableURISpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const FetchableURISpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const URILabel> initial_label() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ESimpleURIKey :
            public MetadataSpecTreeKey<SimpleURISpecTree>,
            private PrivateImplementationPattern<ESimpleURIKey>
        {
            public:
                ESimpleURIKey(const Environment * const,
                        const std::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~ESimpleURIKey();

                virtual const std::shared_ptr<const SimpleURISpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const SimpleURISpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const SimpleURISpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EPlainTextSpecKey :
            public MetadataSpecTreeKey<PlainTextSpecTree>,
            private PrivateImplementationPattern<EPlainTextSpecKey>
        {
            public:
                EPlainTextSpecKey(const Environment * const,
                        const std::shared_ptr<const ERepositoryID> &,
                        const std::shared_ptr<const EAPIMetadataVariable> &,
                        const std::string &, const MetadataKeyType);
                ~EPlainTextSpecKey();

                virtual const std::shared_ptr<const PlainTextSpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const PlainTextSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const PlainTextSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EMyOptionsKey :
            public MetadataSpecTreeKey<PlainTextSpecTree>,
            private PrivateImplementationPattern<EMyOptionsKey>
        {
            public:
                EMyOptionsKey(const Environment * const,
                        const std::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EMyOptionsKey();

                virtual const std::shared_ptr<const PlainTextSpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const PlainTextSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const PlainTextSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EProvideKey :
            public MetadataSpecTreeKey<ProvideSpecTree>,
            private PrivateImplementationPattern<EProvideKey>
        {
            public:
                EProvideKey(const Environment * const,
                        const std::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EProvideKey();

                virtual const std::shared_ptr<const ProvideSpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const ProvideSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const ProvideSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ELicenseKey :
            public MetadataSpecTreeKey<LicenseSpecTree>,
            private PrivateImplementationPattern<ELicenseKey>
        {
            public:
                ELicenseKey(
                        const Environment * const,
                        const std::shared_ptr<const ERepositoryID> &,
                        const std::shared_ptr<const EAPIMetadataVariable> &,
                        const std::string &, const MetadataKeyType);
                ~ELicenseKey();

                virtual const std::shared_ptr<const LicenseSpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const LicenseSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const LicenseSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EKeywordsKey :
            public MetadataCollectionKey<KeywordNameSet>,
            private PrivateImplementationPattern<EKeywordsKey>
        {
            public:
                EKeywordsKey(
                        const Environment * const,
                        const std::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EKeywordsKey();

                const std::shared_ptr<const KeywordNameSet> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const Formatter<KeywordName> &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EStringSetKey :
            public MetadataCollectionKey<Set<std::string> >,
            private PrivateImplementationPattern<EStringSetKey>
        {
            public:
                EStringSetKey(const std::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EStringSetKey();

                const std::shared_ptr<const Set<std::string> > value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const Formatter<std::string> &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EContentsKey :
            public MetadataValueKey<std::shared_ptr<const Contents> >,
            private PrivateImplementationPattern<EContentsKey>
        {
            public:
                EContentsKey(
                        const std::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const FSEntry &, const MetadataKeyType);
                ~EContentsKey();

                const std::shared_ptr<const Contents> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EMTimeKey :
            public MetadataTimeKey,
            private PrivateImplementationPattern<EMTimeKey>
        {
            public:
                EMTimeKey(const std::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const FSEntry &, const MetadataKeyType);
                ~EMTimeKey();

                Timestamp value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ESlotKey :
            public MetadataValueKey<SlotName>,
            private PrivateImplementationPattern<ESlotKey>
        {
            public:
                ESlotKey(
                        const std::shared_ptr<const EAPIMetadataVariable> &,
                        const std::string &, const MetadataKeyType);
                ~ESlotKey();

                virtual const SlotName value() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
