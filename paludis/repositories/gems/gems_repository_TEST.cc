/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/gems/gems_repository.hh>
#include <paludis/repositories/gems/params.hh>
#include <paludis/package_database.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_named_values.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct CreationTest : TestCase
    {
        CreationTest() : TestCase("creation") { }

        void run()
        {
            TestEnvironment env;
            env.package_database()->add_repository(1, make_shared_ptr(new GemsRepository(
                            make_named_values<gems::RepositoryParams>(
                                value_for<n::builddir>(FSEntry("gems_repository_TEST_dir/build")),
                                value_for<n::environment>(&env),
                                value_for<n::install_dir>(FSEntry("gems_repository_TEST_dir/install")),
                                value_for<n::location>(FSEntry("gems_repository_TEST_dir/repo")),
                                value_for<n::sync>(""),
                                value_for<n::sync_options>("")
                            ))));
        }
    } test_creation;
}

