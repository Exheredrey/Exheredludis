/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_HH 1

#include <paludis/resolver/resolver-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/resolver_functions-fwd.hh>
#include <paludis/resolver/decider-fwd.hh>
#include <paludis/resolver/resolved-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/resolver/package_or_block_dep_spec-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/name.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/spec_tree-fwd.hh>
#include <paludis/filtered_generator-fwd.hh>
#include <paludis/generator-fwd.hh>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE Resolver
        {
            private:
                Pimp<Resolver> _imp;

            public:
                Resolver(
                        const Environment * const,
                        const ResolverFunctions &);
                ~Resolver();

                void add_target(const PackageOrBlockDepSpec &, const std::string & extra_information);
                void add_target(const SetName &, const std::string & extra_information);
                void purge();

                void resolve();

                const std::shared_ptr<const Resolved> resolved() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
