/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_MASK_INFO_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_MASK_INFO_HH 1

#include <paludis/util/named_value.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_comment> comment;
        typedef Name<struct name_mask_file> mask_file;
        typedef Name<struct name_token> token;
    }

    namespace erepository
    {
        struct MaskInfo
        {
            NamedValue<n::comment, std::string> comment;
            NamedValue<n::mask_file, FSPath> mask_file;
            NamedValue<n::token, std::string> token;
        };

        typedef Sequence<MaskInfo> MasksInfo;
    }

    extern template class Sequence<erepository::MaskInfo>;
    extern template class WrappedForwardIterator<Sequence<erepository::MaskInfo>::ConstIterator, const erepository::MaskInfo>;
}

#endif
