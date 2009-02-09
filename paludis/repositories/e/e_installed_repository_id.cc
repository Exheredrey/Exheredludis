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

#include <paludis/repositories/e/e_installed_repository_id.hh>
#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/e_choices_key.hh>
#include <paludis/repositories/e/extra_distribution_data.hh>
#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/dependencies_rewriter.hh>
#include <paludis/repositories/e/e_choice_value.hh>

#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/literal_metadata_key.hh>
#include <iterator>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    std::string file_contents(const FSEntry & f)
    {
        Context c("When reading '" + stringify(f) + "':");
        SafeIFStream i(f);
        return strip_trailing(std::string((std::istreambuf_iterator<char>(i)), std::istreambuf_iterator<char>()), "\r\n");
    }
}

namespace paludis
{
    template <>
    struct Implementation<EInstalledRepositoryID>
    {
        mutable Mutex mutex;

        const QualifiedPackageName name;
        const VersionSpec version;
        const Environment * const environment;
        const std::tr1::shared_ptr<const Repository> repository;
        const FSEntry dir;
        mutable bool has_keys;

        std::tr1::shared_ptr<const EAPI> eapi;

        std::tr1::shared_ptr<const MetadataValueKey<SlotName> > slot;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > fs_location;
        std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use;
        std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > inherited;
        std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string > > > raw_iuse;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > raw_myoptions;
        std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string > > > raw_use_expand;
        std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string > > > raw_use_expand_hidden;
        std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > > choices;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > provide;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > restrictions;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > properties;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > src_uri;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > short_description;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > long_description;
        std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > > contents;
        std::tr1::shared_ptr<const MetadataTimeKey> installed_time;
        std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > upstream_changelog;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > upstream_documentation;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > upstream_release_notes;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > remote_ids;
        std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > bugs_to;
        std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > defined_phases;

        std::tr1::shared_ptr<const MetadataValueKey<std::string> > asflags;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > cbuild;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > cflags;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > chost;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > ctarget;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > cxxflags;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > ldflags;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > pkgmanager;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > vdb_format;

        std::tr1::shared_ptr<DependencyLabelSequence> build_dependencies_labels;
        std::tr1::shared_ptr<DependencyLabelSequence> run_dependencies_labels;
        std::tr1::shared_ptr<DependencyLabelSequence> post_dependencies_labels;

        Implementation(const QualifiedPackageName & q, const VersionSpec & v,
                const Environment * const e,
                const std::tr1::shared_ptr<const Repository> r, const FSEntry & f) :
            name(q),
            version(v),
            environment(e),
            repository(r),
            dir(f),
            has_keys(false),
            build_dependencies_labels(new DependencyLabelSequence),
            run_dependencies_labels(new DependencyLabelSequence),
            post_dependencies_labels(new DependencyLabelSequence)
        {
            build_dependencies_labels->push_back(make_shared_ptr(new DependencyBuildLabel("DEPEND")));
            run_dependencies_labels->push_back(make_shared_ptr(new DependencyRunLabel("RDEPEND")));
            post_dependencies_labels->push_back(make_shared_ptr(new DependencyPostLabel("PDEPEND")));
        }
    };
}

EInstalledRepositoryID::EInstalledRepositoryID(const QualifiedPackageName & q, const VersionSpec & v,
        const Environment * const e,
        const std::tr1::shared_ptr<const Repository> & r,
        const FSEntry & f) :
    PrivateImplementationPattern<EInstalledRepositoryID>(new Implementation<EInstalledRepositoryID>(q, v, e, r, f)),
    _imp(PrivateImplementationPattern<EInstalledRepositoryID>::_imp)
{
}

EInstalledRepositoryID::~EInstalledRepositoryID()
{
}

