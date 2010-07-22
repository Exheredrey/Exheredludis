/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>

#include <map>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<FakeRepository>
    {
        std::shared_ptr<FakeRepository::VirtualsSequence> virtual_packages;
        std::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Implementation() :
            virtual_packages(new FakeRepository::VirtualsSequence),
            format_key(new LiteralMetadataValueKey<std::string> (
                        "format", "format", mkt_significant, "fake"))
        {
        }
    };
}

FakeRepository::FakeRepository(const FakeRepositoryParams & params) :
    PrivateImplementationPattern<FakeRepository>(new Implementation<FakeRepository>),
    FakeRepositoryBase(params.environment(), params.name(), make_named_values<RepositoryCapabilities>(
                n::destination_interface() = static_cast<RepositoryDestinationInterface *>(0),
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(0),
                n::make_virtuals_interface() = static_cast<RepositoryMakeVirtualsInterface *>(0),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(0),
                n::provides_interface() = static_cast<RepositoryProvidesInterface *>(0),
                n::virtuals_interface() = (*DistributionData::get_instance()->distribution_from_string(
                            params.environment()->distribution())).support_old_style_virtuals() ? this : 0
                )),
            _imp(PrivateImplementationPattern<FakeRepository>::_imp)
{
    add_metadata_key(_imp->format_key);
}

FakeRepository::~FakeRepository()
{
}

std::shared_ptr<const FakeRepository::VirtualsSequence>
FakeRepository::virtual_packages() const
{
    return _imp->virtual_packages;
}

void
FakeRepository::add_virtual_package(const QualifiedPackageName & q, const std::shared_ptr<const PackageDepSpec> & p)
{
    _imp->virtual_packages->push_back(make_named_values<RepositoryVirtualsEntry>(
                n::provided_by_spec() = p,
                n::virtual_name() = q
                ));
}

namespace paludis
{
    class RepositoryFactory;
}

namespace
{
    struct SupportsActionQuery
    {
        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<ConfigAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<FetchAction> &) const
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
FakeRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

bool
FakeRepository::some_ids_might_not_be_masked() const
{
    return true;
}

const bool
FakeRepository::is_unimportant() const
{
    return false;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
FakeRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSEntry> >
FakeRepository::location_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::shared_ptr<const MetadataValueKey<FSEntry> >
FakeRepository::installed_root_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
FakeRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
FakeRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

