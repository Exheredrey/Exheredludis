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

#include <paludis/repositories/unpackaged/unpackaged_id.hh>
#include <paludis/repositories/unpackaged/unpackaged_key.hh>
#include <paludis/repositories/unpackaged/unpackaged_stripper.hh>
#include <paludis/repositories/unpackaged/dep_parser.hh>
#include <paludis/repositories/unpackaged/dep_printer.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/log.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/elike_choices.hh>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace paludis
{
    template <>
    struct Implementation<UnpackagedID>
    {
        const Environment * const env;
        const QualifiedPackageName name;
        const VersionSpec version;
        const SlotName slot;
        const RepositoryName repository_name;

        std::tr1::shared_ptr<DependencyLabelSequence> build_dependencies_labels;
        std::tr1::shared_ptr<DependencyLabelSequence> run_dependencies_labels;

        const std::tr1::shared_ptr<LiteralMetadataValueKey<FSEntry> > fs_location_key;
        const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key;
        const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key;
        const std::tr1::shared_ptr<const MetadataValueKey<std::string> > description_key;
        const std::tr1::shared_ptr<const UnpackagedChoicesKey> choices_key;

        Implementation(const Environment * const e,
                const QualifiedPackageName & q,
                const VersionSpec & v,
                const SlotName & s,
                const RepositoryName & n,
                const FSEntry & l,
                const std::string & b,
                const std::string & r,
                const std::string & d,
                const UnpackagedID * const id) :
            env(e),
            name(q),
            version(v),
            slot(s),
            repository_name(n),
            build_dependencies_labels(new DependencyLabelSequence),
            run_dependencies_labels(new DependencyLabelSequence),
            fs_location_key(new LiteralMetadataValueKey<FSEntry> ("location", "Location", mkt_normal, l)),
            build_dependencies_key(new UnpackagedDependencyKey(env, "build_dependencies", "Build dependencies", mkt_dependencies,
                        build_dependencies_labels, b)),
            run_dependencies_key(new UnpackagedDependencyKey(env, "run_dependencies", "Run dependencies", mkt_dependencies,
                        run_dependencies_labels, r)),
            description_key(new LiteralMetadataValueKey<std::string> ("description", "Description", mkt_significant, d)),
            choices_key(new UnpackagedChoicesKey(env, "choices", "Choices", mkt_normal, id))
        {
            build_dependencies_labels->push_back(make_shared_ptr(new DependencyBuildLabel("build_dependencies")));
            run_dependencies_labels->push_back(make_shared_ptr(new DependencyRunLabel("run_dependencies")));
        }
    };
}

UnpackagedID::UnpackagedID(const Environment * const e, const QualifiedPackageName & q,
        const VersionSpec & v, const SlotName & s, const RepositoryName & n, const FSEntry & l,
        const std::string & b, const std::string & r, const std::string & d) :
    PrivateImplementationPattern<UnpackagedID>(new Implementation<UnpackagedID>(e, q, v, s, n, l, b, r, d, this)),
    _imp(PrivateImplementationPattern<UnpackagedID>::_imp)
{
    add_metadata_key(_imp->fs_location_key);
    add_metadata_key(_imp->build_dependencies_key);
    add_metadata_key(_imp->run_dependencies_key);
    add_metadata_key(_imp->description_key);
    add_metadata_key(_imp->choices_key);
}

UnpackagedID::~UnpackagedID()
{
}

void
UnpackagedID::need_keys_added() const
{
}

void
UnpackagedID::need_masks_added() const
{
}

const std::string
UnpackagedID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) + ":" +
                stringify(slot()) + "::" + stringify(_imp->repository_name);

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_version:
            return stringify(_imp->name) + ":" + stringify(slot()) + "::" +
                stringify(_imp->repository_name);

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

const QualifiedPackageName
UnpackagedID::name() const
{
    return _imp->name;
}

const VersionSpec
UnpackagedID::version() const
{
    return _imp->version;
}

const SlotName
UnpackagedID::slot() const
{
    return _imp->slot;
}

