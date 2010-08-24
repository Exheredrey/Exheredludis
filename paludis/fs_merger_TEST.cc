/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/fs_merger.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/hooker.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/set.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/hook.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <iterator>
#include <list>

using namespace paludis;
using namespace test;

namespace
{
    std::pair<uid_t, gid_t>
    get_new_ids_or_minus_one(const FSPath &)
    {
        return std::make_pair(-1, -1);
    }

    bool
    timestamps_nearly_equal(const Timestamp & i_set, const Timestamp & reference)
    {
        return i_set == reference ||
            (i_set.seconds() == reference.seconds() &&
             i_set.nanoseconds() % 1000 == 0 &&
             i_set.nanoseconds() / 1000 == reference.nanoseconds() / 1000);
    }
}

namespace paludis
{
    class HookTestEnvironment :
        public TestEnvironment
    {
        private:
            mutable std::shared_ptr<Hooker> hooker;
            mutable std::list<std::pair<FSPath, bool> > hook_dirs;

        public:
            HookTestEnvironment(const FSPath & hooks);

            virtual ~HookTestEnvironment();

            virtual HookResult perform_hook(const Hook &, const std::shared_ptr<OutputManager> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    HookTestEnvironment::HookTestEnvironment(const FSPath & hooks)
    {
        if (hooks.stat().is_directory())
            hook_dirs.push_back(std::make_pair(hooks, false));
    }

    HookTestEnvironment::~HookTestEnvironment()
    {
    }

    HookResult
    HookTestEnvironment::perform_hook(const Hook & hook, const std::shared_ptr<OutputManager> & optional_output_manager) const
    {
        if (! hooker)
        {
            hooker = std::make_shared<Hooker>(this);
            for (std::list<std::pair<FSPath, bool> >::const_iterator h(hook_dirs.begin()),
                    h_end(hook_dirs.end()) ; h != h_end ; ++h)
                hooker->add_dir(h->first, h->second);
        }
        return hooker->perform_hook(hook, optional_output_manager);
    }
}


namespace
{
    struct TestMerger :
        FSMerger
    {
        TestMerger(const FSMergerParams & p) :
            FSMerger(p)
        {
        }

        void record_install_file(const FSPath &, const FSPath &, const std::string &, const FSMergerStatusFlags &)
        {
        }

        void record_install_dir(const FSPath &, const FSPath &, const FSMergerStatusFlags &)
        {
        }

        void record_install_sym(const FSPath &, const FSPath &, const FSMergerStatusFlags &)
        {
        }

        virtual void record_install_under_dir(const FSPath &, const FSMergerStatusFlags &)
        {
        }

        void on_error(bool is_check, const std::string & s)
        {
            if (is_check)
                make_check_fail();
            else
                throw FSMergerError(s);
        }

        void on_warn(bool, const std::string &)
        {
        }

        void display_override(const std::string &) const
        {
        }

        bool config_protected(const FSPath &, const FSPath &)
        {
            return false;
        }

        std::string make_config_protect_name(const FSPath & src, const FSPath &)
        {
            return src.basename() + ".cfgpro";
        }
    };

    class MergerTest :
        public TestCase
    {
        public:
            FSPath image_dir;
            FSPath root_dir;
            HookTestEnvironment env;
            TestMerger merger;

            bool repeatable() const
            {
                return false;
            }

        protected:
            MergerTest(EntryType src_type, EntryType dst_type, int n = 0) :
                TestCase("merge " + stringify(src_type) + " over " + stringify(dst_type) + (0 == n ? "" : " "
                            + stringify(n))),
                image_dir("fs_merger_TEST_dir/" + stringify(src_type) + "_over_" + stringify(dst_type)
                        + (0 == n ? "" : "_" + stringify(n)) + "_dir/image"),
                root_dir("fs_merger_TEST_dir/" + stringify(src_type) + "_over_" + stringify(dst_type)
                        + (0 == n ? "" : "_" + stringify(n)) + "_dir/root"),
                env(FSPath("fs_merger_TEST_dir/hooks")),
                merger(make_named_values<FSMergerParams>(
                            n::environment() = &env,
                            n::fix_mtimes_before() = Timestamp(0, 0),
                            n::get_new_ids_or_minus_one() = &get_new_ids_or_minus_one,
                            n::image() = image_dir,
                            n::install_under() = FSPath("/"),
                            n::maybe_output_manager() = make_null_shared_ptr(),
                            n::merged_entries() = std::make_shared<FSPathSet>(),
                            n::no_chown() = true,
                            n::options() = MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs,
                            n::root() = root_dir
                        ))
            {
            }

            MergerTest(const std::string & custom_test,
                    const MergerOptions & o = MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs,
                    const bool fix = false) :
                TestCase("merge " + custom_test + " test"),
                image_dir("fs_merger_TEST_dir/" + custom_test + "/image"),
                root_dir("fs_merger_TEST_dir/" + custom_test + "/root"),
                env(FSPath("fs_merger_TEST_dir/hooks")),
                merger(make_named_values<FSMergerParams>(
                        n::environment() = &env,
                        n::fix_mtimes_before() = fix ? FSPath("fs_merger_TEST_dir/reference").stat().mtim() : Timestamp(0, 0),
                        n::get_new_ids_or_minus_one() = &get_new_ids_or_minus_one,
                        n::image() = image_dir,
                        n::install_under() = FSPath("/"),
                        n::maybe_output_manager() = make_null_shared_ptr(),
                        n::merged_entries() = std::make_shared<FSPathSet>(),
                        n::no_chown() = true,
                        n::options() = o,
                        n::root() = root_dir
                        ))
            {
            }
    };
}

namespace test_cases
{
    struct MergerTestSymNothing : MergerTest
    {
        MergerTestSymNothing() : MergerTest(et_sym, et_nothing) { }

        void run()
        {
            TEST_CHECK(! (root_dir / "sym").stat().exists());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "sym").stat().is_symlink());
            TEST_CHECK((root_dir / "rewrite_me").stat().is_symlink());
            TEST_CHECK_EQUAL((root_dir / "sym").readlink(), "image_dst");
            TEST_CHECK_EQUAL((root_dir / "rewrite_me").readlink(), "/rewrite_target");
        }
    } test_merger_sym_nothing;

