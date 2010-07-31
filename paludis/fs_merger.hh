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

#ifndef PALUDIS_GUARD_PALUDIS_FS_MERGER_HH
#define PALUDIS_GUARD_PALUDIS_FS_MERGER_HH 1

#include <paludis/fs_merger-fwd.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/options.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/merger_entry_type.hh>
#include <paludis/merger.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/hook-fwd.hh>
#include <iosfwd>
#include <sys/stat.h>
#include <sys/types.h>

/** \file
 * Declarations for the FSMerger class, which can be used by Repository
 * implementations to perform to-filesystem merging.
 *
 * \ingroup g_repository
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    namespace n
    {
        typedef Name<struct environment_name> environment;
        typedef Name<struct fix_mtimes_before_name> fix_mtimes_before;
        typedef Name<struct get_new_ids_or_minus_one_name> get_new_ids_or_minus_one;
        typedef Name<struct image_name> image;
        typedef Name<struct install_under_name> install_under;
        typedef Name<struct merged_entries_name> merged_entries;
        typedef Name<struct no_chown_name> no_chown;
        typedef Name<struct options_name> options;
        typedef Name<struct root_name> root;
    }

    /**
     * Parameters for a basic FSMerger.
     *
     * \see Merger
     * \ingroup g_repository
     * \nosubgrouping
     * \since 0.30
     * \since 0.51 called FSMergerParams instead of MergerParams
     */
    struct FSMergerParams
    {
        NamedValue<n::environment, Environment *> environment;

        /**
         * Rewrite any mtimes that are before this time to this time, even if
         * preserving mtimes.
         *
         * \since 0.44
         */
        NamedValue<n::fix_mtimes_before, Timestamp> fix_mtimes_before;

        NamedValue<n::get_new_ids_or_minus_one, std::function<std::pair<uid_t, gid_t> (const FSEntry &)> > get_new_ids_or_minus_one;
        NamedValue<n::image, FSEntry> image;
        NamedValue<n::install_under, FSEntry> install_under;

        /**
         * We record things we merged here.
         *
         * \since 0.41
         */
        NamedValue<n::merged_entries, std::shared_ptr<FSEntrySet> > merged_entries;

        NamedValue<n::no_chown, bool> no_chown;
        NamedValue<n::options, MergerOptions> options;
        NamedValue<n::root, FSEntry> root;
    };

    /**
     * Thrown if an error occurs during an FSMerger operation.
     *
     * \ingroup g_repository
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE FSMergerError :
        public MergerError
    {
        public:
            ///\name Basic operations
            ///\{

            FSMergerError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * Handles merging an image to a live filesystem.
     *
     * \ingroup g_exceptions
     * \ingroup g_repository
     * \nosubgrouping
     * \since 0.51 called FSMerger instead of Merger
     */
    class PALUDIS_VISIBLE FSMerger :
        private Pimp<FSMerger>,
        public Merger
    {
        private:
            void track_renamed_dir_recursive(const FSEntry &);
            void relabel_dir_recursive(const FSEntry &, const FSEntry &);
            void try_to_copy_xattrs(const FSEntry &, int, FSMergerStatusFlags &);

            Pimp<FSMerger>::ImpPtr & _imp;

        protected:
            ///\name Basic operations
            ///\{

            FSMerger(const FSMergerParams &);

            ///\}

            virtual Hook extend_hook(const Hook &);

            ///\name Track and record merges
            ///\{

            void track_install_file(const FSEntry &, const FSEntry &, const std::string &, const FSMergerStatusFlags &);
            void track_install_dir(const FSEntry &, const FSEntry &, const FSMergerStatusFlags &);
            void track_install_under_dir(const FSEntry &, const FSMergerStatusFlags &);
            void track_install_sym(const FSEntry &, const FSEntry &, const FSMergerStatusFlags &);

            ///\}

            ///\name Handle filesystem entry things
            ///\{

            virtual void on_file_main(bool is_check, const FSEntry & src, const FSEntry & dst);
            virtual void on_file_over_nothing(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_sym(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_misc(bool is_check, const FSEntry &, const FSEntry &);

            virtual FSMergerStatusFlags install_file(const FSEntry &, const FSEntry &, const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void unlink_file(FSEntry);
            virtual void record_install_file(const FSEntry &, const FSEntry &, const std::string &, const FSMergerStatusFlags &) = 0;

            virtual void on_dir_main(bool is_check, const FSEntry & src, const FSEntry & dst);
            virtual void on_dir_over_nothing(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_sym(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_misc(bool is_check, const FSEntry &, const FSEntry &);

            virtual FSMergerStatusFlags install_dir(const FSEntry &, const FSEntry &) PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void unlink_dir(FSEntry);
            virtual void record_install_dir(const FSEntry &, const FSEntry &, const FSMergerStatusFlags &) = 0;
            virtual void record_install_under_dir(const FSEntry &, const FSMergerStatusFlags &) = 0;

            virtual void on_sym_main(bool is_check, const FSEntry & src, const FSEntry & dst);
            virtual void on_sym_over_nothing(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_sym(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_misc(bool is_check, const FSEntry &, const FSEntry &);

            virtual FSMergerStatusFlags install_sym(const FSEntry &, const FSEntry &) PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void unlink_sym(FSEntry);
            virtual void record_install_sym(const FSEntry &, const FSEntry &, const FSMergerStatusFlags &) = 0;

            virtual void unlink_misc(FSEntry);

            virtual void prepare_install_under();

            virtual FSEntry canonicalise_root_path(const FSEntry & f);

            virtual void do_dir_recursive(bool is_check, const FSEntry &, const FSEntry &);

            ///\}

            ///\name Configuration protection
            ///\{

            virtual bool config_protected(const FSEntry &, const FSEntry &) = 0;
            virtual std::string make_config_protect_name(const FSEntry &, const FSEntry &) = 0;

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~FSMerger();

            FSMerger(const FSMerger &) = delete;
            FSMerger & operator= (const FSMerger &) = delete;

            ///\}

            virtual void merge();
    };

}

#endif
