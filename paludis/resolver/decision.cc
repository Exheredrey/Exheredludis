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

#include <paludis/resolver/decision.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/unsuitable_candidates.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/serialise-impl.hh>
#include <sstream>

using namespace paludis;
using namespace paludis::resolver;

#ifdef PALUDIS_HAVE_DEFAULT_DELETED
Decision::~Decision() = default;
#else
Decision::~Decision()
{
}
#endif

const std::tr1::shared_ptr<Decision>
Decision::deserialise(Deserialisation & d)
{
    if (d.class_name() == "NothingNoChangeDecision")
    {
        Deserialisator v(d, "NothingNoChangeDecision");
        return make_shared_ptr(new NothingNoChangeDecision(
                    v.member<bool>("taken")
                    ));
    }
    else if (d.class_name() == "ExistingNoChangeDecision")
    {
        Deserialisator v(d, "ExistingNoChangeDecision");
        return make_shared_ptr(new ExistingNoChangeDecision(
                    v.member<std::tr1::shared_ptr<const PackageID> >("existing_id"),
                    v.member<bool>("is_same"),
                    v.member<bool>("is_same_version"),
                    v.member<bool>("is_transient"),
                    v.member<bool>("taken")
                    ));
    }
    else if (d.class_name() == "ChangesToMakeDecision")
    {
        Deserialisator v(d, "ChangesToMakeDecision");
        return make_shared_ptr(new ChangesToMakeDecision(
                    v.member<std::tr1::shared_ptr<const PackageID> >("origin_id"),
                    v.member<bool>("best"),
                    v.member<bool>("taken"),
                    v.member<std::tr1::shared_ptr<const Destination> >("destination")
                    ));
    }
    else if (d.class_name() == "UnableToMakeDecision")
    {
        Deserialisator v(d, "UnableToMakeDecision");

        std::tr1::shared_ptr<UnsuitableCandidates> unsuitable_candidates(new UnsuitableCandidates);
        Deserialisator vv(*v.find_remove_member("unsuitable_candidates"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            unsuitable_candidates->push_back(vv.member<UnsuitableCandidate>(stringify(n)));

        return make_shared_ptr(new UnableToMakeDecision(
                    unsuitable_candidates,
                    v.member<bool>("taken")
                    ));
    }
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");
}

const std::tr1::shared_ptr<ChangesToMakeDecision>
ChangesToMakeDecision::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "ChangesToMakeDecision");
    return make_shared_ptr(new ChangesToMakeDecision(
                v.member<std::tr1::shared_ptr<const PackageID> >("origin_id"),
                v.member<bool>("best"),
                v.member<bool>("taken"),
                v.member<std::tr1::shared_ptr<const Destination> >("destination")
                ));
}

namespace paludis
{
    template <>
    struct Implementation<NothingNoChangeDecision>
    {
        const bool taken;

        Implementation(const bool t) :
            taken(t)
        {
        }
    };
}

NothingNoChangeDecision::NothingNoChangeDecision(const bool t) :
    PrivateImplementationPattern<NothingNoChangeDecision>(new Implementation<NothingNoChangeDecision>(t))
{
}

#ifdef PALUDIS_HAVE_DEFAULT_DELETED
NothingNoChangeDecision::~NothingNoChangeDecision() = default;
#else
NothingNoChangeDecision::~NothingNoChangeDecision()
{
}
#endif

bool
NothingNoChangeDecision::taken() const
{
    return _imp->taken;
}

void
NothingNoChangeDecision::serialise(Serialiser & s) const
{
    s.object("NothingNoChangeDecision")
        .member(SerialiserFlags<>(), "taken", taken())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<ExistingNoChangeDecision>
    {
        const std::tr1::shared_ptr<const PackageID> existing_id;
        const bool is_same;
        const bool is_same_version;
        const bool is_transient;
        const bool taken;

        Implementation(const std::tr1::shared_ptr<const PackageID> & e,
                const bool s, const bool v, const bool r, const bool t) :
            existing_id(e),
            is_same(s),
            is_same_version(v),
            is_transient(r),
            taken(t)
        {
        }
    };
}

ExistingNoChangeDecision::ExistingNoChangeDecision(const std::tr1::shared_ptr<const PackageID> & e,
        const bool s, const bool v, const bool r, const bool t) :
    PrivateImplementationPattern<ExistingNoChangeDecision>(new Implementation<ExistingNoChangeDecision>(
                e, s, v, r, t))
{
}

#ifdef PALUDIS_HAVE_DEFAULT_DELETED
ExistingNoChangeDecision::~ExistingNoChangeDecision() = default;
#else
ExistingNoChangeDecision::~ExistingNoChangeDecision()
{
}
#endif

