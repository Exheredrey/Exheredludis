/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/resolver/reason.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/serialise-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

Reason::~Reason()
{
}

void
TargetReason::serialise(Serialiser & s) const
{
    s.object("TargetReason")
        ;
}

namespace paludis
{
    template <>
    struct Implementation<DependencyReason>
    {
        const std::tr1::shared_ptr<const PackageID> from_id;
        const Resolvent from_resolvent;
        const SanitisedDependency dep;

        Implementation(const std::tr1::shared_ptr<const PackageID> & i,
                const Resolvent & r, const SanitisedDependency & d) :
            from_id(i),
            from_resolvent(r),
            dep(d)
        {
        }
    };
}

DependencyReason::DependencyReason(const std::tr1::shared_ptr<const PackageID> & i,
        const Resolvent & r,
        const SanitisedDependency & d) :
    PrivateImplementationPattern<DependencyReason>(new Implementation<DependencyReason>(i, r, d))
{
}

DependencyReason::~DependencyReason()
{
}

const std::tr1::shared_ptr<const PackageID>
DependencyReason::from_id() const
{
    return _imp->from_id;
}

const Resolvent
DependencyReason::from_resolvent() const
{
    return _imp->from_resolvent;
}

const SanitisedDependency &
DependencyReason::sanitised_dependency() const
{
    return _imp->dep;
}

void
DependencyReason::serialise(Serialiser & s) const
{
    s.object("DependencyReason")
        .member(SerialiserFlags<serialise::might_be_null>(), "from_id", from_id())
        .member(SerialiserFlags<>(), "from_resolvent", from_resolvent())
        .member(SerialiserFlags<>(), "sanitised_dependency", sanitised_dependency())
        ;
}

void
PresetReason::serialise(Serialiser & s) const
{
    s.object("PresetReason")
        ;
}

namespace paludis
{
    template <>
    struct Implementation<SetReason>
    {
        const SetName set_name;
        const std::tr1::shared_ptr<const Reason> reason_for_set;

        Implementation(const SetName & s, const std::tr1::shared_ptr<const Reason> & r) :
            set_name(s),
            reason_for_set(r)
        {
        }
    };
}

SetReason::SetReason(const SetName & s, const std::tr1::shared_ptr<const Reason> & r) :
    PrivateImplementationPattern<SetReason>(new Implementation<SetReason>(s, r))
{
}

SetReason::~SetReason()
{
}

const SetName
SetReason::set_name() const
{
    return _imp->set_name;
}

const std::tr1::shared_ptr<const Reason>
SetReason::reason_for_set() const
{
    return _imp->reason_for_set;
}

void
SetReason::serialise(Serialiser & s) const
{
    s.object("SetReason")
        .member(SerialiserFlags<serialise::might_be_null>(), "reason_for_set", reason_for_set())
        .member(SerialiserFlags<>(), "set_name", stringify(set_name()))
        ;
}

const std::tr1::shared_ptr<Reason>
Reason::deserialise(Deserialisation & d)
{
    if (d.class_name() == "TargetReason")
    {
        Deserialisator v(d, "TargetReason");
        return make_shared_ptr(new TargetReason);
    }
    else if (d.class_name() == "PresetReason")
    {
        Deserialisator v(d, "PresetReason");
        return make_shared_ptr(new PresetReason);
    }
    else if (d.class_name() == "SetReason")
    {
        Deserialisator v(d, "SetReason");
        return make_shared_ptr(new SetReason(
                    SetName(v.member<std::string>("set_name")),
                    v.member<std::tr1::shared_ptr<Reason> >("reason_for_set")
                    ));
    }
    else if (d.class_name() == "DependencyReason")
    {
        Deserialisator v(d, "DependencyReason");
        const std::tr1::shared_ptr<const PackageID> from_id(v.member<std::tr1::shared_ptr<const PackageID> >("from_id"));
        return make_shared_ptr(new DependencyReason(
                    from_id,
                    v.member<Resolvent>("from_resolvent"),
                    SanitisedDependency::deserialise(*v.find_remove_member("sanitised_dependency"), from_id))
                );
    }
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");
}

template class PrivateImplementationPattern<DependencyReason>;
template class PrivateImplementationPattern<SetReason>;

