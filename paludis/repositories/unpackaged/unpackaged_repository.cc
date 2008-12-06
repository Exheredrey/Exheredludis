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

#include <paludis/repositories/unpackaged/unpackaged_repository.hh>
#include <paludis/repositories/unpackaged/unpackaged_id.hh>
#include <paludis/repositories/unpackaged/exceptions.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/destringify.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace paludis
{
    template <>
    struct Implementation<UnpackagedRepository>
    {
        const UnpackagedRepositoryParams params;
        std::tr1::shared_ptr<const PackageID> id;
        std::tr1::shared_ptr<PackageIDSequence> ids;
        std::tr1::shared_ptr<QualifiedPackageNameSet> package_names;
        std::tr1::shared_ptr<CategoryNamePartSet> category_names;

        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > install_under_key;
        std::tr1::shared_ptr<const MetadataValueKey<long> > rewrite_ids_over_to_root_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > name_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > slot_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > build_dependencies_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > run_dependencies_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > description_key;

        Implementation(const RepositoryName & n,
                const UnpackagedRepositoryParams & p) :
            params(p),
            id(new UnpackagedID(params.environment(), params.name(), params.version(), params.slot(), n, params.location(),
                        params.build_dependencies(), params.run_dependencies(), params.description())),
            ids(new PackageIDSequence),
            package_names(new QualifiedPackageNameSet),
            category_names(new CategoryNamePartSet),
            location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                        mkt_significant, params.location())),
            install_under_key(new LiteralMetadataValueKey<FSEntry> ("install_under", "install_under",
                        mkt_significant, params.install_under())),
            rewrite_ids_over_to_root_key(new LiteralMetadataValueKey<long> ("rewrite_ids_over_to_root", "rewrite_ids_over_to_root",
                        mkt_normal, params.rewrite_ids_over_to_root())),
            name_key(new LiteralMetadataValueKey<std::string> ("name", "name",
                        mkt_normal, stringify(params.name()))),
            slot_key(new LiteralMetadataValueKey<std::string> ("slot", "slot",
                        mkt_normal, stringify(params.slot()))),
            format_key(new LiteralMetadataValueKey<std::string> (
                        "format", "format", mkt_significant, "unpackaged")),
            build_dependencies_key(new LiteralMetadataValueKey<std::string> (
                        "build_dependencies", "build_dependencies", mkt_normal, params.build_dependencies())),
            run_dependencies_key(new LiteralMetadataValueKey<std::string> (
                        "run_dependencies", "run_dependencies", mkt_normal, params.run_dependencies())),
            description_key(new LiteralMetadataValueKey<std::string> (
                        "description", "description", mkt_normal, params.description()))
        {
            ids->push_back(id);
            package_names->insert(id->name());
            category_names->insert(id->name().category());
        }
    };
}

UnpackagedRepository::UnpackagedRepository(const RepositoryName & n,
        const UnpackagedRepositoryParams & params) :
    PrivateImplementationPattern<UnpackagedRepository>(new Implementation<UnpackagedRepository>(n, params)),
    Repository(params.environment(), n, make_named_values<RepositoryCapabilities>(
                value_for<n::destination_interface>(static_cast<RepositoryDestinationInterface *>(0)),
                value_for<n::e_interface>(static_cast<RepositoryEInterface *>(0)),
                value_for<n::environment_variable_interface>(static_cast<RepositoryEnvironmentVariableInterface *>(0)),
                value_for<n::hook_interface>(static_cast<RepositoryHookInterface *>(0)),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(static_cast<RepositoryManifestInterface *>(0)),
                value_for<n::mirrors_interface>(static_cast<RepositoryMirrorsInterface *>(0)),
                value_for<n::provides_interface>(static_cast<RepositoryProvidesInterface *>(0)),
                value_for<n::qa_interface>(static_cast<RepositoryQAInterface *>(0)),
                value_for<n::sets_interface>(static_cast<RepositorySetsInterface *>(0)),
                value_for<n::syncable_interface>(static_cast<RepositorySyncableInterface *>(0)),
                value_for<n::virtuals_interface>(static_cast<RepositoryVirtualsInterface *>(0))
            )),
    _imp(PrivateImplementationPattern<UnpackagedRepository>::_imp)
{
    _add_metadata_keys();
}

