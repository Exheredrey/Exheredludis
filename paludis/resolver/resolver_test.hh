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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_TEST_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_TEST_HH 1

#include <paludis/resolver/qpn_s-fwd.hh>
#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/use_installed-fwd.hh>
#include <paludis/resolver/resolutions-fwd.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <test/test_framework.hh>
#include <tr1/memory>
#include <string>
#include <map>
#include <list>

namespace paludis
{
    namespace resolver
    {
        namespace resolver_test
        {
            std::string from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
                    const std::string & k);

            typedef std::map<QPN_S, std::tr1::shared_ptr<Constraints> > InitialConstraints;

            bool care_about_dep_fn(const QPN_S &, const std::tr1::shared_ptr<const Resolution> &, const SanitisedDependency &);

            const std::tr1::shared_ptr<Constraints> initial_constraints_for_fn(
                    const InitialConstraints & initial_constraints,
                    const QPN_S & qpn_s);

            std::tr1::shared_ptr<QPN_S_Sequence> get_qpn_s_s_for_fn(const PackageDepSpec & spec,
                    const std::tr1::shared_ptr<const Reason> &);

            bool is_suggestion(const SanitisedDependency & dep);

            bool take_dependency_fn(
                    const QPN_S &,
                    const SanitisedDependency & dep,
                    const std::tr1::shared_ptr<const Reason> &);

            UseInstalled get_use_installed_fn(
                    const QPN_S &,
                    const PackageDepSpec &,
                    const std::tr1::shared_ptr<const Reason> &);

            struct ResolverTestCase : test::TestCase
            {
                TestEnvironment env;
                std::tr1::shared_ptr<Repository> repo, inst_repo;
                std::tr1::shared_ptr<FakeInstalledRepository> fake_inst_repo;

                ResolverTestCase(const std::string & group, const std::string & test_name);

                const std::tr1::shared_ptr<const ResolutionLists> get_resolutions(const PackageDepSpec & target);

                const std::tr1::shared_ptr<const ResolutionLists> get_resolutions(const std::string & target);

                struct ResolutionListChecks
                {
                    typedef std::tr1::function<bool (const std::tr1::shared_ptr<const Resolution> &) > CheckFunction;
                    typedef std::tr1::function<std::string (const std::tr1::shared_ptr<const Resolution> &) > MessageFunction;
                    typedef std::list<std::pair<CheckFunction, MessageFunction> > List;
                    List checks;

                    static bool check_qpn(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Resolution> & r);

                    static std::string check_generic_msg(const std::string & q, const std::tr1::shared_ptr<const Resolution> & r);

                    static std::string check_qpn_msg(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Resolution> & r);

                    static bool check_finished(const std::tr1::shared_ptr<const Resolution> & r);

                    static std::string check_finished_msg(const std::tr1::shared_ptr<const Resolution> & r);

                    ResolutionListChecks & qpn(const QualifiedPackageName & q);

                    ResolutionListChecks & finished();
                };

                void check_resolution_list(const std::tr1::shared_ptr<const Resolutions> & list, const ResolutionListChecks & checks);

                void install(const std::string & c, const std::string & p, const std::string & v);
            };
        }
    }
}

#endif