void
EInstalledRepositoryID::need_keys_added() const
{
    Lock l(_imp->mutex);

    if (_imp->has_keys)
        return;
    _imp->has_keys = true;

    // fs_location key could have been loaded by the ::fs_location_key() already. keep this
    // at the top, other keys use it.
    if (! _imp->fs_location)
    {
        _imp->fs_location.reset(new LiteralMetadataValueKey<FSEntry> (fs_location_raw_name(), fs_location_human_name(),
                    mkt_internal, _imp->dir));
        add_metadata_key(_imp->fs_location);
    }

    Context context("When loading ID keys from '" + stringify(_imp->dir) + "':");

    if (! eapi()->supported())
    {
        Log::get_instance()->message("e.eapi.unsupported", ll_debug, lc_context)
            << "Not loading further keys for '" << *this << "' because EAPI '"
            << eapi()->name() << "' is not supported";
        return;
    }

    std::tr1::shared_ptr<const EAPIEbuildMetadataVariables> vars(eapi()->supported()->ebuild_metadata_variables());
    std::tr1::shared_ptr<const EAPIEbuildEnvironmentVariables> env(eapi()->supported()->ebuild_environment_variables());

    if (! env->env_use().empty())
        if ((_imp->dir / env->env_use()).exists())
        {
            _imp->raw_use.reset(new EStringSetKey(shared_from_this(), env->env_use(), env->description_use(),
                        file_contents(_imp->dir / env->env_use()), mkt_internal));
            add_metadata_key(_imp->raw_use);
        }

    if (! vars->slot()->name().empty())
        if ((_imp->dir / vars->slot()->name()).exists())
        {
            _imp->slot.reset(new LiteralMetadataValueKey<SlotName>(vars->slot()->name(), vars->slot()->description(),
                        mkt_internal, SlotName(file_contents(_imp->dir / vars->slot()->name()))));
            add_metadata_key(_imp->slot);
        }

    if (! vars->inherited()->name().empty())
        if ((_imp->dir / vars->inherited()->name()).exists())
        {
            _imp->inherited.reset(new EStringSetKey(shared_from_this(), vars->inherited()->name(), vars->inherited()->description(),
                        file_contents(_imp->dir / vars->inherited()->name()), mkt_internal));
            add_metadata_key(_imp->inherited);
        }

    if (! vars->defined_phases()->name().empty())
        if ((_imp->dir / vars->defined_phases()->name()).exists())
        {
            std::string d(file_contents(_imp->dir / vars->defined_phases()->name()));
            if (! d.empty())
            {
                _imp->defined_phases.reset(new EStringSetKey(shared_from_this(), vars->defined_phases()->name(), vars->defined_phases()->description(),
                            d, mkt_internal));
                add_metadata_key(_imp->defined_phases);
            }
        }

    if (! vars->iuse()->name().empty())
    {
        if ((_imp->dir / vars->iuse()->name()).exists())
            _imp->raw_iuse.reset(new EStringSetKey(shared_from_this(), vars->iuse()->name(), vars->iuse()->description(),
                        file_contents(_imp->dir / vars->iuse()->name()), mkt_internal));
        else
        {
            /* hack: if IUSE doesn't exist, we still need an iuse_key to make the choices
             * code behave sanely. */
            _imp->raw_iuse.reset(new EStringSetKey(shared_from_this(), vars->iuse()->name(), vars->iuse()->description(),
                        "", mkt_internal));
        }
        add_metadata_key(_imp->raw_iuse);
    }

    if (! vars->myoptions()->name().empty())
        if ((_imp->dir / vars->myoptions()->name()).exists())
        {
            _imp->raw_myoptions.reset(new EMyOptionsKey(_imp->environment, shared_from_this(), vars->myoptions()->name(),
                        vars->myoptions()->description(), file_contents(_imp->dir / vars->myoptions()->name()), mkt_internal));
            add_metadata_key(_imp->raw_myoptions);
        }

    if (! vars->use_expand()->name().empty())
        if ((_imp->dir / vars->use_expand()->name()).exists())
        {
            _imp->raw_use_expand.reset(new EStringSetKey(shared_from_this(), vars->use_expand()->name(), vars->use_expand()->description(),
                        file_contents(_imp->dir / vars->use_expand()->name()), mkt_internal));
            add_metadata_key(_imp->raw_use_expand);
        }

    if (! vars->use_expand_hidden()->name().empty())
        if ((_imp->dir / vars->use_expand_hidden()->name()).exists())
        {
            _imp->raw_use_expand_hidden.reset(new EStringSetKey(shared_from_this(), vars->use_expand_hidden()->name(), vars->use_expand_hidden()->description(),
                        file_contents(_imp->dir / vars->use_expand_hidden()->name()), mkt_internal));
            add_metadata_key(_imp->raw_use_expand_hidden);
        }

    if (! vars->license()->name().empty())
        if ((_imp->dir / vars->license()->name()).exists())
        {
            _imp->license.reset(new ELicenseKey(_imp->environment, shared_from_this(), vars->license(),
                        file_contents(_imp->dir / vars->license()->name()),  mkt_normal));
            add_metadata_key(_imp->license);
        }

    if (! vars->provide()->name().empty())
        if ((_imp->dir / vars->provide()->name()).exists())
        {
            _imp->provide.reset(new EProvideKey(_imp->environment, shared_from_this(), vars->provide()->name(), vars->provide()->description(),
                        file_contents(_imp->dir / vars->provide()->name()), mkt_internal));
            add_metadata_key(_imp->provide);
        }

    if (! vars->dependencies()->name().empty())
    {
        if ((_imp->dir / vars->dependencies()->name()).exists())
        {
            DependenciesRewriter rewriter;
            parse_depend(file_contents(_imp->dir / vars->dependencies()->name()), _imp->environment, shared_from_this(), *eapi())->root()->accept(rewriter);

            _imp->build_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), vars->dependencies()->name() + ".DEPEND",
                        vars->dependencies()->description() + " (build)", rewriter.depend(), _imp->build_dependencies_labels, mkt_dependencies));
            add_metadata_key(_imp->build_dependencies);

            _imp->run_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), vars->dependencies()->name() + ".RDEPEND",
                        vars->dependencies()->description() + " (run)", rewriter.rdepend(), _imp->build_dependencies_labels, mkt_dependencies));
            add_metadata_key(_imp->run_dependencies);

            _imp->post_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), vars->dependencies()->name() + ".PDEPEND",
                        vars->dependencies()->description() + " (post)", rewriter.pdepend(), _imp->build_dependencies_labels, mkt_dependencies));
            add_metadata_key(_imp->post_dependencies);
        }
    }
    else
    {
        if (! vars->build_depend()->name().empty())
            if ((_imp->dir / vars->build_depend()->name()).exists())
            {
                _imp->build_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), vars->build_depend()->name(),
                            vars->build_depend()->description(), file_contents(_imp->dir / vars->build_depend()->name()),
                            _imp->build_dependencies_labels, mkt_dependencies));
                add_metadata_key(_imp->build_dependencies);
            }

        if (! vars->run_depend()->name().empty())
            if ((_imp->dir / vars->run_depend()->name()).exists())
            {
                _imp->run_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), vars->run_depend()->name(),
                            vars->run_depend()->description(), file_contents(_imp->dir / vars->run_depend()->name()),
                            _imp->run_dependencies_labels, mkt_dependencies));
                add_metadata_key(_imp->run_dependencies);
            }

        if (! vars->pdepend()->name().empty())
            if ((_imp->dir / vars->pdepend()->name()).exists())
            {
                _imp->post_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), vars->pdepend()->name(),
                            vars->pdepend()->description(), file_contents(_imp->dir / vars->pdepend()->name()),
                            _imp->post_dependencies_labels, mkt_dependencies));
                add_metadata_key(_imp->post_dependencies);
            }
    }

    if (! vars->restrictions()->name().empty())
        if ((_imp->dir / vars->restrictions()->name()).exists())
        {
            _imp->restrictions.reset(new EPlainTextSpecKey(_imp->environment, shared_from_this(), vars->restrictions(),
                        file_contents(_imp->dir / vars->restrictions()->name()), mkt_internal));
            add_metadata_key(_imp->restrictions);
        }

    if (! vars->properties()->name().empty())
        if ((_imp->dir / vars->properties()->name()).exists())
        {
            _imp->properties.reset(new EPlainTextSpecKey(_imp->environment, shared_from_this(), vars->properties(),
                        file_contents(_imp->dir / vars->properties()->name()), mkt_internal));
            add_metadata_key(_imp->properties);
        }

    if (! vars->src_uri()->name().empty())
        if ((_imp->dir / vars->src_uri()->name()).exists())
        {
            _imp->src_uri.reset(new EFetchableURIKey(_imp->environment, shared_from_this(), vars->src_uri(),
                        file_contents(_imp->dir / vars->src_uri()->name()), mkt_dependencies));
            add_metadata_key(_imp->src_uri);
        }

    if (! vars->short_description()->name().empty())
        if ((_imp->dir / vars->short_description()->name()).exists())
        {
            _imp->short_description.reset(new LiteralMetadataValueKey<std::string> (vars->short_description()->name(),
                        vars->short_description()->description(), mkt_significant, file_contents(_imp->dir / vars->short_description()->name())));
            add_metadata_key(_imp->short_description);
        }

    if (! vars->long_description()->name().empty())
        if ((_imp->dir / vars->long_description()->name()).exists())
        {
            std::string value(file_contents(_imp->dir / vars->long_description()->name()));
            if (! value.empty())
            {
                _imp->long_description.reset(new LiteralMetadataValueKey<std::string> (vars->long_description()->name(),
                            vars->long_description()->description(), mkt_significant, value));
                add_metadata_key(_imp->long_description);
            }
        }

    if (! vars->upstream_changelog()->name().empty())
        if ((_imp->dir / vars->upstream_changelog()->name()).exists())
        {
            std::string value(file_contents(_imp->dir / vars->upstream_changelog()->name()));
            if (! value.empty())
            {
                _imp->upstream_changelog.reset(new ESimpleURIKey(_imp->environment, shared_from_this(),
                            vars->upstream_changelog()->name(),
                            vars->upstream_changelog()->description(), value, mkt_normal));
                add_metadata_key(_imp->upstream_changelog);
            }
        }

    if (! vars->upstream_release_notes()->name().empty())
        if ((_imp->dir / vars->upstream_release_notes()->name()).exists())
        {
            std::string value(file_contents(_imp->dir / vars->upstream_release_notes()->name()));
            if (! value.empty())
            {
                _imp->upstream_release_notes.reset(new ESimpleURIKey(_imp->environment, shared_from_this(),
                            vars->upstream_release_notes()->name(),
                            vars->upstream_release_notes()->description(), value, mkt_normal));
                add_metadata_key(_imp->upstream_release_notes);
            }
        }

    if (! vars->upstream_documentation()->name().empty())
        if ((_imp->dir / vars->upstream_documentation()->name()).exists())
        {
            std::string value(file_contents(_imp->dir / vars->upstream_documentation()->name()));
            if (! value.empty())
            {
                _imp->upstream_documentation.reset(new ESimpleURIKey(_imp->environment, shared_from_this(),
                            vars->upstream_documentation()->name(),
                            vars->upstream_documentation()->description(), value, mkt_normal));
                add_metadata_key(_imp->upstream_documentation);
            }
        }

    if (! vars->bugs_to()->name().empty())
        if ((_imp->dir / vars->bugs_to()->name()).exists())
        {
            std::string value(file_contents(_imp->dir / vars->bugs_to()->name()));
            if (! value.empty())
            {
                _imp->bugs_to.reset(new EPlainTextSpecKey(_imp->environment, shared_from_this(),
                            vars->bugs_to(), value, mkt_normal));
                add_metadata_key(_imp->bugs_to);
            }
        }

    if (! vars->remote_ids()->name().empty())
        if ((_imp->dir / vars->remote_ids()->name()).exists())
        {
            std::string value(file_contents(_imp->dir / vars->remote_ids()->name()));
            if (! value.empty())
            {
                _imp->remote_ids.reset(new EPlainTextSpecKey(_imp->environment, shared_from_this(),
                            vars->remote_ids(), value, mkt_internal));
                add_metadata_key(_imp->remote_ids);
            }
        }

    if (! vars->homepage()->name().empty())
        if ((_imp->dir / vars->homepage()->name()).exists())
        {
            _imp->homepage.reset(new ESimpleURIKey(_imp->environment, shared_from_this(), vars->homepage()->name(),
                        vars->homepage()->description(),
                        file_contents(_imp->dir / vars->homepage()->name()), mkt_significant));
            add_metadata_key(_imp->homepage);
        }

    _imp->contents = make_contents_key();
    add_metadata_key(_imp->contents);

    _imp->installed_time.reset(new EMTimeKey(shared_from_this(), "INSTALLED_TIME", "Installed time",
                _imp->dir / contents_filename(), mkt_normal));
    add_metadata_key(_imp->installed_time);

    if (_imp->eapi->supported())
        _imp->choices.reset(new EChoicesKey(_imp->environment, shared_from_this(), "PALUDIS_CHOICES",
                    _imp->eapi->supported()->ebuild_environment_variables()->description_choices(),
                    mkt_normal, make_null_shared_ptr(), make_null_shared_ptr()));
    else
        _imp->choices.reset(new EChoicesKey(_imp->environment, shared_from_this(), "PALUDIS_CHOICES", "Choices", mkt_normal,
                    make_null_shared_ptr(), make_null_shared_ptr()));

    add_metadata_key(_imp->choices);

    std::tr1::shared_ptr<Set<std::string> > from_repositories_value(new Set<std::string>);
    if ((_imp->dir / "REPOSITORY").exists())
        from_repositories_value->insert(file_contents(_imp->dir / "REPOSITORY"));
    if ((_imp->dir / "repository").exists())
        from_repositories_value->insert(file_contents(_imp->dir / "repository"));
    if ((_imp->dir / "BINARY_REPOSITORY").exists())
        from_repositories_value->insert(file_contents(_imp->dir / "BINARY_REPOSITORY"));
    if (! from_repositories_value->empty())
    {
        _imp->from_repositories.reset(new LiteralMetadataStringSetKey("REPOSITORIES",
                    "From repositories", mkt_normal, from_repositories_value));
        add_metadata_key(_imp->from_repositories);
    }

    if ((_imp->dir / "ASFLAGS").exists())
    {
        _imp->asflags.reset(new LiteralMetadataValueKey<std::string> ("ASFLAGS", "ASFLAGS",
                    mkt_internal, file_contents(_imp->dir / "ASFLAGS")));
        add_metadata_key(_imp->asflags);
    }

    if ((_imp->dir / "CBUILD").exists())
    {
        _imp->cbuild.reset(new LiteralMetadataValueKey<std::string> ("CBUILD", "CBUILD",
                    mkt_internal, file_contents(_imp->dir / "CBUILD")));
        add_metadata_key(_imp->cbuild);
    }

    if ((_imp->dir / "CFLAGS").exists())
    {
        _imp->cflags.reset(new LiteralMetadataValueKey<std::string> ("CFLAGS", "CFLAGS",
                    mkt_internal, file_contents(_imp->dir / "CFLAGS")));
        add_metadata_key(_imp->cflags);
    }

    if ((_imp->dir / "CHOST").exists())
    {
        _imp->chost.reset(new LiteralMetadataValueKey<std::string> ("CHOST", "CHOST",
                    mkt_internal, file_contents(_imp->dir / "CHOST")));
        add_metadata_key(_imp->chost);
    }

    if ((_imp->dir / "CXXFLAGS").exists())
    {
        _imp->cxxflags.reset(new LiteralMetadataValueKey<std::string> ("CXXFLAGS", "CXXFLAGS",
                    mkt_internal, file_contents(_imp->dir / "CXXFLAGS")));
        add_metadata_key(_imp->cxxflags);
    }

    if ((_imp->dir / "LDFLAGS").exists())
    {
        _imp->ldflags.reset(new LiteralMetadataValueKey<std::string> ("LDFLAGS", "LDFLAGS",
                    mkt_internal, file_contents(_imp->dir / "LDFLAGS")));
        add_metadata_key(_imp->ldflags);
    }

    if ((_imp->dir / "PKGMANAGER").exists())
    {
        _imp->pkgmanager.reset(new LiteralMetadataValueKey<std::string> ("PKGMANAGER", "Installed using",
                    mkt_normal, file_contents(_imp->dir / "PKGMANAGER")));
        add_metadata_key(_imp->pkgmanager);
    }

    if ((_imp->dir / "VDB_FORMAT").exists())
    {
        _imp->vdb_format.reset(new LiteralMetadataValueKey<std::string> ("VDB_FORMAT", "VDB Format",
                    mkt_internal, file_contents(_imp->dir / "VDB_FORMAT")));
        add_metadata_key(_imp->vdb_format);
    }
}