UnpackagedRepository::~UnpackagedRepository()
{
}

void
UnpackagedRepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->install_under_key);
    add_metadata_key(_imp->rewrite_ids_over_to_root_key);
    add_metadata_key(_imp->name_key);
    add_metadata_key(_imp->slot_key);
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->build_dependencies_key);
    add_metadata_key(_imp->run_dependencies_key);
    add_metadata_key(_imp->description_key);
}

std::tr1::shared_ptr<const PackageIDSequence>
UnpackagedRepository::package_ids(const QualifiedPackageName & n) const
{
    return n == _imp->id->name() ? _imp->ids : make_shared_ptr(new PackageIDSequence);
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
UnpackagedRepository::package_names(const CategoryNamePart & c) const
{
    return c == _imp->id->name().category() ? _imp->package_names : make_shared_ptr(new QualifiedPackageNameSet);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
UnpackagedRepository::category_names() const
{
    return _imp->category_names;
}

std::tr1::shared_ptr<const CategoryNamePartSet>
UnpackagedRepository::category_names_containing_package(const PackageNamePart & p) const
{
    return p == _imp->id->name().package() ? _imp->category_names : make_shared_ptr(new CategoryNamePartSet);
}

bool
UnpackagedRepository::has_package_named(const QualifiedPackageName & q) const
{
    return q == _imp->id->name();
}

bool
UnpackagedRepository::has_category_named(const CategoryNamePart & c) const
{
    return c == _imp->id->name().category();
}

bool
UnpackagedRepository::some_ids_might_support_action(const SupportsActionTestBase & test) const
{
    return _imp->id->supports_action(test);
}

void
UnpackagedRepository::invalidate()
{
    _imp.reset(new Implementation<UnpackagedRepository>(name(), _imp->params));
    _add_metadata_keys();
}

void
UnpackagedRepository::invalidate_masks()
{
}

void
UnpackagedRepository::need_keys_added() const
{
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnpackagedRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
UnpackagedRepository::location_key() const
{
    return _imp->location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
UnpackagedRepository::installed_root_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

std::tr1::shared_ptr<Repository>
UnpackagedRepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    Context context("When creating UnpackagedRepository:");

    std::string location(f("location"));
    if (location.empty())
        throw unpackaged_repositories::RepositoryConfigurationError("Key 'location' not specified or empty");

    std::string install_under(f("install_under"));
    if (install_under.empty())
        install_under = "/";

    std::string name(f("name"));
    if (name.empty())
        throw unpackaged_repositories::RepositoryConfigurationError("Key 'name' not specified or empty");

    std::string version(f("version"));
    if (version.empty())
        throw unpackaged_repositories::RepositoryConfigurationError("Key 'version' not specified or empty");

    std::string slot(f("slot"));
    if (slot.empty())
        throw unpackaged_repositories::RepositoryConfigurationError("Key 'slot' not specified or empty");

    std::string build_dependencies(f("build_dependencies"));
    std::string run_dependencies(f("run_dependencies"));
    std::string description(f("description"));

    int rewrite_ids_over_to_root(-1);
    if (! f("rewrite_ids_over_to_root").empty())
    {
        Context item_context("When handling rewrite_ids_over_to_root key:");
        rewrite_ids_over_to_root = destringify<int>(f("rewrite_ids_over_to_root"));
    }

    return make_shared_ptr(new UnpackagedRepository(RepositoryName("unpackaged"),
                make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                    value_for<n::build_dependencies>(build_dependencies),
                    value_for<n::description>(description),
                    value_for<n::environment>(env),
                    value_for<n::install_under>(install_under),
                    value_for<n::location>(location),
                    value_for<n::name>(QualifiedPackageName(name)),
                    value_for<n::rewrite_ids_over_to_root>(rewrite_ids_over_to_root),
                    value_for<n::run_dependencies>(run_dependencies),
                    value_for<n::slot>(SlotName(slot)),
                    value_for<n::version>(VersionSpec(version))
                )));
}

RepositoryName
UnpackagedRepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    return RepositoryName(f("name"));
}

std::tr1::shared_ptr<const RepositoryNameSet>
UnpackagedRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

