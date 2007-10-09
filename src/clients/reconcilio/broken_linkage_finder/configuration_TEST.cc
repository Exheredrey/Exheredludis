/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton <levertond@googlemail.com>
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

#include "configuration.hh"

#include <paludis/util/fs_entry.hh>
#include <paludis/util/join.hh>

#include <test/test_runner.hh>
#include <test/test_framework.hh>

#include <libwrapiter/libwrapiter_forward_iterator-impl.hh>

#include <unistd.h>

using namespace broken_linkage_finder;

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct ConfigurationTest : TestCase
    {
        ConfigurationTest() : TestCase("configuration") {}

        void run()
        {
            setenv("SEARCH_DIRS", "/quuxlib", 1);
            setenv("SEARCH_DIRS_MASK", "/quuxlib/quux", 1);
            setenv("LD_LIBRARY_MASK", "libquux.so", 1);

            Configuration config(FSEntry::cwd() / "configuration_TEST_dir");

            TEST_CHECK_EQUAL(join(config.begin_search_dirs(), config.end_search_dirs(), " "),
                             "/alib /barbin /barlib/foo /bazbin /bin /blib /foobin /foolib/bar /lib32 /lib64 /quuxlib /sbin /usr/bin /usr/lib* /usr/sbin");

            TEST_CHECK(config.dir_is_masked(FSEntry("/meh")));
            TEST_CHECK(config.dir_is_masked(FSEntry("/quuxlib/quux")));
            TEST_CHECK(! config.dir_is_masked(FSEntry("/feh")));
            TEST_CHECK(! config.dir_is_masked(FSEntry("/opt/OpenOffice")));
            TEST_CHECK(! config.dir_is_masked(FSEntry("/usr/lib/openoffice")));
            TEST_CHECK(! config.dir_is_masked(FSEntry("/foo")));

            TEST_CHECK(config.lib_is_masked("libquux.so"));
            TEST_CHECK(config.lib_is_masked("libxyzzy.so"));
            TEST_CHECK(config.lib_is_masked("libodbcinst.so"));
            TEST_CHECK(config.lib_is_masked("libodbc.so"));
            TEST_CHECK(config.lib_is_masked("libjava.so"));
            TEST_CHECK(config.lib_is_masked("libjvm.so"));
            TEST_CHECK(! config.lib_is_masked("libfoo.so"));
        }
    } configuration_test;
}

