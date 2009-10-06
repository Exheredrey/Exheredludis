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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_REASON_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_REASON_HH 1

#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/type_list.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/serialise-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace resolver
    {
        class Reason :
            public virtual DeclareAbstractAcceptMethods<Reason, MakeTypeList<
                TargetReason, DependencyReason, PresetReason, SetReason>::Type>
        {
            public:
                virtual ~Reason() = 0;

                virtual void serialise(Serialiser &) const = 0;

                static const std::tr1::shared_ptr<Reason> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class TargetReason :
            public Reason,
            public ImplementAcceptMethods<Reason, TargetReason>
        {
            public:
                virtual void serialise(Serialiser &) const;
        };

        class DependencyReason :
            private PrivateImplementationPattern<DependencyReason>,
            public Reason,
            public ImplementAcceptMethods<Reason, DependencyReason>
        {
            public:
                DependencyReason(
                        const std::tr1::shared_ptr<const PackageID> & id,
                        const Resolvent &,
                        const SanitisedDependency & s);

                ~DependencyReason();

                const std::tr1::shared_ptr<const PackageID> from_id() const;
                const Resolvent from_resolvent() const;
                const SanitisedDependency & sanitised_dependency() const;

                virtual void serialise(Serialiser &) const;
        };

        class PresetReason :
            public Reason,
            public ImplementAcceptMethods<Reason, PresetReason>
        {
            public:
                virtual void serialise(Serialiser &) const;
        };

        class SetReason :
            public Reason,
            private PrivateImplementationPattern<SetReason>,
            public ImplementAcceptMethods<Reason, SetReason>
        {
            public:
                SetReason(const SetName &, const std::tr1::shared_ptr<const Reason> &);
                ~SetReason();

                const SetName set_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::tr1::shared_ptr<const Reason> reason_for_set() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<resolver::DependencyReason>;
    extern template class PrivateImplementationPattern<resolver::SetReason>;
#endif

}

#endif
