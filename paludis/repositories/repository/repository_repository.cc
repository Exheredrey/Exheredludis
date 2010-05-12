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

#include <paludis/repositories/repository/repository_repository.hh>
#include <paludis/repositories/repository/repository_repository_store.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/active_object_ptr.hh>
#include <paludis/util/deferred_construction_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/syncer.hh>
#include <paludis/hook.hh>
#include <list>

using namespace paludis;
using namespace paludis::repository_repository;

namespace
{
    std::tr1::shared_ptr<RepositoryRepositoryStore>
    make_store(const RepositoryRepository * const repo, const RepositoryRepositoryParams & p)
    {
        return make_shared_ptr(new RepositoryRepositoryStore(p.environment(), repo));
    }
}

namespace paludis
{
    template <>
    struct Implementation<RepositoryRepository>
    {
        const RepositoryRepositoryParams params;

        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > config_filename_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > config_template_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<FSEntry> > installed_root_key;

        const ActiveObjectPtr<DeferredConstructionPtr<
            std::tr1::shared_ptr<RepositoryRepositoryStore> > > store;

        Implementation(const RepositoryRepository * const repo, const RepositoryRepositoryParams & p) :
            params(p),
            format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                        mkt_significant, "repository")),
            config_filename_key(new LiteralMetadataValueKey<std::string> (
                        "config_filename", "config_filename", mkt_normal, params.config_filename())),
            config_template_key(new LiteralMetadataValueKey<std::string> (
                        "config_template", "config_template", mkt_normal, params.config_template())),
            installed_root_key(new LiteralMetadataValueKey<FSEntry>("root", "root", mkt_normal, p.root())),
            store(DeferredConstructionPtr<std::tr1::shared_ptr<RepositoryRepositoryStore> > (
                        std::tr1::bind(&make_store, repo, std::tr1::cref(params))))
        {
        }
    };
}

RepositoryRepositoryConfigurationError::RepositoryRepositoryConfigurationError(const std::string & s) throw () :
    ConfigurationError("RepositoryRepository configuration error: " + s)
{
}

RepositoryRepository::RepositoryRepository(const RepositoryRepositoryParams & p) :
    PrivateImplementationPattern<RepositoryRepository>(new Implementation<RepositoryRepository>(this, p)),
    Repository(
            p.environment(),
            p.name(),
            make_named_values<RepositoryCapabilities>(
                value_for<n::destination_interface>(static_cast<RepositoryDestinationInterface *>(0)),
                value_for<n::environment_variable_interface>(static_cast<RepositoryEnvironmentVariableInterface *>(0)),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(static_cast<RepositoryManifestInterface *>(0)),
                value_for<n::provides_interface>(static_cast<RepositoryProvidesInterface *>(0)),
                value_for<n::virtuals_interface>(static_cast<RepositoryVirtualsInterface *>(0))
                )),
    _imp(PrivateImplementationPattern<RepositoryRepository>::_imp)
{
    _add_metadata_keys();
}

RepositoryRepository::~RepositoryRepository()
{
}

bool
RepositoryRepository::can_be_favourite_repository() const
{
    return false;
}

const bool
RepositoryRepository::is_unimportant() const
{
    return true;
}

void
RepositoryRepository::_add_metadata_keys()
{
    clear_metadata_keys();
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->config_filename_key);
    add_metadata_key(_imp->config_template_key);
    add_metadata_key(_imp->installed_root_key);
}

void
RepositoryRepository::need_keys_added() const
{
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
RepositoryRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
RepositoryRepository::location_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
RepositoryRepository::installed_root_key() const
{
    return _imp->installed_root_key;
}

void
RepositoryRepository::invalidate()
{
    _imp.reset(new Implementation<RepositoryRepository>(this, _imp->params));
    _add_metadata_keys();
}

void
RepositoryRepository::invalidate_masks()
{
}

bool
RepositoryRepository::has_category_named(const CategoryNamePart & c) const
{
    return _imp->store->has_category_named(c);
}

bool
RepositoryRepository::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->store->has_package_named(q);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
RepositoryRepository::category_names() const
{
    return _imp->store->category_names();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
RepositoryRepository::unimportant_category_names() const
{
    return _imp->store->unimportant_category_names();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
RepositoryRepository::category_names_containing_package(const PackageNamePart & p) const
{
    return Repository::category_names_containing_package(p);
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
RepositoryRepository::package_names(const CategoryNamePart & c) const
{
    return _imp->store->package_names(c);
}

std::tr1::shared_ptr<const PackageIDSequence>
RepositoryRepository::package_ids(const QualifiedPackageName & p) const
{
    return _imp->store->package_ids(p);
}

namespace
{
    struct SupportsActionQuery
    {
        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<ConfigAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return false;
        }
    };
}

bool
RepositoryRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

bool
RepositoryRepository::sync(const std::tr1::shared_ptr<OutputManager> &) const
{
    return false;
}

std::tr1::shared_ptr<Repository>
RepositoryRepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    Context context("When making repository repository from repo_file '" + f("repo_file") + "':");

    std::string name_str(f("name"));
    if (name_str.empty())
        name_str = "repository";

    std::string config_filename(f("config_filename"));
    if (config_filename.empty())
        throw RepositoryRepositoryConfigurationError("Key 'config_filename' not specified or empty");

    std::string config_template(f("config_template"));
    if (config_template.empty())
        throw RepositoryRepositoryConfigurationError("Key 'config_template' not specified or empty");

    std::string root_str(f("root"));
    if (root_str.empty())
        root_str = "/";

    return std::tr1::shared_ptr<RepositoryRepository>(new RepositoryRepository(
                make_named_values<RepositoryRepositoryParams>(
                    value_for<n::config_filename>(config_filename),
                    value_for<n::config_template>(config_template),
                    value_for<n::environment>(env),
                    value_for<n::name>(RepositoryName(name_str)),
                    value_for<n::root>(root_str)
                )));
}

RepositoryName
RepositoryRepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    if (f("name").empty())
        return RepositoryName("repository");
    else
        return RepositoryName(f("name"));
}

std::tr1::shared_ptr<const RepositoryNameSet>
RepositoryRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

void
RepositoryRepository::populate_sets() const
{
}

HookResult
RepositoryRepository::perform_hook(const Hook &)
{
    return make_named_values<HookResult>(value_for<n::max_exit_status>(0), value_for<n::output>(""));
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
RepositoryRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
RepositoryRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

template class PrivateImplementationPattern<repository_repository::RepositoryRepository>;

