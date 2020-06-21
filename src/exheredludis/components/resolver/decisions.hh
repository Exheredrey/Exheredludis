/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_DECISIONS_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_DECISIONS_HH 1

#include <paludis/resolver/decisions-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/orderer_notes-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/no_type.hh>
#include <paludis/serialise-fwd.hh>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        template <typename Decision_, typename Notes_>
        struct DecisionsConstIteratorTag;

        template <typename Decision_, typename Notes_>
        struct DecisionsIteratorValueType
        {
            typedef const std::pair<std::shared_ptr<const Decision_>, Notes_> Type;
        };

        template <typename Decision_>
        struct DecisionsIteratorValueType<Decision_, NoType<0u> *>
        {
            typedef const std::shared_ptr<const Decision_> Type;
        };

        template <typename Decision_, typename Notes_>
        class PALUDIS_VISIBLE Decisions
        {
            private:
                Pimp<Decisions> _imp;

            public:
                Decisions();
                ~Decisions();

                void push_back(
                        const std::shared_ptr<const Decision_> &,
                        const Notes_ & = static_cast<NoType<0u> *>(nullptr));

                typedef DecisionsConstIteratorTag<Decision_, Notes_> ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag,
                        typename DecisionsIteratorValueType<Decision_, Notes_>::Type> ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                bool empty() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void serialise(Serialiser &) const;
                static const std::shared_ptr<Decisions> deserialise(Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        extern template class PALUDIS_VISIBLE Decisions<UnableToMakeDecision>;
        extern template class PALUDIS_VISIBLE Decisions<ChangesToMakeDecision>;
        extern template class PALUDIS_VISIBLE Decisions<ChangeOrRemoveDecision>;
        extern template class PALUDIS_VISIBLE Decisions<ConfirmableDecision>;
        extern template class PALUDIS_VISIBLE Decisions<ChangeOrRemoveDecision, std::shared_ptr<const OrdererNotes> >;
    }

    extern template class PALUDIS_VISIBLE WrappedForwardIterator<resolver::Decisions<resolver::UnableToMakeDecision>::ConstIteratorTag,
           const std::shared_ptr<const resolver::UnableToMakeDecision> >;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<resolver::Decisions<resolver::ChangesToMakeDecision>::ConstIteratorTag,
           const std::shared_ptr<const resolver::ChangesToMakeDecision> >;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<resolver::Decisions<resolver::ChangeOrRemoveDecision>::ConstIteratorTag,
           const std::shared_ptr<const resolver::ChangeOrRemoveDecision> >;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<resolver::Decisions<resolver::ConfirmableDecision>::ConstIteratorTag,
           const std::shared_ptr<const resolver::ConfirmableDecision> >;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<resolver::Decisions<resolver::ChangeOrRemoveDecision,
           std::shared_ptr<const resolver::OrdererNotes> >::ConstIteratorTag,
           const std::pair<
               std::shared_ptr<const resolver::ChangeOrRemoveDecision>,
               std::shared_ptr<const resolver::OrdererNotes> > >;
}

#endif
