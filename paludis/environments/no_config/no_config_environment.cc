/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/map.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/distribution.hh>
#include <paludis/package_database.hh>
#include <paludis/hook.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/repository_factory.hh>
#include <algorithm>
#include <set>
#include <list>

#include "config.h"

using namespace paludis;
using namespace paludis::no_config_environment;

#include <paludis/environments/no_config/no_config_environment-se.cc>

namespace paludis
{
    template<>
    struct Implementation<NoConfigEnvironment>
    {
        const no_config_environment::Params params;

        const FSEntry top_level_dir;
        const FSEntry write_cache;
        bool accept_unstable;
        bool is_vdb;

        std::tr1::shared_ptr<Repository> main_repo;
        std::tr1::shared_ptr<Repository> master_repo;
        std::list<std::tr1::shared_ptr<Repository> > extra_repos;

        std::string paludis_command;

        std::tr1::shared_ptr<PackageDatabase> package_database;

        std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        std::tr1::shared_ptr<LiteralMetadataValueKey<FSEntry> > repository_dir_key;

        Implementation(NoConfigEnvironment * const env, const no_config_environment::Params & params);
        void initialise(NoConfigEnvironment * const env);
    };

    /* This goat is for Dave Wickham */
}

namespace
{
    bool is_vdb_repository(const FSEntry & location, no_config_environment::RepositoryType type)
    {
        switch (type)
        {
            case ncer_ebuild:
                return false;
            case ncer_vdb:
                return true;
            case ncer_auto:
            case last_ncer:
                ;
        }

        Context context("When determining repository type at '" + stringify(location) + "':");

        if (! location.is_directory())
            throw ConfigurationError("Location is not a directory");

        if ((location / "profiles").is_directory())
        {
            Log::get_instance()->message("no_config_environment.ebuild_detected", ll_debug, lc_context)
                << "Found profiles/, looks like Ebuild format";
            return false;
        }

        int outer_count(0);
        for (DirIterator d(location), d_end ; d != d_end ; ++d)
        {
            if (! d->is_directory())
                continue;

            int inner_count(0);
            for (DirIterator e(*d), e_end ; e != e_end ; ++e)
            {
                if (! e->is_directory())
                    continue;

                if ((*e / "CONTENTS").exists())
                {
                    Log::get_instance()->message("no_config_environment.vdb_detected", ll_debug, lc_context)
                        << "Found '" << stringify(*e) << "/CONTENTS', looks like VDB format";
                    return true;
                }

                if (inner_count++ >= 5)
                    break;
            }

            if (outer_count++ >= 5)
                break;
        }

        throw ConfigurationError("Can't work out what kind of repository this is");
    }

    std::string from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }
}

Implementation<NoConfigEnvironment>::Implementation(
        NoConfigEnvironment * const env, const no_config_environment::Params & p) :
    params(p),
    top_level_dir(p.repository_dir()),
    write_cache(p.write_cache()),
    accept_unstable(p.accept_unstable()),
    is_vdb(is_vdb_repository(p.repository_dir(), p.repository_type())),
    paludis_command("false"),
    package_database(new PackageDatabase(env)),
    format_key(new LiteralMetadataValueKey<std::string>("format", "Format", mkt_significant, "no_config")),
    repository_dir_key(new LiteralMetadataValueKey<FSEntry>("repository_dir", "Repository dir",
                mkt_normal, p.repository_dir()))
{
}

