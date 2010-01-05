/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#include <paludis/create_output_manager_info.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/validated.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/action.hh>
#include <paludis/name.hh>
#include <ostream>

using namespace paludis;

#include <paludis/create_output_manager_info-se.cc>

namespace
{
    struct GetFlagsVisitor
    {
        std::tr1::shared_ptr<Set<std::string> > visit(const InstallAction &) const
        {
            return make_shared_ptr(new Set<std::string>);
        }

        std::tr1::shared_ptr<Set<std::string> > visit(const UninstallAction &) const
        {
            return make_shared_ptr(new Set<std::string>);
        }

        std::tr1::shared_ptr<Set<std::string> > visit(const PretendAction &) const
        {
            return make_shared_ptr(new Set<std::string>);
        }

        std::tr1::shared_ptr<Set<std::string> > visit(const FetchAction & a) const
        {
            std::tr1::shared_ptr<Set<std::string> > result(new Set<std::string>);
            if (a.options.ignore_unfetched())
                result->insert(FetchAction::ignore_unfetched_flag_name());
            return result;
        }

        std::tr1::shared_ptr<Set<std::string> > visit(const PretendFetchAction &) const
        {
            return make_shared_ptr(new Set<std::string>);
        }

        std::tr1::shared_ptr<Set<std::string> > visit(const ConfigAction &) const
        {
            return make_shared_ptr(new Set<std::string>);
        }

        std::tr1::shared_ptr<Set<std::string> > visit(const InfoAction &) const
        {
            return make_shared_ptr(new Set<std::string>);
        }
    };

    std::tr1::shared_ptr<Set<std::string> > get_flags(const Action & a)
    {
        return a.accept_returning<std::tr1::shared_ptr<Set<std::string> > >(GetFlagsVisitor());
    }
}

namespace paludis
{
    template <>
    struct Implementation<CreateOutputManagerForPackageIDActionInfo>
    {
        const std::tr1::shared_ptr<const PackageID> id;
        const std::string action_name;
        const std::tr1::shared_ptr<const Set<std::string> > action_flags;
        const OutputExclusivity output_exclusivity;

        Implementation(const std::tr1::shared_ptr<const PackageID> & i,
                const std::string & a,
                const std::tr1::shared_ptr<const Set<std::string> > & f,
                const OutputExclusivity e) :
            id(i),
            action_name(a),
            action_flags(f),
            output_exclusivity(e)
        {
        }
    };

    template <>
    struct Implementation<CreateOutputManagerForRepositorySyncInfo>
    {
        const RepositoryName repo_name;
        const OutputExclusivity output_exclusivity;

        Implementation(const RepositoryName & r, const OutputExclusivity e) :
            repo_name(r),
            output_exclusivity(e)
        {
        }
    };
}

const std::tr1::shared_ptr<CreateOutputManagerInfo>
CreateOutputManagerInfo::deserialise(Deserialisation & d)
{
    if (d.class_name() == "CreateOutputManagerForPackageIDActionInfo")
        return CreateOutputManagerForPackageIDActionInfo::deserialise(d);
    else if (d.class_name() == "CreateOutputManagerForRepositorySyncInfo")
        return CreateOutputManagerForRepositorySyncInfo::deserialise(d);
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");
}

CreateOutputManagerForPackageIDActionInfo::CreateOutputManagerForPackageIDActionInfo(
        const std::tr1::shared_ptr<const PackageID> & i,
        const Action & a,
        const OutputExclusivity e) :
    PrivateImplementationPattern<CreateOutputManagerForPackageIDActionInfo>(
            new Implementation<CreateOutputManagerForPackageIDActionInfo>(i, a.simple_name(),
                get_flags(a), e))
{
}

CreateOutputManagerForPackageIDActionInfo::CreateOutputManagerForPackageIDActionInfo(
        const std::tr1::shared_ptr<const PackageID> & i,
        const std::string & a,
        const std::tr1::shared_ptr<const Set<std::string> > & f,
        const OutputExclusivity e) :
    PrivateImplementationPattern<CreateOutputManagerForPackageIDActionInfo>(
            new Implementation<CreateOutputManagerForPackageIDActionInfo>(i, a, f, e))
{
}

