/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/match_package.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/package_dep_spec_requirement.hh>
#include <paludis/contents.hh>
#include <paludis/version_operator.hh>

#include <paludis/util/set.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/join.hh>

#include <functional>
#include <algorithm>
#include <istream>
#include <ostream>
#include <list>

using namespace paludis;

#include <paludis/match_package-se.cc>

namespace
{
    struct RequirementChecker
    {
        const Environment & env;
        const std::shared_ptr<const PackageID> & id;
        const MatchPackageOptions & options;

        bool version_requirements_ok;
        std::list<const ChoiceRequirement *> defer_choice_requirements;

        RequirementChecker(
                const Environment & e,
                const std::shared_ptr<const PackageID> & i,
                const MatchPackageOptions & o) :
            env(e),
            id(i),
            options(o),
            version_requirements_ok(true)
        {
        }

        bool visit(const NameRequirement & r)
        {
            return r.name() == id->name();
        }

        bool visit(const PackageNamePartRequirement & r)
        {
            return r.name_part() == id->name().package();
        }

        bool visit(const CategoryNamePartRequirement & r)
        {
            return r.name_part() == id->name().category();
        }

        bool visit(const VersionRequirement & r)
        {
            bool one(r.version_operator().as_version_spec_comparator()(id->version(), r.version_spec()));

            switch (r.combiner())
            {
                case vrc_and:   version_requirements_ok &= one; break;
                case vrc_or:    version_requirements_ok |= one; break;
                case last_vrc:  throw InternalError(PALUDIS_HERE, "Bad vrc");
            }

            return true;
        }

        bool visit(const InRepositoryRequirement & r)
        {
            return r.name() == id->repository_name();
        }

        bool visit(const FromRepositoryRequirement & r)
        {
            if (! id->from_repositories_key())
                return false;

            auto v(id->from_repositories_key()->parse_value());
            return v->end() != v->find(stringify(r.name()));
        }

        bool visit(const InstalledAtPathRequirement & r)
        {
            auto repo(env.fetch_repository(id->repository_name()));
            if (! repo->installed_root_key())
                return false;
            if (repo->installed_root_key()->parse_value() != r.path())
                return false;
            return true;
        }

        bool visit(const InstallableToRepositoryRequirement & r)
        {
            if (! id->supports_action(SupportsActionTest<InstallAction>()))
                return false;
            if (! r.include_masked())
                if (id->masked())
                    return false;

            const std::shared_ptr<const Repository> dest(env.fetch_repository(r.name()));
            if (! dest->destination_interface())
                return false;
            if (! dest->destination_interface()->is_suitable_destination_for(id))
                return false;

            return true;
        }

        bool visit(const InstallableToPathRequirement & r)
        {
            if (! id->supports_action(SupportsActionTest<InstallAction>()))
                return false;
            if (! r.include_masked())
                if (id->masked())
                    return false;

            bool ok(false);
            for (auto d(env.begin_repositories()), d_end(env.end_repositories()) ;
                    d != d_end ; ++d)
            {
                if (! (*d)->destination_interface())
                    continue;
                if (! (*d)->installed_root_key())
                    continue;
                if ((*d)->installed_root_key()->parse_value() != r.path())
                    continue;
                if (! (*d)->destination_interface()->is_suitable_destination_for(id))
                    continue;

                ok = true;
                break;
            }

            if (! ok)
                return false;

            return true;
        }

        bool visit(const ExactSlotRequirement & r)
        {
            if ((! id->slot_key()) || (id->slot_key()->parse_value() != r.name()))
                return false;

            return true;
        }

        bool visit(const AnySlotRequirement &)
        {
            /* don't care */
            return true;
        }

        bool visit(const ChoiceRequirement & r)
        {
            if (! options[mpo_ignore_choice_requirements])
                defer_choice_requirements.push_back(&r);

            return true;
        }

        bool visit(const KeyRequirement & r)
        {
            if (! r.matches(&env, id))
                return false;

            return true;
        }
    };
}

bool
paludis::match_package_with_maybe_changes(
        const Environment & env,
        const PackageDepSpec & spec,
        const ChangedChoices * const maybe_changes_to_owner,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const PackageID> & from_id,
        const ChangedChoices * const maybe_changes_to_target,
        const MatchPackageOptions & options)
{
    RequirementChecker c{env, id, options};

    for (auto r(spec.requirements()->begin()), r_end(spec.requirements()->end()) ;
            r != r_end ; ++r)
        if (! (*r)->accept_returning<bool>(c))
            return false;

    if (! c.version_requirements_ok)
        return false;


    for (auto r(c.defer_choice_requirements.begin()), r_end(c.defer_choice_requirements.end()) ;
            r != r_end ; ++r)
        if (! (*r)->requirement_met(&env, maybe_changes_to_owner, id, from_id, maybe_changes_to_target).first)
            return false;

    return true;
}

bool
paludis::match_package(
        const Environment & env,
        const PackageDepSpec & spec,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const PackageID> & from_id,
        const MatchPackageOptions & options)
{
    return match_package_with_maybe_changes(env, spec, 0, id, from_id, 0, options);
}

bool
paludis::match_package_in_set(
        const Environment & env,
        const SetSpecTree & target,
        const std::shared_ptr<const PackageID> & id,
        const MatchPackageOptions & options)
{
    using namespace std::placeholders;

    DepSpecFlattener<SetSpecTree, PackageDepSpec> f(&env, id);
    target.top()->accept(f);
    return indirect_iterator(f.end()) != std::find_if(
            indirect_iterator(f.begin()), indirect_iterator(f.end()),
            std::bind(&match_package, std::cref(env), _1, std::cref(id), make_null_shared_ptr(), std::cref(options)));
}

