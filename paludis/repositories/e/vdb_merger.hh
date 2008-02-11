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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_VDB_MERGER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_VDB_MERGER_HH 1

#include <paludis/merger.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    class Hook;

#include <paludis/repositories/e/vdb_merger-sr.hh>

    /**
     * Merger for VDBRepository.
     *
     * \ingroup grpvdbrepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE VDBMerger :
        public Merger,
        private PrivateImplementationPattern<VDBMerger>
    {
        private:
            void display_override(const std::string &) const;
            std::string make_arrows(const MergeStatusFlags &) const;

        public:
            ///\name Basic operations
            ///\{

            VDBMerger(const VDBMergerParams &);
            ~VDBMerger();

            ///\}

            virtual Hook extend_hook(const Hook &);

            virtual void record_install_file(const FSEntry &, const FSEntry &, const std::string &, const MergeStatusFlags &);
            virtual void record_install_dir(const FSEntry &, const FSEntry &, const MergeStatusFlags &);
            virtual void record_install_sym(const FSEntry &, const FSEntry &, const MergeStatusFlags &);

            virtual void on_error(bool is_check, const std::string &);
            virtual void on_warn(bool is_check, const std::string &);
            virtual void on_enter_dir(bool is_check, const FSEntry);

            virtual void on_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym(bool is_check, const FSEntry &, const FSEntry &);

            virtual bool config_protected(const FSEntry &, const FSEntry &);
            virtual std::string make_config_protect_name(const FSEntry &, const FSEntry &);

            virtual void merge();
            virtual bool check();
    };
}

#endif
