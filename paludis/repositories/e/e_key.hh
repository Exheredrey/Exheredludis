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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_E_KEY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_E_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map-fwd.hh>

namespace paludis
{
    class ERepository;

    namespace erepository
    {
        class ERepositoryID;

        class EMutableRepositoryMaskInfoKey :
            public MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> >
        {
            private:
                std::tr1::shared_ptr<const RepositoryMaskInfo> _value;

            public:
                EMutableRepositoryMaskInfoKey(const std::tr1::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::tr1::shared_ptr<const RepositoryMaskInfo> &, const MetadataKeyType);
                ~EMutableRepositoryMaskInfoKey();

                virtual const std::tr1::shared_ptr<const RepositoryMaskInfo> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                void set_value(const std::tr1::shared_ptr<const RepositoryMaskInfo> &);
        };

        class EDependenciesKey :
            public MetadataSpecTreeKey<DependencySpecTree>,
            private PrivateImplementationPattern<EDependenciesKey>
        {
            private:
                PrivateImplementationPattern<EDependenciesKey>::ImpPtr & _imp;

            public:
                EDependenciesKey(
                        const Environment * const,
                        const std::tr1::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &,
                        const std::tr1::shared_ptr<const DependencyLabelSequence> &,
                        const MetadataKeyType);
                ~EDependenciesKey();

                virtual const std::tr1::shared_ptr<const DependencySpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const DependencySpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const DependencySpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::tr1::shared_ptr<const DependencyLabelSequence> initial_labels() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EFetchableURIKey :
            public MetadataSpecTreeKey<FetchableURISpecTree>,
            private PrivateImplementationPattern<EFetchableURIKey>
        {
            private:
                PrivateImplementationPattern<EFetchableURIKey>::ImpPtr & _imp;

            public:
                EFetchableURIKey(const Environment * const,
                        const std::tr1::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EFetchableURIKey();

                virtual const std::tr1::shared_ptr<const FetchableURISpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const FetchableURISpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const FetchableURISpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::tr1::shared_ptr<const URILabel> initial_label() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ESimpleURIKey :
            public MetadataSpecTreeKey<SimpleURISpecTree>,
            private PrivateImplementationPattern<ESimpleURIKey>
        {
            private:
                PrivateImplementationPattern<ESimpleURIKey>::ImpPtr & _imp;

            public:
                ESimpleURIKey(const Environment * const,
                        const std::tr1::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~ESimpleURIKey();

                virtual const std::tr1::shared_ptr<const SimpleURISpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const SimpleURISpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const SimpleURISpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EPlainTextSpecKey :
            public MetadataSpecTreeKey<PlainTextSpecTree>,
            private PrivateImplementationPattern<EPlainTextSpecKey>
        {
            private:
                PrivateImplementationPattern<EPlainTextSpecKey>::ImpPtr & _imp;

            public:
                EPlainTextSpecKey(const Environment * const,
                        const std::tr1::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EPlainTextSpecKey();

                virtual const std::tr1::shared_ptr<const PlainTextSpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const PlainTextSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const PlainTextSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EMyOptionsKey :
            public MetadataSpecTreeKey<PlainTextSpecTree>,
            private PrivateImplementationPattern<EMyOptionsKey>
        {
            private:
                PrivateImplementationPattern<EMyOptionsKey>::ImpPtr & _imp;

            public:
                EMyOptionsKey(const Environment * const,
                        const std::tr1::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EMyOptionsKey();

                virtual const std::tr1::shared_ptr<const PlainTextSpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const PlainTextSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const PlainTextSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EProvideKey :
            public MetadataSpecTreeKey<ProvideSpecTree>,
            private PrivateImplementationPattern<EProvideKey>
        {
            private:
                PrivateImplementationPattern<EProvideKey>::ImpPtr & _imp;

            public:
                EProvideKey(const Environment * const,
                        const std::tr1::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EProvideKey();

                virtual const std::tr1::shared_ptr<const ProvideSpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const ProvideSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const ProvideSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ELicenseKey :
            public MetadataSpecTreeKey<LicenseSpecTree>,
            private PrivateImplementationPattern<ELicenseKey>
        {
            private:
                PrivateImplementationPattern<ELicenseKey>::ImpPtr & _imp;

            public:
                ELicenseKey(
                        const Environment * const,
                        const std::tr1::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~ELicenseKey();

                virtual const std::tr1::shared_ptr<const LicenseSpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const LicenseSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const LicenseSpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EKeywordsKey :
            public MetadataCollectionKey<KeywordNameSet>,
            private PrivateImplementationPattern<EKeywordsKey>
        {
            private:
                PrivateImplementationPattern<EKeywordsKey>::ImpPtr & _imp;

            public:
                EKeywordsKey(
                        const Environment * const,
                        const std::tr1::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EKeywordsKey();

                const std::tr1::shared_ptr<const KeywordNameSet> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const Formatter<KeywordName> &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EStringSetKey :
            public MetadataCollectionKey<Set<std::string> >,
            private PrivateImplementationPattern<EStringSetKey>
        {
            private:
                PrivateImplementationPattern<EStringSetKey>::ImpPtr & _imp;

            public:
                EStringSetKey(const std::tr1::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EStringSetKey();

                const std::tr1::shared_ptr<const Set<std::string> > value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const Formatter<std::string> &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EContentsKey :
            public MetadataValueKey<std::tr1::shared_ptr<const Contents> >,
            private PrivateImplementationPattern<EContentsKey>
        {
            private:
                PrivateImplementationPattern<EContentsKey>::ImpPtr & _imp;

            public:
                EContentsKey(
                        const std::tr1::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const FSEntry &, const MetadataKeyType);
                ~EContentsKey();

                const std::tr1::shared_ptr<const Contents> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EMTimeKey :
            public MetadataTimeKey,
            private PrivateImplementationPattern<EMTimeKey>
        {
            private:
                PrivateImplementationPattern<EMTimeKey>::ImpPtr & _imp;

            public:
                EMTimeKey(const std::tr1::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const FSEntry &, const MetadataKeyType);
                ~EMTimeKey();

                time_t value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