const std::tr1::shared_ptr<const Repository>
UnpackagedID::repository() const
{
    return _imp->env->package_database()->fetch_repository(_imp->repository_name);
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
UnpackagedID::virtual_for_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
UnpackagedID::keywords_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
UnpackagedID::provide_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
UnpackagedID::contains_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
UnpackagedID::contained_in_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::build_dependencies_key() const
{
    return _imp->build_dependencies_key;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::run_dependencies_key() const
{
    return _imp->run_dependencies_key;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::post_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::suggested_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
UnpackagedID::fetches_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
UnpackagedID::homepage_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnpackagedID::short_description_key() const
{
    return _imp->description_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnpackagedID::long_description_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
UnpackagedID::contents_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >();
}

const std::tr1::shared_ptr<const MetadataTimeKey>
UnpackagedID::installed_time_key() const
{
    return std::tr1::shared_ptr<const MetadataTimeKey>();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
UnpackagedID::from_repositories_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
UnpackagedID::fs_location_key() const
{
    return _imp->fs_location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<bool> >
UnpackagedID::transient_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<bool> >();
}

bool
UnpackagedID::supports_action(const SupportsActionTestBase & test) const
{
    return simple_visitor_cast<const SupportsActionTest<InstallAction> >(test);
}

void
UnpackagedID::perform_action(Action & action) const
{
    const InstallAction * const install_action(simple_visitor_cast<const InstallAction>(action));
    if (! install_action)
        throw UnsupportedActionError(*this, action);

    if (! (*install_action->options.destination()).destination_interface())
        throw InstallActionError("Can't install '" + stringify(*this)
                + "' to destination '" + stringify(install_action->options.destination()->name())
                + "' because destination does not provide destination_interface");

    std::string libdir("lib");
    FSEntry root(install_action->options.destination()->installed_root_key() ?
            stringify(install_action->options.destination()->installed_root_key()->value()) : "/");
    if ((root / "usr" / "lib").is_symbolic_link())
    {
        libdir = (root / "usr" / "lib").readlink();
        if (std::string::npos != libdir.find_first_of("./"))
            libdir = "lib";
    }

    Log::get_instance()->message("unpackaged.libdir", ll_debug, lc_context) << "Using '" << libdir << "' for libdir";

    std::tr1::shared_ptr<const ChoiceValue> strip_choice(choices_key()->value()->find_by_name_with_prefix(
                ELikeStripChoiceValue::canonical_name_with_prefix()));
    std::tr1::shared_ptr<const ChoiceValue> split_choice(choices_key()->value()->find_by_name_with_prefix(
                ELikeSplitChoiceValue::canonical_name_with_prefix()));

    UnpackagedStripper stripper(make_named_values<UnpackagedStripperOptions>(
                value_for<n::debug_dir>(fs_location_key()->value() / "usr" / libdir / "debug"),
                value_for<n::image_dir>(fs_location_key()->value()),
                value_for<n::package_id>(shared_from_this()),
                value_for<n::split>(split_choice && split_choice->enabled()),
                value_for<n::strip>(strip_choice && strip_choice->enabled())
            ));

    stripper.strip();

    (*install_action->options.destination()).destination_interface()->merge(
            make_named_values<MergeParams>(
                value_for<n::environment_file>(FSEntry("/dev/null")),
                value_for<n::image_dir>(fs_location_key()->value()),
                value_for<n::options>(MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs),
                value_for<n::package_id>(shared_from_this()),
                value_for<n::used_this_for_config_protect>(install_action->options.used_this_for_config_protect())
            ));
}

void
UnpackagedID::invalidate_masks() const
{
}

std::tr1::shared_ptr<const Set<std::string> >
UnpackagedID::breaks_portage() const
{
    std::tr1::shared_ptr<Set<std::string> > why(new Set<std::string>);
    why->insert("format");
    return why;
}

bool
UnpackagedID::arbitrary_less_than_comparison(const PackageID & other) const
{
    return slot().data() < other.slot().data();
}

std::size_t
UnpackagedID::extra_hash_value() const
{
    return Hash<SlotName>()(slot());
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
UnpackagedID::choices_key() const
{
    return _imp->choices_key;
}