const std::tr1::shared_ptr<const PackageID>
ExistingNoChangeDecision::existing_id() const
{
    return _imp->existing_id;
}

bool
ExistingNoChangeDecision::is_same() const
{
    return _imp->is_same;
}

bool
ExistingNoChangeDecision::is_same_version() const
{
    return _imp->is_same_version;
}

bool
ExistingNoChangeDecision::is_transient() const
{
    return _imp->is_transient;
}

bool
ExistingNoChangeDecision::taken() const
{
    return _imp->taken;
}

void
ExistingNoChangeDecision::serialise(Serialiser & s) const
{
    s.object("ExistingNoChangeDecision")
        .member(SerialiserFlags<serialise::might_be_null>(), "existing_id", existing_id())
        .member(SerialiserFlags<>(), "is_same", is_same())
        .member(SerialiserFlags<>(), "is_same_version", is_same_version())
        .member(SerialiserFlags<>(), "is_transient", is_transient())
        .member(SerialiserFlags<>(), "taken", taken())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<ChangesToMakeDecision>
    {
        const std::tr1::shared_ptr<const PackageID> origin_id;
        const bool best;
        const bool taken;
        std::tr1::shared_ptr<const Destination> destination;

        Implementation(
                const std::tr1::shared_ptr<const PackageID> & o,
                const bool b,
                const bool t,
                const std::tr1::shared_ptr<const Destination> & d) :
            origin_id(o),
            best(b),
            taken(t),
            destination(d)
        {
        }
    };
}

ChangesToMakeDecision::ChangesToMakeDecision(
        const std::tr1::shared_ptr<const PackageID> & o,
        const bool b,
        const bool t,
        const std::tr1::shared_ptr<const Destination> & d) :
    PrivateImplementationPattern<ChangesToMakeDecision>(new Implementation<ChangesToMakeDecision>(o, b, t, d))
{
}

#ifdef PALUDIS_HAVE_DEFAULT_DELETED
ChangesToMakeDecision::~ChangesToMakeDecision() = default;
#else
ChangesToMakeDecision::~ChangesToMakeDecision()
{
}
#endif

const std::tr1::shared_ptr<const Destination>
ChangesToMakeDecision::destination() const
{
    return _imp->destination;
}

void
ChangesToMakeDecision::set_destination(const std::tr1::shared_ptr<const Destination> & d)
{
    _imp->destination = d;
}

const std::tr1::shared_ptr<const PackageID>
ChangesToMakeDecision::origin_id() const
{
    return _imp->origin_id;
}

bool
ChangesToMakeDecision::best() const
{
    return _imp->best;
}

bool
ChangesToMakeDecision::taken() const
{
    return _imp->taken;
}

void
ChangesToMakeDecision::serialise(Serialiser & s) const
{
    s.object("ChangesToMakeDecision")
        .member(SerialiserFlags<serialise::might_be_null>(), "origin_id", origin_id())
        .member(SerialiserFlags<>(), "best", best())
        .member(SerialiserFlags<serialise::might_be_null>(), "destination", destination())
        .member(SerialiserFlags<>(), "taken", taken())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<UnableToMakeDecision>
    {
        const std::tr1::shared_ptr<const UnsuitableCandidates> unsuitable_candidates;
        const bool taken;

        Implementation(const std::tr1::shared_ptr<const UnsuitableCandidates> & u, const bool t) :
            unsuitable_candidates(u),
            taken(t)
        {
        }
    };
}

UnableToMakeDecision::UnableToMakeDecision(
        const std::tr1::shared_ptr<const UnsuitableCandidates> & u,
        const bool t) :
    PrivateImplementationPattern<UnableToMakeDecision>(new Implementation<UnableToMakeDecision>(u, t))
{
}

#ifdef PALUDIS_HAVE_DEFAULT_DELETED
UnableToMakeDecision::~UnableToMakeDecision() = default;
#else
UnableToMakeDecision::~UnableToMakeDecision()
{
}
#endif

const std::tr1::shared_ptr<const UnsuitableCandidates>
UnableToMakeDecision::unsuitable_candidates() const
{
    return _imp->unsuitable_candidates;
}

bool
UnableToMakeDecision::taken() const
{
    return _imp->taken;
}

void
UnableToMakeDecision::serialise(Serialiser & s) const
{
    s.object("UnableToMakeDecision")
        .member(SerialiserFlags<>(), "taken", taken())
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "unsuitable_candidates", unsuitable_candidates())
        ;
}

template class PrivateImplementationPattern<NothingNoChangeDecision>;
template class PrivateImplementationPattern<ExistingNoChangeDecision>;
template class PrivateImplementationPattern<ChangesToMakeDecision>;
template class PrivateImplementationPattern<UnableToMakeDecision>;

