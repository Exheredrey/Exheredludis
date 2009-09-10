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

#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/serialise-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <sstream>
#include <list>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<Constraints>
    {
        UseInstalled strictest_use_installed;
        bool nothing_is_fine_too;
        DestinationTypes to_destinations;
        Sequence<std::tr1::shared_ptr<const Constraint> > constraints;

        Implementation() :
            strictest_use_installed(ui_if_possible),
            nothing_is_fine_too(true)
        {
        }
    };
}

Constraints::Constraints() :
    PrivateImplementationPattern<Constraints>(new Implementation<Constraints>)
{
}

Constraints::~Constraints()
{
}

Constraints::ConstIterator
Constraints::begin() const
{
    return ConstIterator(_imp->constraints.begin());
}

Constraints::ConstIterator
Constraints::end() const
{
    return ConstIterator(_imp->constraints.end());
}

void
Constraints::add(const std::tr1::shared_ptr<const Constraint> & c)
{
    _imp->constraints.push_back(c);
    _imp->strictest_use_installed = std::min(_imp->strictest_use_installed, c->use_installed());
    _imp->nothing_is_fine_too = _imp->nothing_is_fine_too && c->nothing_is_fine_too();
    _imp->to_destinations |= c->to_destinations();
}

bool
Constraints::empty() const
{
    return _imp->constraints.empty();
}

UseInstalled
Constraints::strictest_use_installed() const
{
    return _imp->strictest_use_installed;
}

bool
Constraints::nothing_is_fine_too() const
{
    return _imp->nothing_is_fine_too;
}

const DestinationTypes
Constraints::to_destinations() const
{
    return _imp->to_destinations;
}

void
Constraints::serialise(Serialiser & s) const
{
    s.object("Constraints")
        .member(SerialiserFlags<serialise::container>(), "items", _imp->constraints)
        ;
}

const std::tr1::shared_ptr<Constraints>
Constraints::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Constraints");
    Deserialisator vv(*v.find_remove_member("items"), "c");
    std::tr1::shared_ptr<Constraints> result(new Constraints);
    for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
        result->add(vv.member<std::tr1::shared_ptr<Constraint> >(stringify(n)));
    return result;
}

void
Constraint::serialise(Serialiser & s) const
{
    s.object("Constraint")
        .member(SerialiserFlags<>(), "nothing_is_fine_too", nothing_is_fine_too())
        .member(SerialiserFlags<serialise::might_be_null>(), "reason", reason())
        .member(SerialiserFlags<>(), "spec", spec())
        .member(SerialiserFlags<>(), "to_destinations", to_destinations())
        .member(SerialiserFlags<>(), "use_installed", stringify(use_installed()))
        ;
}

namespace
{
    struct IDFinder
    {
        const std::tr1::shared_ptr<const PackageID> visit(const DependencyReason & r) const
        {
            return r.from_id();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const SetReason &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const PresetReason &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const TargetReason &) const
        {
            return make_null_shared_ptr();
        }
    };
}

const std::tr1::shared_ptr<Constraint>
Constraint::deserialise(Deserialisation & d)
{
    Context context("When deserialising '" + d.raw_string() + "':");

    Deserialisator v(d, "Constraint");

    const std::tr1::shared_ptr<Reason> reason(v.member<std::tr1::shared_ptr<Reason> >("reason"));
    IDFinder id_finder;

    return make_shared_ptr(new Constraint(make_named_values<Constraint>(
                    value_for<n::nothing_is_fine_too>(v.member<bool>("nothing_is_fine_too")),
                    value_for<n::reason>(reason),
                    value_for<n::spec>(PackageOrBlockDepSpec::deserialise(*v.find_remove_member("spec"),
                            reason->accept_returning<std::tr1::shared_ptr<const PackageID> >(id_finder))),
                    value_for<n::to_destinations>(v.member<DestinationTypes>("to_destinations")),
                    value_for<n::use_installed>(destringify<UseInstalled>(v.member<std::string>("use_installed")))
            )));
}

template class PrivateImplementationPattern<Constraints>;
template class WrappedForwardIterator<Constraints::ConstIteratorTag, const std::tr1::shared_ptr<const Constraint> >;

