/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SET_FILE_HH
#define PALUDIS_GUARD_PALUDIS_SET_FILE_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/options.hh>
#include <paludis/util/named_value.hh>
#include <paludis/name.hh>
#include <paludis/spec_tree.hh>
#include <paludis/dep_spec-fwd.hh>
#include <functional>
#include <iosfwd>

/** \file
 * Declarations for the SetFile classes, which are used by Environment and
 * Repository implementations for files containing a package set.
 *
 * \ingroup g_environment
 * \ingroup g_repository
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    class Environment;

#include <paludis/set_file-se.hh>

    namespace n
    {
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_file_name> file_name;
        typedef Name<struct name_parser> parser;
        typedef Name<struct name_set_operator_mode> set_operator_mode;
        typedef Name<struct name_tag> tag;
        typedef Name<struct name_type> type;
    }

    /**
     * Parameters for a SetFile.
     *
     * \ingroup g_environment
     * \ingroup g_repository
     */
    struct SetFileParams
    {
        NamedValue<n::environment, const Environment *> environment;
        NamedValue<n::file_name, FSPath> file_name;
        NamedValue<n::parser, std::function<PackageDepSpec (const std::string &)> > parser;
        NamedValue<n::set_operator_mode, SetFileSetOperatorMode> set_operator_mode;
        NamedValue<n::type, SetFileType> type;
    };

    /**
     * Thrown if there is a problem reading or writing a SetFile.
     *
     * \ingroup g_environment
     * \ingroup g_repository
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE SetFileError :
        public ConfigurationError
    {
        public:
            ///\name Basic operations
            ///\{

            SetFileError(const FSPath &, const std::string &) noexcept;

            ///\}
    };

    /**
     * Shared code for files containing a package set.
     *
     * Various set file formats are supported:
     *
     * - sft_paludis_conf, a line-based set file with prefixed entries
     * - sft_paludis_bash, a bash script that outputs an sft_paludis_conf
     * - sft_simple, a simple line-based file
     *
     * The file can be modified if it is sft_paludis_conf or sft_simple.
     *
     * \ingroup g_environment
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE SetFile
    {
        private:
            Pimp<SetFile> _imp;

        public:
            ///\name Basic operations
            ///\{

            SetFile(const SetFileParams &);
            ~SetFile();

            SetFile(const SetFile &) = delete;
            SetFile & operator= (const SetFile &) = delete;

            ///\}

            /**
             * Fetch our contents.
             */
            const std::shared_ptr<const SetSpecTree> contents() const;

            /**
             * Rewrite our contents.
             */
            void rewrite() const;

            /**
             * Add an item to our contents, if it is not there already.
             *
             * \since 0.49 returns whether any lines were added
             */
            bool add(const std::string &);

            /**
             * Remove any matching lines.
             *
             * \since 0.48 returns whether any lines were removed
             */
            bool remove(const std::string &);
    };

    /**
     * Split a SetName into a SetName and a SetFileSetOperatorMode.
     *
     * \see SetName
     * \ingroup g_repository
     * \since 0.26
     */
    std::pair<SetName, SetFileSetOperatorMode> find_base_set_name_and_suffix_mode(const SetName &)
        PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));
}

#endif
