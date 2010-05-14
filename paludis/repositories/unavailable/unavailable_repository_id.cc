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

#include <paludis/repositories/unavailable/unavailable_repository_id.hh>
#include <paludis/repositories/unavailable/unavailable_repository.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/unchoices_key.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/output_manager.hh>

using namespace paludis;
using namespace paludis::unavailable_repository;

namespace paludis
{
    template <>
    struct Implementation<UnavailableRepositoryID>
    {
        const Environment * const env;
        const QualifiedPackageName name;
        const VersionSpec version;
        const UnavailableRepository * const repo;

        const std::tr1::shared_ptr<const MetadataValueKey<std::string> > description_key;
        const std::tr1::shared_ptr<const MetadataValueKey<std::string> > homepage_key;
        const std::tr1::shared_ptr<const MetadataValueKey<std::string> > sync_key;
        const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;
        const std::tr1::shared_ptr<const Mask> mask;

        Implementation(
                const UnavailableRepositoryIDParams & e) :
            env(e.environment()),
            name(e.name()),
            version("0", VersionSpecOptions()),
            repo(e.repository()),
            description_key(e.description()),
            homepage_key(e.homepage()),
            sync_key(e.sync()),
            format_key(e.format()),
            mask(e.mask())
        {
        }
    };
}

UnavailableRepositoryID::UnavailableRepositoryID(const UnavailableRepositoryIDParams & entry) :
    PrivateImplementationPattern<UnavailableRepositoryID>(new Implementation<UnavailableRepositoryID>(entry)),
    _imp(PrivateImplementationPattern<UnavailableRepositoryID>::_imp)
{
    if (_imp->description_key)
        add_metadata_key(_imp->description_key);
    if (_imp->homepage_key)
        add_metadata_key(_imp->homepage_key);
    if (_imp->sync_key)
        add_metadata_key(_imp->sync_key);
    if (_imp->format_key)
        add_metadata_key(_imp->format_key);
    if (_imp->mask)
        add_mask(_imp->mask);
}

UnavailableRepositoryID::~UnavailableRepositoryID()
{
}

void
UnavailableRepositoryID::need_keys_added() const
{
}

void
UnavailableRepositoryID::need_masks_added() const
{
}

const std::string
UnavailableRepositoryID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) +
                "::" + stringify(_imp->repo->name());

        case idcf_no_version:
            return stringify(_imp->name) + "::" + stringify(_imp->repo->name());

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_name:
            return stringify(_imp->version) + "::" + stringify(_imp->repo->name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
UnavailableRepositoryID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version())
            + "::" + stringify(repository()->name()),
            _imp->env, UserPackageDepSpecOptions());
}

const QualifiedPackageName
UnavailableRepositoryID::name() const
{
    return _imp->name;
}

const VersionSpec
UnavailableRepositoryID::version() const
{
    return _imp->version;
}

const std::tr1::shared_ptr<const Repository>
UnavailableRepositoryID::repository() const
{
    return _imp->repo->shared_from_this();
}

bool
UnavailableRepositoryID::supports_action(const SupportsActionTestBase & a) const
{
    return simple_visitor_cast<const SupportsActionTest<InstallAction> >(a);
}

namespace
{
    void used_this_for_config_protect(std::string & s, const std::string & v)
    {
        s = v;
    }

    std::tr1::shared_ptr<OutputManager> this_output_manager(
            const std::tr1::shared_ptr<OutputManager> & o, const Action &)
    {
        return o;
    }

    bool ignore_nothing(const FSEntry &)
    {
        return false;
    }
}

