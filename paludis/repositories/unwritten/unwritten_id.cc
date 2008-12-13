/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/repositories/unwritten/unwritten_id.hh>
#include <paludis/repositories/unwritten/unwritten_repository.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/hashes.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/unchoices_key.hh>

using namespace paludis;
using namespace paludis::unwritten_repository;

namespace paludis
{
    template <>
    struct Implementation<UnwrittenID>
    {
        const QualifiedPackageName name;
        const VersionSpec version;
        const SlotName slot;
        const UnwrittenRepository * const repo;

        const std::tr1::shared_ptr<const MetadataValueKey<std::string> > description_key;
        const std::tr1::shared_ptr<const MetadataValueKey<std::string> > added_by_key;
        const std::tr1::shared_ptr<const MetadataValueKey<std::string> > comment_key;
        const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage_key;
        const std::tr1::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > > bug_ids_key;
        const std::tr1::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > > remote_ids_key;
        const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > > choices_key;
        const std::tr1::shared_ptr<const Mask> mask;

        Implementation(
                const UnwrittenIDParams & e) :
            name(e.name()),
            version(e.version()),
            slot(e.slot()),
            repo(e.repository()),
            description_key(e.description()),
            added_by_key(e.added_by()),
            comment_key(e.comment()),
            homepage_key(e.homepage()),
            bug_ids_key(e.bug_ids()),
            remote_ids_key(e.remote_ids()),
            choices_key(unchoices_key()),
            mask(e.mask())
        {
        }
    };
}

UnwrittenID::UnwrittenID(const UnwrittenIDParams & entry) :
    PrivateImplementationPattern<UnwrittenID>(new Implementation<UnwrittenID>(entry)),
    _imp(PrivateImplementationPattern<UnwrittenID>::_imp)
{
    if (_imp->description_key)
        add_metadata_key(_imp->description_key);
    if (_imp->homepage_key)
        add_metadata_key(_imp->homepage_key);
    if (_imp->added_by_key)
        add_metadata_key(_imp->added_by_key);
    if (_imp->comment_key)
        add_metadata_key(_imp->comment_key);
    if (_imp->bug_ids_key)
        add_metadata_key(_imp->bug_ids_key);
    if (_imp->remote_ids_key)
        add_metadata_key(_imp->remote_ids_key);
    add_mask(_imp->mask);
}

UnwrittenID::~UnwrittenID()
{
}

void
UnwrittenID::need_keys_added() const
{
}

void
UnwrittenID::need_masks_added() const
{
}

const std::string
UnwrittenID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) +
                ":" + stringify(_imp->slot) + "::" + stringify(_imp->repo->name());

        case idcf_no_version:
            return stringify(_imp->name) + ":" + stringify(_imp->slot) +
                "::" + stringify(_imp->repo->name());

        case idcf_version:
            return stringify(_imp->version);

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

const QualifiedPackageName
UnwrittenID::name() const
{
    return _imp->name;
}

const VersionSpec
UnwrittenID::version() const
{
    return _imp->version;
}

const SlotName
UnwrittenID::slot() const
{
    return _imp->slot;
}

const std::tr1::shared_ptr<const Repository>
UnwrittenID::repository() const
{
    return _imp->repo->shared_from_this();
}

bool
UnwrittenID::supports_action(const SupportsActionTestBase & a) const
{
    return simple_visitor_cast<const SupportsActionTest<InstallAction> >(a);
}

void
UnwrittenID::perform_action(Action & a) const
{
    throw UnsupportedActionError(*this, a);
}

std::tr1::shared_ptr<const Set<std::string> >
UnwrittenID::breaks_portage() const
{
    return make_shared_ptr(new Set<std::string>);
}

bool
UnwrittenID::arbitrary_less_than_comparison(const PackageID & other) const
{
    if (slot() < other.slot())
        return true;

    return false;
}

std::size_t
UnwrittenID::extra_hash_value() const
{
    return Hash<SlotName>()(slot());
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
UnwrittenID::contains_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
UnwrittenID::contained_in_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
UnwrittenID::fs_location_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<bool> >
UnwrittenID::transient_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<bool> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
UnwrittenID::virtual_for_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
UnwrittenID::keywords_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
UnwrittenID::provide_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnwrittenID::build_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnwrittenID::run_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnwrittenID::post_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnwrittenID::suggested_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnwrittenID::short_description_key() const
{
    return _imp->description_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnwrittenID::long_description_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
UnwrittenID::fetches_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
UnwrittenID::homepage_key() const
{
    return _imp->homepage_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
UnwrittenID::contents_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >();
}

const std::tr1::shared_ptr<const MetadataTimeKey>
UnwrittenID::installed_time_key() const
{
    return std::tr1::shared_ptr<const MetadataTimeKey>();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
UnwrittenID::from_repositories_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
UnwrittenID::choices_key() const
{
    return _imp->choices_key;
}

template class PrivateImplementationPattern<UnwrittenID>;