CreateOutputManagerForPackageIDActionInfo::~CreateOutputManagerForPackageIDActionInfo()
{
}

const std::tr1::shared_ptr<const PackageID>
CreateOutputManagerForPackageIDActionInfo::package_id() const
{
    return _imp->id;
}

const std::string
CreateOutputManagerForPackageIDActionInfo::action_name() const
{
    return _imp->action_name;
}

const std::tr1::shared_ptr<const Set<std::string> >
CreateOutputManagerForPackageIDActionInfo::action_flags() const
{
    return _imp->action_flags;
}

OutputExclusivity
CreateOutputManagerForPackageIDActionInfo::output_exclusivity() const
{
    return _imp->output_exclusivity;
}

void
CreateOutputManagerForPackageIDActionInfo::serialise(Serialiser & s) const
{
    s.object("CreateOutputManagerForPackageIDActionInfo")
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "action_flags", action_flags())
        .member(SerialiserFlags<>(), "action_name", action_name())
        .member(SerialiserFlags<>(), "output_exclusivity", stringify(output_exclusivity()))
        .member(SerialiserFlags<serialise::might_be_null>(), "package_id", package_id())
        ;
}

const std::tr1::shared_ptr<CreateOutputManagerForPackageIDActionInfo>
CreateOutputManagerForPackageIDActionInfo::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "CreateOutputManagerForPackageIDActionInfo");

    std::tr1::shared_ptr<Set<std::string> > action_flags(new Set<std::string>);
    Deserialisator vv(*v.find_remove_member("action_flags"), "c");
    for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
        action_flags->insert(vv.member<std::string>(stringify(n)));

    return make_shared_ptr(new CreateOutputManagerForPackageIDActionInfo(
                v.member<std::tr1::shared_ptr<const PackageID> >("package_id"),
                v.member<std::string>("action_name"),
                action_flags,
                destringify<OutputExclusivity>(v.member<std::string>("output_exclusivity"))
                ));
}

CreateOutputManagerForRepositorySyncInfo::CreateOutputManagerForRepositorySyncInfo(
        const RepositoryName & r, const OutputExclusivity e) :
    PrivateImplementationPattern<CreateOutputManagerForRepositorySyncInfo>(
            new Implementation<CreateOutputManagerForRepositorySyncInfo>(r, e))
{
}

CreateOutputManagerForRepositorySyncInfo::~CreateOutputManagerForRepositorySyncInfo()
{
}

const RepositoryName
CreateOutputManagerForRepositorySyncInfo::repository_name() const
{
    return _imp->repo_name;
}

OutputExclusivity
CreateOutputManagerForRepositorySyncInfo::output_exclusivity() const
{
    return _imp->output_exclusivity;
}

void
CreateOutputManagerForRepositorySyncInfo::serialise(Serialiser & s) const
{
    s.object("CreateOutputManagerForRepositorySyncInfo")
        .member(SerialiserFlags<>(), "repository_name", stringify(repository_name()))
        .member(SerialiserFlags<>(), "output_exclusivity", stringify(output_exclusivity()))
        ;
}

const std::tr1::shared_ptr<CreateOutputManagerForRepositorySyncInfo>
CreateOutputManagerForRepositorySyncInfo::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "CreateOutputManagerForRepositorySyncInfo");
    return make_shared_ptr(new CreateOutputManagerForRepositorySyncInfo(
                RepositoryName(v.member<std::string>("repo_name")),
                destringify<OutputExclusivity>(v.member<std::string>("output_exclusivity"))
                ));
}

template class PrivateImplementationPattern<CreateOutputManagerForRepositorySyncInfo>;
template class PrivateImplementationPattern<CreateOutputManagerForPackageIDActionInfo>;