    struct MergerTestSymSym : MergerTest
    {
        MergerTestSymSym() : MergerTest(et_sym, et_sym) { }

        void run()
        {
            TEST_CHECK((root_dir / "sym").stat().is_symlink());
            TEST_CHECK_EQUAL((root_dir / "sym").readlink(), "root_dst");

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "sym").stat().is_symlink());
            TEST_CHECK((root_dir / "rewrite_me").stat().is_symlink());
            TEST_CHECK_EQUAL((root_dir / "sym").readlink(), "image_dst");
            TEST_CHECK_EQUAL((root_dir / "rewrite_me").readlink(), "/rewrite_target");
        }
    } test_merger_sym_sym;

    struct MergerTestSymFile : MergerTest
    {
        MergerTestSymFile() : MergerTest(et_sym, et_file) { }

        void run()
        {
            TEST_CHECK((root_dir / "sym").stat().is_regular_file());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "sym").stat().is_symlink());
            TEST_CHECK((root_dir / "rewrite_me").stat().is_symlink());
            TEST_CHECK_EQUAL((root_dir / "sym").readlink(), "image_dst");
            TEST_CHECK_EQUAL((root_dir / "rewrite_me").readlink(), "/rewrite_target");
        }
    } test_merger_sym_file;

    struct MergerTestSymDir : MergerTest
    {
        MergerTestSymDir() : MergerTest(et_sym, et_dir) { }

        void run()
        {
            TEST_CHECK((root_dir / "sym").stat().is_directory());

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), FSMergerError);

            TEST_CHECK((root_dir / "sym").stat().is_directory());
        }
    } test_merger_sym_dir;

    struct MergerTestDirNothing : MergerTest
    {
        MergerTestDirNothing() : MergerTest(et_dir, et_nothing) { }

        void run()
        {
            TEST_CHECK(! (root_dir / "dir").stat().exists());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "dir").stat().is_directory());
        }
    } test_merger_dir_nothing;

    struct MergerTestDirDir : MergerTest
    {
        MergerTestDirDir() : MergerTest(et_dir, et_dir) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").stat().is_directory());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "dir").stat().is_directory());
        }
    } test_merger_dir_dir;

    struct MergerTestDirFile : MergerTest
    {
        MergerTestDirFile() : MergerTest(et_dir, et_file) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").stat().is_regular_file());

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), FSMergerError);

            TEST_CHECK((root_dir / "dir").stat().is_regular_file());
        }
    } test_merger_dir_file;

    struct MergerTestDirSym1 : MergerTest
    {
        MergerTestDirSym1() : MergerTest(et_dir, et_sym, 1) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").stat().is_symlink());
            TEST_CHECK((root_dir / "dir").realpath().stat().is_directory());
            TEST_CHECK(! (root_dir / "dir" / "file").stat().exists());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "dir").stat().is_symlink());
            TEST_CHECK((root_dir / "dir").realpath().stat().is_directory());
            TEST_CHECK((root_dir / "dir" / "file").stat().is_regular_file());
        }
    } test_merger_dir_sym_1;

    struct MergerTestDirSym2 : MergerTest
    {
        MergerTestDirSym2() : MergerTest(et_dir, et_sym, 2) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").stat().is_symlink());
            TEST_CHECK((root_dir / "dir").realpath().stat().is_regular_file());

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), FSMergerError);

            TEST_CHECK((root_dir / "dir").stat().is_symlink());
            TEST_CHECK((root_dir / "dir").realpath().stat().is_regular_file());
        }
    } test_merger_dir_sym_2;

    struct MergerTestDirSym3 : MergerTest
    {
        MergerTestDirSym3() : MergerTest(et_dir, et_sym, 3) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").stat().is_symlink());
            TEST_CHECK_THROWS((root_dir / "dir").realpath(), FSError);

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), FSMergerError);

            TEST_CHECK((root_dir / "dir").stat().is_symlink());
            TEST_CHECK_THROWS((root_dir / "dir").realpath(), FSError);
        }
    } test_merger_dir_sym_3;

    struct MergerTestFileNothing : MergerTest
    {
        MergerTestFileNothing() : MergerTest(et_file, et_nothing) { }

        void run()
        {
            TEST_CHECK(! (root_dir / "file").stat().exists());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "file").stat().is_regular_file());
            SafeIFStream f(root_dir / "file");
            TEST_CHECK(f);
            std::string fs(std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()));
            TEST_CHECK_EQUAL(fs, "image contents\n");
        }
    } test_merger_file_nothing;

    struct MergerTestFileFile : MergerTest
    {
        MergerTestFileFile() : MergerTest(et_file, et_file) { }

        void run()
        {
            TEST_CHECK((root_dir / "file").stat().is_regular_file());
            SafeIFStream b(root_dir / "file");
            TEST_CHECK(b);
            std::string bs((std::istreambuf_iterator<char>(b)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(bs, "root contents\n");

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "file").stat().is_regular_file());
            SafeIFStream f(root_dir / "file");
            TEST_CHECK(f);
            std::string fs((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(fs, "image contents\n");
        }
    } test_merger_file_file;

    struct MergerTestFileSym : MergerTest
    {
        MergerTestFileSym() : MergerTest(et_file, et_sym) { }

        void run()
        {
            TEST_CHECK((root_dir / "file1").stat().is_symlink());
            TEST_CHECK((root_dir / "file2").stat().is_symlink());
            TEST_CHECK((root_dir / "file3").stat().is_symlink());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "file1").stat().is_regular_file());
            SafeIFStream f(root_dir / "file1");
            TEST_CHECK(f);
            std::string fs((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(fs, "image 1 contents\n");

            TEST_CHECK((root_dir / "file2").stat().is_regular_file());
            SafeIFStream f2(root_dir / "file2");
            TEST_CHECK(f2);
            std::string fs2((std::istreambuf_iterator<char>(f2)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(fs2, "image 2 contents\n");

            TEST_CHECK((root_dir / "file3").stat().is_regular_file());
            SafeIFStream f3(root_dir / "file3");
            TEST_CHECK(f3);
            std::string fs3((std::istreambuf_iterator<char>(f3)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(fs3, "image 3 contents\n");
        }
    } test_merger_file_sym;

    struct MergerTestFileDir : MergerTest
    {
        MergerTestFileDir() : MergerTest(et_file, et_dir) { }

        void run()
        {
            TEST_CHECK((root_dir / "file").stat().is_directory());

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), FSMergerError);

            TEST_CHECK((root_dir / "file").stat().is_directory());
        }
    } test_merger_file_dir;

    struct MergerOverrideTest : MergerTest
    {
        MergerOverrideTest() : MergerTest("override") { }

        void run()
        {
            TEST_CHECK((image_dir / "dir_skip_me").stat().is_directory());
            TEST_CHECK((image_dir / "dir_install_me").stat().is_directory());
            TEST_CHECK((image_dir / "file_skip_me").stat().is_regular_file());
            TEST_CHECK((image_dir / "file_install_me").stat().is_regular_file());
            TEST_CHECK((image_dir / "sym_skip_me").stat().is_symlink());
            TEST_CHECK((image_dir / "sym_install_me").stat().is_symlink());

            TEST_CHECK(merger.check());
            merger.merge();


            TEST_CHECK(! (root_dir / "dir_skip_me").stat().exists());
            TEST_CHECK((root_dir / "dir_install_me").stat().is_directory());
            TEST_CHECK(! (root_dir / "file_skip_me").stat().exists());
            TEST_CHECK((root_dir / "file_install_me").stat().is_regular_file());
            TEST_CHECK(! (root_dir / "sym_skip_me").stat().exists());
            TEST_CHECK((root_dir / "sym_install_me").stat().is_symlink());
        }
    } test_merger_override;

    struct MergerEmptyDirAllowedTest : MergerTest
    {
        MergerEmptyDirAllowedTest() : MergerTest("empty_dir_allowed", { mo_allow_empty_dirs }) { }

        void run()
        {
            TEST_CHECK((image_dir / "empty").stat().is_directory());
            TEST_CHECK(FSIterator(image_dir / "empty", { fsio_include_dotfiles, fsio_first_only }) == FSIterator());

            TEST_CHECK(merger.check());
        }
    } test_merger_empty_dir_allowed;

    struct MergerEmptyDirDisallowedTest : MergerTest
    {
        MergerEmptyDirDisallowedTest() : MergerTest("empty_dir_disallowed", { }) { }

        void run()
        {
            TEST_CHECK((image_dir / "empty").stat().is_directory());
            TEST_CHECK(FSIterator(image_dir / "empty", { fsio_include_dotfiles, fsio_first_only }) == FSIterator());

            TEST_CHECK(! merger.check());
        }
    } test_merger_empty_dir_disallowed;

    struct MergerEmptyRootAllowedTest : MergerTest
    {
        MergerEmptyRootAllowedTest() : MergerTest("empty_root_allowed", { mo_allow_empty_dirs }) { }

        void run()
        {
            TEST_CHECK(FSIterator(image_dir, { fsio_include_dotfiles, fsio_first_only }) == FSIterator());

            TEST_CHECK(merger.check());
        }
    } test_merger_empty_root_allowed;

    struct MergerEmptyRootDisallowedTest : MergerTest
    {
        MergerEmptyRootDisallowedTest() : MergerTest("empty_root_disallowed", { }) { }

        void run()
        {
            TEST_CHECK(FSIterator(image_dir, { fsio_include_dotfiles, fsio_first_only }) == FSIterator());

            TEST_CHECK(! merger.check());
        }
    } test_merger_empty_root_disallowed;

    struct MergerMtimesTest : MergerTest
    {
        MergerMtimesTest() : MergerTest("mtimes", { mo_preserve_mtimes }) { }

        void run()
        {
            Timestamp m_new((image_dir / "new_file").stat().mtim());
            Timestamp m_existing((image_dir / "existing_file").stat().mtim());
            Timestamp m_dir_new((image_dir / "dir" / "new_file").stat().mtim());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK(timestamps_nearly_equal((root_dir / "new_file").stat().mtim(), m_new));
            TEST_CHECK(timestamps_nearly_equal((root_dir / "existing_file").stat().mtim(), m_existing));
            TEST_CHECK(Timestamp::now().seconds() - (root_dir / "dodgy_file").stat().mtim().seconds() >= (60 * 60 * 24 * 365 * 3) - 1);

            TEST_CHECK(timestamps_nearly_equal((root_dir / "dir" / "new_file").stat().mtim(), m_dir_new));
            TEST_CHECK(Timestamp::now().seconds() - (root_dir / "dir" / "dodgy_file").stat().mtim().seconds() >= (60 * 60 * 24 * 365 * 3) - 1);
        }
    } test_merger_mtimes;

    struct MergerMtimesFixTest : MergerTest
    {
        MergerMtimesFixTest() : MergerTest("mtimes_fix", { mo_preserve_mtimes }, true) { }

        void run()
        {
            Timestamp m_new((image_dir / "new_file").stat().mtim());
            Timestamp m_existing((image_dir / "existing_file").stat().mtim());
            Timestamp m_dir_new((image_dir / "dir" / "new_file").stat().mtim());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK(timestamps_nearly_equal((root_dir / "new_file").stat().mtim(), m_new));
            TEST_CHECK(timestamps_nearly_equal((root_dir / "existing_file").stat().mtim(), m_existing));
            TEST_CHECK(timestamps_nearly_equal((root_dir / "dodgy_file").stat().mtim(), FSPath("fs_merger_TEST_dir/reference").stat().mtim()));

            TEST_CHECK(timestamps_nearly_equal((root_dir / "dir" / "new_file").stat().mtim(), m_dir_new));
            TEST_CHECK(timestamps_nearly_equal((root_dir / "dir" / "dodgy_file").stat().mtim(), FSPath("fs_merger_TEST_dir/reference").stat().mtim()));
        }
    } test_merger_mtimes_fix;
}

