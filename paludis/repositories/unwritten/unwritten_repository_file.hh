/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNWRITTEN_UNWRITTEN_REPOSITORY_FILE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNWRITTEN_UNWRITTEN_REPOSITORY_FILE_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/named_value.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/spec_tree-fwd.hh>

namespace paludis
{
    namespace n
    {
        struct added_by;
        struct bug_ids;
        struct comment;
        struct description;
        struct homepage;
        struct name;
        struct remote_ids;
        struct slot;
        struct version;
    }

    namespace unwritten_repository
    {
        class UnwrittenRepositoryFile;

        struct UnwrittenRepositoryFileEntry
        {
            NamedValue<n::added_by, std::tr1::shared_ptr<const MetadataValueKey<std::string> > > added_by;
            NamedValue<n::bug_ids, std::tr1::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > > > bug_ids;
            NamedValue<n::comment, std::tr1::shared_ptr<const MetadataValueKey<std::string> > > comment;
            NamedValue<n::description, std::tr1::shared_ptr<const MetadataValueKey<std::string> > > description;
            NamedValue<n::homepage, std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > > homepage;
            NamedValue<n::name, QualifiedPackageName> name;
            NamedValue<n::remote_ids, std::tr1::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > > > remote_ids;
            NamedValue<n::slot, std::tr1::shared_ptr<const MetadataValueKey<SlotName> > > slot;
            NamedValue<n::version, VersionSpec> version;
        };

        class PALUDIS_VISIBLE UnwrittenRepositoryFile :
            private PrivateImplementationPattern<UnwrittenRepositoryFile>
        {
            private:
                void _load(const FSEntry &);

            public:
                UnwrittenRepositoryFile(const FSEntry &);
                ~UnwrittenRepositoryFile();

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const UnwrittenRepositoryFileEntry> ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<unwritten_repository::UnwrittenRepositoryFile>;
#endif
}

#endif
