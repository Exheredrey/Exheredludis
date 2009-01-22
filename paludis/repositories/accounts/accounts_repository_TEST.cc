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

#include <paludis/repositories/accounts/accounts_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/package_id.hh>
#include <paludis/package_database.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <tr1/memory>

using namespace paludis;
using namespace paludis::accounts_repository;
using namespace test;

namespace test_cases
{
    struct AccountsRepositoryCreationTest : TestCase
    {
        AccountsRepositoryCreationTest() : TestCase("creation") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<AccountsRepository> repo(new AccountsRepository(
                        make_named_values<AccountsRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::name>(RepositoryName("accounts"))
                        )));
            env.package_database()->add_repository(1, repo);
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "accounts");
        }
    } test_creation;
}

