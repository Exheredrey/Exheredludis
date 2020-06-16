/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVED_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVED_HH 1

#include <paludis/resolver/resolved-fwd.hh>
#include <paludis/resolver/decisions-fwd.hh>
#include <paludis/resolver/resolutions_by_resolvent-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/job_lists-fwd.hh>
#include <paludis/resolver/nag-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/serialise-fwd.hh>
#include <memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_job_lists> job_lists;
        typedef Name<struct name_nag> nag;
        typedef Name<struct name_resolutions_by_resolvent> resolutions_by_resolvent;
        typedef Name<struct name_taken_change_or_remove_decisions> taken_change_or_remove_decisions;
        typedef Name<struct name_taken_unable_to_make_decisions> taken_unable_to_make_decisions;
        typedef Name<struct name_taken_unconfirmed_decisions> taken_unconfirmed_decisions;
        typedef Name<struct name_taken_unorderable_decisions> taken_unorderable_decisions;
        typedef Name<struct name_untaken_change_or_remove_decisions> untaken_change_or_remove_decisions;
        typedef Name<struct name_untaken_unable_to_make_decisions> untaken_unable_to_make_decisions;
    }

    namespace resolver
    {
        struct Resolved
        {
            NamedValue<n::job_lists, std::shared_ptr<JobLists> > job_lists;
            NamedValue<n::nag, std::shared_ptr<NAG> > nag;
            NamedValue<n::resolutions_by_resolvent, std::shared_ptr<ResolutionsByResolvent> > resolutions_by_resolvent;
            NamedValue<n::taken_change_or_remove_decisions, std::shared_ptr<OrderedChangeOrRemoveDecisions> > taken_change_or_remove_decisions;
            NamedValue<n::taken_unable_to_make_decisions, std::shared_ptr<Decisions<UnableToMakeDecision> > > taken_unable_to_make_decisions;
            NamedValue<n::taken_unconfirmed_decisions, std::shared_ptr<Decisions<ConfirmableDecision> > > taken_unconfirmed_decisions;
            NamedValue<n::taken_unorderable_decisions, std::shared_ptr<OrderedChangeOrRemoveDecisions> > taken_unorderable_decisions;
            NamedValue<n::untaken_change_or_remove_decisions, std::shared_ptr<Decisions<ChangeOrRemoveDecision> > > untaken_change_or_remove_decisions;
            NamedValue<n::untaken_unable_to_make_decisions, std::shared_ptr<Decisions<UnableToMakeDecision> > > untaken_unable_to_make_decisions;

            static const Resolved deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
            void serialise(Serialiser &) const;
        };
    }
}

#endif
