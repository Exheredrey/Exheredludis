/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/repositories/e/exheres_profile.hh>
#include <paludis/repositories/e/e_repository_mask_file.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/profile_file.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/map.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/system.hh>
#include <paludis/choice.hh>
#include <paludis/dep_tag.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <paludis/distribution.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/paludislike_options_conf.hh>
#include <tr1/unordered_map>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    const std::tr1::shared_ptr<const LineConfigFile> make_config_file(
            const FSEntry & f,
            const LineConfigFileOptions & o)
    {
        return make_shared_ptr(new LineConfigFile(f, o));
    }

    typedef std::tr1::unordered_map<std::string, std::string, Hash<std::string> > EnvironmentVariablesMap;
    typedef std::tr1::unordered_map<QualifiedPackageName,
            std::list<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::tr1::shared_ptr<const RepositoryMaskInfo> > >,
            Hash<QualifiedPackageName> > PackageMaskMap;
}

namespace paludis
{
    template<>
    struct Implementation<ExheresProfile>
    {
        const Environment * const env;
        const ERepository * const repository;

        PaludisLikeOptionsConf options_conf;
        EnvironmentVariablesMap environment_variables;
        PackageMaskMap package_mask;

        ProfileFile<MaskFile> package_mask_file;
        ProfileFile<LineConfigFile> packages_file;

        const std::tr1::shared_ptr<Set<std::string> > use_expand;
        const std::tr1::shared_ptr<Set<std::string> > use_expand_hidden;
        const std::tr1::shared_ptr<Set<std::string> > use_expand_unprefixed;
        const std::tr1::shared_ptr<Set<std::string> > use_expand_implicit;
        const std::tr1::shared_ptr<Set<std::string> > iuse_implicit;
        const std::tr1::shared_ptr<Set<std::string> > use_expand_values;

        const std::tr1::shared_ptr<SetSpecTree> system_packages;
        const std::tr1::shared_ptr<GeneralSetDepTag> system_tag;

        Implementation(const Environment * const e, const ERepository * const p,
                const RepositoryName & name, const FSEntrySequence &,
                const std::string &, const bool) :
            env(e),
            repository(p),
            options_conf(make_named_values<PaludisLikeOptionsConfParams>(
                        value_for<n::allow_locking>(true),
                        value_for<n::environment>(e),
                        value_for<n::make_config_file>(&make_config_file)
                        )),
            package_mask_file(p),
            packages_file(p),
            use_expand(new Set<std::string>),
            use_expand_hidden(new Set<std::string>),
            use_expand_unprefixed(new Set<std::string>),
            use_expand_implicit(new Set<std::string>),
            iuse_implicit(new Set<std::string>),
            use_expand_values(new Set<std::string>),
            system_packages(new SetSpecTree(make_shared_ptr(new AllDepSpec))),
            system_tag(new GeneralSetDepTag(SetName("system"), stringify(name)))
        {
            environment_variables["CONFIG_PROTECT"] = getenv_with_default("CONFIG_PROTECT", "/etc");
            environment_variables["CONFIG_PROTECT_MASK"] = getenv_with_default("CONFIG_PROTECT_MASK", "");
        }
    };
}

ExheresProfile::ExheresProfile(
        const Environment * const env, const ERepository * const p, const RepositoryName & name,
        const FSEntrySequence & location,
        const std::string & arch_var_if_special, const bool x) :
    PrivateImplementationPattern<ExheresProfile>(
            new Implementation<ExheresProfile>(env, p, name, location, arch_var_if_special, x))
{
    for (FSEntrySequence::ConstIterator l(location.begin()), l_end(location.end()) ;
            l != l_end ; ++l)
        _load_dir(*l);

    const std::tr1::shared_ptr<const Set<UnprefixedChoiceName> > s(_imp->options_conf.known_choice_value_names(
                make_null_shared_ptr(), ChoicePrefixName("suboptions")));
    std::transform(s->begin(), s->end(), _imp->use_expand->inserter(),
            paludis::stringify<UnprefixedChoiceName>);

    const std::tr1::shared_ptr<const Set<UnprefixedChoiceName> > sh(_imp->options_conf.known_choice_value_names(
                make_null_shared_ptr(), ChoicePrefixName("hidden_suboptions")));
    std::transform(sh->begin(), sh->end(), _imp->use_expand_hidden->inserter(),
            paludis::stringify<UnprefixedChoiceName>);

    if (! _imp->repository->params().master_repositories())
        for (ProfileFile<LineConfigFile>::ConstIterator i(_imp->packages_file.begin()),
                i_end(_imp->packages_file.end()) ; i != i_end ; ++i)
        {
            if (0 != i->second.compare(0, 1, "*", 0, 1))
                continue;

            Context context_spec("When parsing '" + i->second + "':");
            std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(
                        parse_elike_package_dep_spec(i->second.substr(1),
                            i->first->supported()->package_dep_spec_parse_options(),
                            i->first->supported()->version_spec_options(),
                            std::tr1::shared_ptr<const PackageID>())));

            spec->set_tag(_imp->system_tag);
            _imp->system_packages->root()->append(spec);
        }
}

ExheresProfile::~ExheresProfile()
{
}

