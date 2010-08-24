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

#include <paludis/tar_merger.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/hooker.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/set.hh>
#include <paludis/util/process.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/hook.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

#include "config.h"

using namespace paludis;
using namespace test;

namespace
{
    std::pair<uid_t, gid_t>
    get_new_ids_or_minus_one(const FSPath &)
    {
        return std::make_pair(-1, -1);
    }
}

namespace
{
    struct TestTarMerger :
        TarMerger
    {
        TestTarMerger(const TarMergerParams & p) :
            TarMerger(p)
        {
        }

        virtual void on_error(bool is_check, const std::string & s)
        {
            if (is_check)
                make_check_fail();
            else
                throw MergerError(s);
        }

        void on_warn(bool, const std::string &)
        {
        }

        void display_override(const std::string &) const
        {
        }

        void track_install_file(const FSPath &, const FSPath &)
        {
        }

        void track_install_sym(const FSPath &, const FSPath &)
        {
        }
    };
}

namespace test_cases
{
#if ENABLE_PBINS

    struct SimpleTarMergerTest : TestCase
    {
        SimpleTarMergerTest() : TestCase("simple tar merge") { }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            auto output(FSPath("tar_merger_TEST_dir") / "simple.tar");

            TestEnvironment env;
            TestTarMerger merger(make_named_values<TarMergerParams>(
                        n::compression() = tmc_none,
                        n::environment() = &env,
                        n::fix_mtimes_before() = Timestamp(0, 0),
                        n::get_new_ids_or_minus_one() = &get_new_ids_or_minus_one,
                        n::image() = FSPath("tar_merger_TEST_dir") / "simple",
                        n::install_under() = FSPath("/"),
                        n::maybe_output_manager() = make_null_shared_ptr(),
                        n::merged_entries() = std::make_shared<FSPathSet>(),
                        n::no_chown() = true,
                        n::options() = MergerOptions(),
                        n::root() = FSPath("/"),
                        n::tar_file() = output
                        ));

            TEST_CHECK(! output.stat().is_regular_file());

            merger.merge();
            output = FSPath(stringify(output));

            TEST_CHECK(output.stat().is_regular_file());
            TEST_CHECK(output.stat().file_size() > 100);

            Process untar_process(ProcessCommand({"sh", "-c", "tar xf ../simple.tar 2>&1"}));
            untar_process.chdir(FSPath("tar_merger_TEST_dir/simple_extract"));
            TEST_CHECK_EQUAL(0, untar_process.run().wait());

            TEST_CHECK((FSPath("tar_merger_TEST_dir") / "simple_extract" / "file").stat().is_regular_file());
            TEST_CHECK_EQUAL((FSPath("tar_merger_TEST_dir") / "simple_extract" / "file").stat().file_size(),
                    (FSPath("tar_merger_TEST_dir") / "simple" / "file").stat().file_size());

            TEST_CHECK((FSPath("tar_merger_TEST_dir") / "simple_extract" / "subdir" / "another").stat().is_regular_file());

            TEST_CHECK((FSPath("tar_merger_TEST_dir") / "simple_extract" / "subdir" / "subsubdir" / "script").stat().is_regular_file());
            TEST_CHECK(0 != ((FSPath("tar_merger_TEST_dir") / "simple_extract" / "subdir" / "subsubdir" / "script").stat().permissions() & S_IXUSR));

            TEST_CHECK((FSPath("tar_merger_TEST_dir") / "simple_extract" / "goodsym").stat().is_symlink());
            TEST_CHECK((FSPath("tar_merger_TEST_dir") / "simple_extract" / "goodsym").readlink() == "file");

            TEST_CHECK((FSPath("tar_merger_TEST_dir") / "simple_extract" / "badsym").stat().is_symlink());
            TEST_CHECK((FSPath("tar_merger_TEST_dir") / "simple_extract" / "badsym").readlink() == "nothing");
        }
    } test_simple_tar_merger;

#else

    struct TarMergerNotAvailable : TestCase
    {
        TarMergerNotAvailable() : TestCase("tar merger not available") { }

        void run()
        {
            auto output(FSPath("tar_merger_TEST_dir") / "simple.tar");

            TestEnvironment env;
            TestTarMerger merger(make_named_values<TarMergerParams>(
                        n::compression() = tmc_none,
                        n::environment() = &env,
                        n::fix_mtimes_before() = Timestamp(0, 0),
                        n::get_new_ids_or_minus_one() = &get_new_ids_or_minus_one,
                        n::image() = FSPath("tar_merger_TEST_dir") / "simple",
                        n::install_under() = FSPath("/"),
                        n::maybe_output_manager() = make_null_shared_ptr(),
                        n::merged_entries() = std::make_shared<FSPathSet>(),
                        n::no_chown() = true,
                        n::options() = MergerOptions(),
                        n::root() = FSPath("/"),
                        n::tar_file() = output
                        ));

            TEST_CHECK(! output.stat().is_regular_file());

            TEST_CHECK_THROWS(merger.merge(), NotAvailableError);
        }
    } test_tar_merger_not_available;

#endif

}