void
UnavailableRepositoryID::perform_action(Action & action) const
{
    Timestamp build_start_time(Timestamp::now());

    const InstallAction * const install_action(simple_visitor_cast<const InstallAction>(action));
    if (! install_action)
        throw ActionFailedError("Unsupported action: " + action.simple_name());

    if (! (*install_action->options.destination()).destination_interface())
        throw ActionFailedError("Can't install '" + stringify(*this)
                + "' to destination '" + stringify(install_action->options.destination()->name())
                + "' because destination does not provide destination_interface");

    std::tr1::shared_ptr<OutputManager> output_manager(install_action->options.make_output_manager()(*install_action));
    std::string used_config_protect;

    switch (install_action->options.want_phase()("merge"))
    {
        case wp_yes:
            {
                (*install_action->options.destination()).destination_interface()->merge(
                        make_named_values<MergeParams>(
                            value_for<n::build_start_time>(build_start_time),
                            value_for<n::environment_file>(FSEntry("/dev/null")),
                            value_for<n::image_dir>(FSEntry("/dev/null")),
                            value_for<n::merged_entries>(make_shared_ptr(new FSEntrySet)),
                            value_for<n::options>(MergerOptions()),
                            value_for<n::output_manager>(output_manager),
                            value_for<n::package_id>(shared_from_this()),
                            value_for<n::perform_uninstall>(install_action->options.perform_uninstall()),
                            value_for<n::used_this_for_config_protect>(std::tr1::bind(
                                    &used_this_for_config_protect, std::tr1::ref(used_config_protect), std::tr1::placeholders::_1))
                            ));
            }
            break;

        case wp_skip:
            break;

        case wp_abort:
            throw ActionAbortedError("Told to abort install");

        case last_wp:
            throw InternalError(PALUDIS_HERE, "bad WantPhase");
    }

    for (PackageIDSequence::ConstIterator i(install_action->options.replacing()->begin()),
            i_end(install_action->options.replacing()->end()) ;
            i != i_end ; ++i)
    {
        Context local_context("When cleaning '" + stringify(**i) + "':");

        UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                    value_for<n::config_protect>(used_config_protect),
                    value_for<n::if_for_install_id>(shared_from_this()),
                    value_for<n::ignore_for_unmerge>(&ignore_nothing),
                    value_for<n::is_overwrite>(false),
                    value_for<n::make_output_manager>(std::tr1::bind(
                            &this_output_manager, output_manager, std::tr1::placeholders::_1))
                    ));
        install_action->options.perform_uninstall()(*i, uo);
    }

    output_manager->succeeded();
}

std::tr1::shared_ptr<const Set<std::string> >
UnavailableRepositoryID::breaks_portage() const
{
    return make_shared_ptr(new Set<std::string>);
}

bool
UnavailableRepositoryID::arbitrary_less_than_comparison(const PackageID &) const
{
    return false;
}

std::size_t
UnavailableRepositoryID::extra_hash_value() const
{
    return 0;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
UnavailableRepositoryID::contains_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
UnavailableRepositoryID::contained_in_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
UnavailableRepositoryID::fs_location_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<bool> >
UnavailableRepositoryID::transient_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<bool> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
UnavailableRepositoryID::virtual_for_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
UnavailableRepositoryID::keywords_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
UnavailableRepositoryID::provide_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnavailableRepositoryID::dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnavailableRepositoryID::build_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnavailableRepositoryID::run_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnavailableRepositoryID::post_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnavailableRepositoryID::suggested_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnavailableRepositoryID::short_description_key() const
{
    return _imp->description_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnavailableRepositoryID::long_description_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
UnavailableRepositoryID::fetches_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
UnavailableRepositoryID::homepage_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
UnavailableRepositoryID::contents_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >();
}

const std::tr1::shared_ptr<const MetadataTimeKey>
UnavailableRepositoryID::installed_time_key() const
{
    return std::tr1::shared_ptr<const MetadataTimeKey>();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
UnavailableRepositoryID::from_repositories_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
UnavailableRepositoryID::choices_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<SlotName> >
UnavailableRepositoryID::slot_key() const
{
    return make_null_shared_ptr();
}

template class PrivateImplementationPattern<UnavailableRepositoryID>;