void
ExheresProfile::_load_dir(const FSEntry & f)
{
    if (! f.is_directory_or_symlink_to_directory())
    {
        Log::get_instance()->message("e.exheres_profile.not_a_directory", ll_warning, lc_context) <<
            "Profile component '" << f << "' is not a directory";
        return;
    }

    if ((f / "options.conf").exists())
        _imp->options_conf.add_file(f / "options.conf");

    if ((f / "package_mask.conf").exists())
        _imp->package_mask_file.add_file(f / "package_mask.conf");

    if ((f / "packages").exists())
        _imp->packages_file.add_file(f / "packages");

    if ((f / "make.defaults").exists())
    {
        const std::tr1::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(
                    _imp->repository->eapi_for_file(f / "make.defaults")));
        if (! eapi->supported())
            throw ERepositoryConfigurationError("Can't use profile directory '" + stringify(f) +
                    "' because it uses an unsupported EAPI");

        KeyValueConfigFile file(f / "make.defaults", KeyValueConfigFileOptions() +
                kvcfo_disallow_source + kvcfo_disallow_space_inside_unquoted_values +
                kvcfo_allow_inline_comments + kvcfo_allow_multiple_assigns_per_line,
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

        for (KeyValueConfigFile::ConstIterator k(file.begin()), k_end(file.end()) ;
                k != k_end ; ++k)
            _imp->environment_variables[k->first] = k->second;
    }

    if ((f / "parents.conf").exists())
    {
        LineConfigFile file(f / "parents.conf", LineConfigFileOptions());
        for (LineConfigFile::ConstIterator line(file.begin()), line_end(file.end()) ;
                line != line_end ; ++line)
            _load_dir((f / *line).realpath());
    }
}

bool
ExheresProfile::use_masked(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_unprefixed,
        const ChoiceNameWithPrefix &
        ) const
{
    std::pair<Tribool, bool> enabled_locked(_imp->options_conf.want_choice_enabled_locked(
                id, choice->prefix(), value_unprefixed));
    return enabled_locked.first.is_true() && enabled_locked.second;
}

bool
ExheresProfile::use_forced(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_unprefixed,
        const ChoiceNameWithPrefix &
        ) const
{
    std::pair<Tribool, bool> enabled_locked(_imp->options_conf.want_choice_enabled_locked(
                id, choice->prefix(), value_unprefixed));
    return enabled_locked.first.is_false() && enabled_locked.second;
}

Tribool
ExheresProfile::use_state_ignoring_masks(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_unprefixed,
        const ChoiceNameWithPrefix &
        ) const
{
    std::pair<Tribool, bool> enabled_locked(_imp->options_conf.want_choice_enabled_locked(
                id, choice->prefix(), value_unprefixed));
    return enabled_locked.first;
}

const std::tr1::shared_ptr<const Set<UnprefixedChoiceName> >
ExheresProfile::known_choice_value_names(
        const std::tr1::shared_ptr<const erepository::ERepositoryID> & id,
        const std::tr1::shared_ptr<const Choice> & choice
        ) const
{
    return _imp->options_conf.known_choice_value_names(id, choice->prefix());
}

const std::tr1::shared_ptr<const Set<std::string> >
ExheresProfile::use_expand() const
{
    return _imp->use_expand;
}

const std::tr1::shared_ptr<const Set<std::string> >
ExheresProfile::use_expand_hidden() const
{
    return _imp->use_expand_hidden;
}

const std::tr1::shared_ptr<const Set<std::string> >
ExheresProfile::use_expand_unprefixed() const
{
    return _imp->use_expand_unprefixed;
}

const std::tr1::shared_ptr<const Set<std::string> >
ExheresProfile::use_expand_implicit() const
{
    return _imp->use_expand_implicit;
}

const std::tr1::shared_ptr<const Set<std::string> >
ExheresProfile::iuse_implicit() const
{
    return _imp->iuse_implicit;
}

const std::tr1::shared_ptr<const Set<std::string> >
ExheresProfile::use_expand_values(const std::string &) const
{
    return _imp->use_expand_values;
}

const std::string
ExheresProfile::environment_variable(const std::string & s) const
{
    EnvironmentVariablesMap::const_iterator i(_imp->environment_variables.find(s));
    if (_imp->environment_variables.end() == i)
    {
        Log::get_instance()->message("e.exheres_profile.unknown", ll_warning, lc_context) <<
            "Something is asking for environment variable '" << s << "' from profiles, but that isn't in "
            "our list of special vars. This is probably a bug.";
        return "";
    }
    else
        return i->second;
}

const std::tr1::shared_ptr<const RepositoryMaskInfo>
ExheresProfile::profile_masked(const PackageID & id) const
{
    PackageMaskMap::const_iterator rr(_imp->package_mask.find(id.name()));
    if (_imp->package_mask.end() == rr)
        return std::tr1::shared_ptr<const RepositoryMaskInfo>();
    else
    {
        for (std::list<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::tr1::shared_ptr<const RepositoryMaskInfo> > >::const_iterator k(rr->second.begin()),
                k_end(rr->second.end()) ; k != k_end ; ++k)
            if (match_package(*_imp->env, *k->first, id, MatchPackageOptions()))
                return k->second;
    }

    return std::tr1::shared_ptr<const RepositoryMaskInfo>();
}

const std::tr1::shared_ptr<const SetSpecTree>
ExheresProfile::system_packages() const
{
    return _imp->system_packages;
}

const std::tr1::shared_ptr<const Map<QualifiedPackageName, PackageDepSpec> >
ExheresProfile::virtuals() const
{
    return make_shared_ptr(new Map<QualifiedPackageName, PackageDepSpec>);
}