void
EInstalledRepositoryID::need_masks_added() const
{
}

const std::string
EInstalledRepositoryID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            if (_imp->slot)
                return stringify(name()) + "-" + stringify(version()) + ":" + stringify(_imp->slot->value()) + "::" +
                    stringify(repository()->name());

            return stringify(name()) + "-" + stringify(version()) + "::" + stringify(repository()->name());

        case idcf_no_version:
            if (_imp->slot)
                return stringify(name()) + ":" + stringify(_imp->slot->value()) + "::" +
                    stringify(repository()->name());

            return stringify(name()) + "::" + stringify(repository()->name());

        case idcf_version:
            return stringify(version());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

const QualifiedPackageName
EInstalledRepositoryID::name() const
{
    return _imp->name;
}

const VersionSpec
EInstalledRepositoryID::version() const
{
    return _imp->version;
}

const std::tr1::shared_ptr<const Repository>
EInstalledRepositoryID::repository() const
{
    return _imp->repository;
}

const std::tr1::shared_ptr<const EAPI>
EInstalledRepositoryID::eapi() const
{
    Lock l(_imp->mutex);

    if (_imp->eapi)
        return _imp->eapi;

    Context context("When finding EAPI for '" + canonical_form(idcf_full) + "':");

    if ((_imp->dir / "EAPI").exists())
        _imp->eapi = EAPIData::get_instance()->eapi_from_string(file_contents(_imp->dir / "EAPI"));
    else
    {
        Log::get_instance()->message("e.no_eapi", ll_debug, lc_context) << "No EAPI entry in '" << _imp->dir << "', pretending '"
            << _imp->environment->distribution() << "'";
        _imp->eapi = EAPIData::get_instance()->eapi_from_string(
                EExtraDistributionData::get_instance()->data_from_distribution(*DistributionData::get_instance()->distribution_from_string(
                        _imp->environment->distribution()))->default_eapi_when_unspecified());
    }

    return _imp->eapi;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
EInstalledRepositoryID::virtual_for_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
EInstalledRepositoryID::keywords_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::raw_use_key() const
{
    need_keys_added();
    return _imp->raw_use;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::raw_iuse_key() const
{
    need_keys_added();
    return _imp->raw_iuse;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EInstalledRepositoryID::raw_myoptions_key() const
{
    need_keys_added();
    return _imp->raw_myoptions;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::raw_use_expand_key() const
{
    need_keys_added();
    return _imp->raw_use_expand;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::raw_use_expand_hidden_key() const
{
    need_keys_added();
    return _imp->raw_use_expand_hidden;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >
EInstalledRepositoryID::license_key() const
{
    need_keys_added();
    return _imp->license;
}

const std::tr1::shared_ptr<const MetadataValueKey<bool> >
EInstalledRepositoryID::transient_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<bool> >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::inherited_key() const
{
    need_keys_added();
    return _imp->inherited;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::defined_phases_key() const
{
    need_keys_added();
    return _imp->defined_phases;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
EInstalledRepositoryID::provide_key() const
{
    need_keys_added();
    return _imp->provide;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EInstalledRepositoryID::build_dependencies_key() const
{
    need_keys_added();
    return _imp->build_dependencies;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EInstalledRepositoryID::run_dependencies_key() const
{
    need_keys_added();
    return _imp->run_dependencies;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EInstalledRepositoryID::post_dependencies_key() const
{
    need_keys_added();
    return _imp->post_dependencies;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EInstalledRepositoryID::suggested_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EInstalledRepositoryID::restrict_key() const
{
    need_keys_added();
    return _imp->restrictions;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EInstalledRepositoryID::properties_key() const
{
    need_keys_added();
    return _imp->properties;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
EInstalledRepositoryID::choices_key() const
{
    need_keys_added();
    return _imp->choices;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
EInstalledRepositoryID::fetches_key() const
{
    need_keys_added();
    return _imp->src_uri;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
EInstalledRepositoryID::homepage_key() const
{
    need_keys_added();
    return _imp->homepage;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
EInstalledRepositoryID::short_description_key() const
{
    need_keys_added();
    return _imp->short_description;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
EInstalledRepositoryID::long_description_key() const
{
    need_keys_added();
    return _imp->long_description;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
EInstalledRepositoryID::contents_key() const
{
    need_keys_added();
    return _imp->contents;
}

const std::tr1::shared_ptr<const MetadataTimeKey>
EInstalledRepositoryID::installed_time_key() const
{
    need_keys_added();
    return _imp->installed_time;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::from_repositories_key() const
{
    need_keys_added();
    return _imp->from_repositories;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
EInstalledRepositoryID::fs_location_key() const
{
    // Avoid loading whole metadata
    if (! _imp->fs_location)
    {
        Lock l(_imp->mutex);

        _imp->fs_location.reset(new LiteralMetadataValueKey<FSEntry> (fs_location_raw_name(),
                    fs_location_human_name(), mkt_internal, _imp->dir));
        add_metadata_key(_imp->fs_location);
    }

    return _imp->fs_location;
}

bool
EInstalledRepositoryID::arbitrary_less_than_comparison(const PackageID &) const
{
    return false;
}

std::size_t
EInstalledRepositoryID::extra_hash_value() const
{
    return 0;
}

namespace
{
    struct SupportsActionQuery
    {
        bool visit(const SupportsActionTest<InstalledAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<ConfigAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return true;
        }
    };
}

bool
EInstalledRepositoryID::supports_action(const SupportsActionTestBase & b) const
{
    SupportsActionQuery q;
    return b.accept_returning<bool>(q);
}

namespace
{
    struct PerformAction
    {
        const std::tr1::shared_ptr<const erepository::ERepositoryID> id;

        PerformAction(const std::tr1::shared_ptr<const erepository::ERepositoryID> i) :
            id(i)
        {
        }

        void visit(const UninstallAction & a)
        {
            std::tr1::static_pointer_cast<const EInstalledRepository>(id->repository())->perform_uninstall(id, false, a.options.config_protect());
        }

        void visit(const InstalledAction &)
        {
        }

        void visit(const ConfigAction &)
        {
            std::tr1::static_pointer_cast<const EInstalledRepository>(id->repository())->perform_config(id);
        }

        void visit(const InfoAction &)
        {
            std::tr1::static_pointer_cast<const EInstalledRepository>(id->repository())->perform_info(id);
        }

        void visit(const InstallAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const PretendAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const FetchAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const PretendFetchAction & a) PALUDIS_ATTRIBUTE((noreturn));
    };

    void PerformAction::visit(const InstallAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }

    void PerformAction::visit(const PretendAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }

    void PerformAction::visit(const FetchAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }

    void PerformAction::visit(const PretendFetchAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }
}

void
EInstalledRepositoryID::perform_action(Action & a) const
{
    PerformAction b(shared_from_this());
    a.accept(b);
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
EInstalledRepositoryID::contains_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
EInstalledRepositoryID::contained_in_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataValueKey<SlotName> >
EInstalledRepositoryID::slot_key() const
{
    need_keys_added();
    return _imp->slot;
}

std::tr1::shared_ptr<ChoiceValue>
EInstalledRepositoryID::make_choice_value(const std::tr1::shared_ptr<const Choice> & c, const UnprefixedChoiceName & v,
        const Tribool, const bool explicitly_listed, const std::string & override_description, const bool) const
{
    if (! eapi()->supported())
        throw InternalError(PALUDIS_HERE, "Unsupported EAPI");

    std::string name_with_prefix;
    if (stringify(c->prefix()).empty())
        name_with_prefix = stringify(v);
    else
    {
        char use_expand_separator(eapi()->supported()->choices_options()->use_expand_separator());
        if (! use_expand_separator)
            throw InternalError(PALUDIS_HERE, "No use_expand_separator defined");
        name_with_prefix = stringify(c->prefix()) + std::string(1, use_expand_separator) + stringify(v);
    }

    bool enabled(false);
    if (raw_use_key())
        enabled = (raw_use_key()->value()->end() != raw_use_key()->value()->find(name_with_prefix));

    return make_shared_ptr(new EChoiceValue(c->prefix(), v, ChoiceNameWithPrefix(name_with_prefix), name(), std::tr1::shared_ptr<const UseDesc>(),
                enabled, enabled, true, explicitly_listed, override_description));
}

void
EInstalledRepositoryID::add_build_options(const std::tr1::shared_ptr<Choices> &) const
{
}

