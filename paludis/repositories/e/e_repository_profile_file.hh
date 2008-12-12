/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_PROFILE_FILE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_PROFILE_FILE_HH 1

#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <tr1/type_traits>

namespace paludis
{
    struct ERepository;

    namespace erepository
    {
        /**
         * A file in a ERepository profile.
         *
         * Handles -lines, comments, inherits automatically.
         *
         * \ingroup grperepository
         */
        template <typename F_>
        class PALUDIS_VISIBLE ProfileFile :
            private PrivateImplementationPattern<ProfileFile<F_> >
        {
            public:
                ///\name Basic operations
                ///\{

                ProfileFile(const ERepository * const);
                ~ProfileFile();

                ///\}

                /**
                 * Add a file.
                 */
                void add_file(const FSEntry &);

                ///\name Iterate over our profile lines.
                ///\{

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::pair<
                    std::tr1::shared_ptr<const erepository::EAPI>,
                          const typename std::tr1::remove_reference<
                              typename F_::ConstIterator::value_type>::type> > ConstIterator;
                ConstIterator begin() const;
                ConstIterator end() const;

                ///\}
        };
    }
}

#endif