void
Implementation<NoConfigEnvironment>::initialise(NoConfigEnvironment * const env)
{
    Context context("When initialising NoConfigEnvironment at '" + stringify(params.repository_dir()) + "':");

    if (! is_vdb)
    {
        bool ignored_one(false);
        for (FSEntrySequence::ConstIterator d(params.extra_repository_dirs()->begin()), d_end(params.extra_repository_dirs()->end()) ;
                d != d_end ; ++d)
        {
            if (params.repository_dir().realpath() == d->realpath())
            {
                Log::get_instance()->message("no_config_environment.extra_repository.ignoring", ll_warning, lc_context)
                    << "Ignoring extra_repository_dir '" << *d << "' because it is the same as repository_dir";
                ignored_one = true;
                continue;
            }

            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);

            if (params.extra_params())
                std::copy(params.extra_params()->begin(), params.extra_params()->end(), keys->inserter());

            keys->insert("format", "ebuild");
            keys->insert("location", stringify(*d));
            keys->insert("profiles", "/var/empty");
            keys->insert("ignore_deprecated_profiles", "true");
            keys->insert("write_cache", stringify(params.write_cache()));
            keys->insert("names_cache", "/var/empty");
            keys->insert("builddir", "/var/empty");
            if (params.disable_metadata_cache())
                keys->insert("cache", "/var/empty");

            std::tr1::shared_ptr<Repository> repo(RepositoryFactory::get_instance()->create(
                        env, std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            if (stringify(repo->name()) == params.master_repository_name())
                master_repo = repo;
            package_database->add_repository(1, repo);
        }

        if ((! params.master_repository_name().empty()) && (! master_repo) && (! ignored_one))
            throw ConfigurationError("Can't find repository '" + params.master_repository_name() + "'");

        std::tr1::shared_ptr<Map<std::string, std::string> > keys( new Map<std::string, std::string>);

        if (params.extra_params())
            std::copy(params.extra_params()->begin(), params.extra_params()->end(), keys->inserter());

        keys->insert("format", "ebuild");
        keys->insert("location", stringify(params.repository_dir()));
        keys->insert("profiles", "/var/empty");
        keys->insert("ignore_deprecated_profiles", "true");
        keys->insert("write_cache", stringify(params.write_cache()));
        keys->insert("names_cache", "/var/empty");
        keys->insert("builddir", "/var/empty");

        if (params.disable_metadata_cache())
            keys->insert("cache", "/var/empty");

        if (master_repo)
            keys->insert("master_repository", params.master_repository_name());

        if ((params.repository_dir() / "metadata" / "profiles_desc.conf").exists())
            keys->insert("layout", "exheres");

        package_database->add_repository(2, ((main_repo =
                        RepositoryFactory::get_instance()->create(env,
                            std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)))));

        if ((! params.master_repository_name().empty()) && (! master_repo) && (params.master_repository_name() != stringify(main_repo->name())))
            throw ConfigurationError("Can't find repository '" + params.master_repository_name() + "'");

#ifdef ENABLE_VIRTUALS_REPOSITORY
        std::tr1::shared_ptr<Map<std::string, std::string> > v_keys(new Map<std::string, std::string>);
        v_keys->insert("format", "virtuals");
        if ((*DistributionData::get_instance()->distribution_from_string(env->distribution())).support_old_style_virtuals())
            package_database->add_repository(-2, RepositoryFactory::get_instance()->create(env,
                        std::tr1::bind(from_keys, v_keys, std::tr1::placeholders::_1)));
#endif
    }
    else
    {
        Log::get_instance()->message("no_config_environment.vdb_detected", ll_debug, lc_context) << "VDB, using vdb_db";

        std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
        if (params.extra_params())
            std::copy(params.extra_params()->begin(), params.extra_params()->end(), keys->inserter());

        keys->insert("format", "vdb");
        keys->insert("names_cache", "/var/empty");
        keys->insert("provides_cache", "/var/empty");
        keys->insert("location", stringify(top_level_dir));

        package_database->add_repository(1, RepositoryFactory::get_instance()->create(env,
                    std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

        std::tr1::shared_ptr<Map<std::string, std::string> > iv_keys(
                new Map<std::string, std::string>);
        iv_keys->insert("root", "/");
        iv_keys->insert("format", "installed_virtuals");

#ifdef ENABLE_VIRTUALS_REPOSITORY
        if ((*DistributionData::get_instance()->distribution_from_string(env->distribution())).support_old_style_virtuals())
            package_database->add_repository(-2, RepositoryFactory::get_instance()->create(env,
                        std::tr1::bind(from_keys, iv_keys, std::tr1::placeholders::_1)));
#endif
    }
}

NoConfigEnvironment::NoConfigEnvironment(const no_config_environment::Params & params) :
    PrivateImplementationPattern<NoConfigEnvironment>(
            new Implementation<NoConfigEnvironment>(this, params)),
    _imp(PrivateImplementationPattern<NoConfigEnvironment>::_imp)
{
    _imp->initialise(this);

    if (_imp->main_repo)
        if ((*_imp->main_repo).e_interface()->end_profiles() != (*_imp->main_repo).e_interface()->begin_profiles())
            (*_imp->main_repo).e_interface()->set_profile((*_imp->main_repo).e_interface()->begin_profiles());

    if (_imp->master_repo)
        if ((*_imp->master_repo).e_interface()->end_profiles() !=
                (*_imp->master_repo).e_interface()->begin_profiles())
            (*_imp->master_repo).e_interface()->set_profile(
                    (*_imp->master_repo).e_interface()->begin_profiles());

    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->repository_dir_key);
}

NoConfigEnvironment::~NoConfigEnvironment()
{
}

FSEntry
NoConfigEnvironment::main_repository_dir() const
{
    return _imp->top_level_dir;
}

void
NoConfigEnvironment::set_accept_unstable(const bool value)
{
    _imp->accept_unstable = value;
    for (PackageDatabase::RepositoryConstIterator it(_imp->package_database->begin_repositories()),
             it_end(_imp->package_database->end_repositories());
         it_end != it; ++it)
        (*it)->invalidate_masks();
}

std::tr1::shared_ptr<Repository>
NoConfigEnvironment::main_repository()
{
    return _imp->main_repo;
}

std::tr1::shared_ptr<const Repository>
NoConfigEnvironment::main_repository() const
{
    return _imp->main_repo;
}

std::tr1::shared_ptr<Repository>
NoConfigEnvironment::master_repository()
{
    return _imp->master_repo;
}

std::tr1::shared_ptr<const Repository>
NoConfigEnvironment::master_repository() const
{
    return _imp->master_repo;
}

std::tr1::shared_ptr<PackageDatabase>
NoConfigEnvironment::package_database()
{
    return _imp->package_database;
}

std::tr1::shared_ptr<const PackageDatabase>
NoConfigEnvironment::package_database() const
{
    return _imp->package_database;
}

std::string
NoConfigEnvironment::paludis_command() const
{
    return _imp->paludis_command;
}

void
NoConfigEnvironment::set_paludis_command(const std::string & s)
{
    _imp->paludis_command = s;
}

bool
NoConfigEnvironment::accept_keywords(const std::tr1::shared_ptr<const KeywordNameSet> & keywords,
        const PackageID &) const
{
    if (_imp->is_vdb)
        return true;

    std::string accept_keywords_var((*_imp->main_repo).e_interface()->accept_keywords_variable());
    std::string ak;
    if (! accept_keywords_var.empty())
        ak = (*_imp->main_repo).e_interface()->profile_variable(accept_keywords_var);

    if (ak.empty())
    {
        std::string arch_var((*_imp->main_repo).e_interface()->arch_variable());

        if (arch_var.empty())
        {
            if (_imp->params.extra_accept_keywords().empty())
                throw ConfigurationError("Don't know how to work out whether keywords are acceptable");
        }
        else
        {
            std::string arch((*_imp->main_repo).e_interface()->profile_variable(arch_var));

            if (keywords->end() != keywords->find(KeywordName(arch)))
                return true;

            if (_imp->accept_unstable && keywords->end() != keywords->find(KeywordName("~" + arch)))
                return true;
        }
    }
    else
    {
        std::list<KeywordName> accepted;
        tokenise_whitespace(ak, create_inserter<KeywordName>(std::back_inserter(accepted)));

        for (KeywordNameSet::ConstIterator k(keywords->begin()), k_end(keywords->end()) ;
                k != k_end ; ++k)
        {
            if (accepted.end() != std::find(accepted.begin(), accepted.end(), *k))
                return true;

            if (_imp->accept_unstable && stringify(*k).at(0) == '~')
                if (accepted.end() != std::find(accepted.begin(), accepted.end(), KeywordName(stringify(*k).substr(1))))
                    return true;
        }
    }

    {
        std::list<KeywordName> accepted;
        tokenise_whitespace(_imp->params.extra_accept_keywords(),
                create_inserter<KeywordName>(std::back_inserter(accepted)));

        for (KeywordNameSet::ConstIterator k(keywords->begin()), k_end(keywords->end()) ;
                k != k_end ; ++k)
        {
            if (accepted.end() != std::find(accepted.begin(), accepted.end(), *k))
                return true;

            if (_imp->accept_unstable && stringify(*k).at(0) == '~')
                if (accepted.end() != std::find(accepted.begin(), accepted.end(), KeywordName(stringify(*k).substr(1))))
                    return true;
        }
    }

    return false;
}

const std::tr1::shared_ptr<const SetSpecTree>
NoConfigEnvironment::local_set(const SetName &) const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const SetSpecTree>
NoConfigEnvironment::world_set() const
{
    return make_null_shared_ptr();
}

void
NoConfigEnvironment::add_to_world(const QualifiedPackageName &) const
{
}

void
NoConfigEnvironment::remove_from_world(const QualifiedPackageName &) const
{
}

void
NoConfigEnvironment::add_to_world(const SetName &) const
{
}

void
NoConfigEnvironment::remove_from_world(const SetName &) const
{
}

bool
NoConfigEnvironment::unmasked_by_user(const PackageID &) const
{
    return false;
}

const std::tr1::shared_ptr<const Mask>
NoConfigEnvironment::mask_for_breakage(const PackageID &) const
{
    return std::tr1::shared_ptr<const Mask>();
}

const std::tr1::shared_ptr<const Mask>
NoConfigEnvironment::mask_for_user(const PackageID &) const
{
    return std::tr1::shared_ptr<const Mask>();
}

uid_t
NoConfigEnvironment::reduced_uid() const
{
    return getuid();
}

gid_t
NoConfigEnvironment::reduced_gid() const
{
    return getgid();
}

std::tr1::shared_ptr<const MirrorsSequence>
NoConfigEnvironment::mirrors(const std::string &) const
{
    return make_shared_ptr(new MirrorsSequence);
}

bool
NoConfigEnvironment::accept_license(const std::string &, const PackageID &) const
{
    return true;
}

const FSEntry
NoConfigEnvironment::root() const
{
    return FSEntry("/");
}

HookResult
NoConfigEnvironment::perform_hook(const Hook &) const
{
    return make_named_values<HookResult>(value_for<n::max_exit_status>(0), value_for<n::output>(""));
}

std::tr1::shared_ptr<const FSEntrySequence>
NoConfigEnvironment::hook_dirs() const
{
    return make_shared_ptr(new FSEntrySequence);
}

void
NoConfigEnvironment::need_keys_added() const
{
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
NoConfigEnvironment::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
NoConfigEnvironment::config_location_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const Tribool
NoConfigEnvironment::want_choice_enabled(
        const std::tr1::shared_ptr<const PackageID> &,
        const std::tr1::shared_ptr<const Choice> &,
        const UnprefixedChoiceName &
        ) const
{
    return Tribool(indeterminate);
}

std::tr1::shared_ptr<const Set<UnprefixedChoiceName> >
NoConfigEnvironment::known_choice_value_names(
        const std::tr1::shared_ptr<const PackageID> &,
        const std::tr1::shared_ptr<const Choice> &
        ) const
{
    return make_shared_ptr(new Set<UnprefixedChoiceName>);
}

