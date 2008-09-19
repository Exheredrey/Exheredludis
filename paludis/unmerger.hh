/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
 * Copyright (c) 2007 Piotr Jaroszyński
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

#ifndef PALUDIS_GUARD_PALUDIS_MERGER_UNMERGER_HH
#define PALUDIS_GUARD_PALUDIS_MERGER_UNMERGER_HH 1

#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/named_value.hh>
#include <paludis/merger_entry_type.hh>

/** \file
 * Declarations for the Unmerger class, which can be used by Repository
 * to implement from-filesystem unmerging.
 *
 * \ingroup g_repository
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    class Hook;
    class Environment;

    namespace n
    {
        struct environment;
        struct root;
    }

    /**
     * Options for a basic Unmerger.
     *
     * \see Unmerger
     * \ingroup g_repository
     * \since 0.30
     */
    struct UnmergerOptions
    {
        NamedValue<n::environment, const Environment *> environment;
        NamedValue<n::root, FSEntry> root;
    };

    /**
     * Thrown if an error occurs during an unmerge.
     *
     * \ingroup g_repository
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UnmergerError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            UnmergerError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * Handles unmerging items.
     *
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Unmerger :
        private PrivateImplementationPattern<Unmerger>
    {
        protected:
            ///\name Basic operations
            ///\{

            Unmerger(const UnmergerOptions &);

            ///\}

            /**
             * Base class for extra information associated with a file
             * to be unmerged.
             *
             * \ingroup g_repository
             * \nosubgrouping
             */
            class PALUDIS_VISIBLE ExtraInfo
            {
                public:
                    virtual ~ExtraInfo();
            };

            friend class Implementation<Unmerger>;

            /**
             * Add entry to the unmerge set.
             */
            void add_unmerge_entry(const std::string &, EntryType, const std::tr1::shared_ptr<ExtraInfo> &);

            /**
             * Populate the unmerge set.
             */
            virtual void populate_unmerge_set() = 0;

            /**
             * Extend a hook with extra options.
             */
            virtual Hook extend_hook(const Hook &) const;

            ///\name Unmerge operations
            ///\{

            virtual void unmerge_file(FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const;
            virtual void unmerge_dir(FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const;
            virtual void unmerge_sym(FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const;
            virtual void unmerge_misc(FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const;

            ///\}

            ///\name Check operations
            ///\{

            virtual bool check_file(const FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const
            {
                return true;
            }

            virtual bool check_dir(const FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const
            {
                return true;
            }

            virtual bool check_sym(const FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const
            {
                return true;
            }

            virtual bool check_misc(const FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const
            {
                return true;
            }

            ///\}

            ///\name Unlink operations
            ///\{

            virtual void unlink_file(FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const;
            virtual void unlink_dir(FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const;
            virtual void unlink_sym(FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const;
            virtual void unlink_misc(FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const;

            ///\}

            virtual void display(const std::string &) const = 0;

        public:
            ///\name Basic operations
            ///\{

            virtual ~Unmerger() = 0;

            ///\}

            /**
             * Perform the unmerge.
             */
            void unmerge();
    };
}

#endif
