/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_HASHES_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_HASHES_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/wrapped_value-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <cstddef>
#include <utility>
#include <string>
#include <tr1/type_traits>
#include <tr1/memory>

namespace paludis
{
    namespace hash_internals
    {
        template <typename T_, bool is_numeric_>
        struct DefaultHash
        {
            static std::size_t hash(const T_ & t)
            {
                return static_cast<std::size_t>(t);
            }
        };

        template <typename T_>
        struct DefaultHash<T_, false>
        {
            static std::size_t hash(const T_ & t)
            {
                return t.hash();
            }
        };
    }

    template <typename T_>
    class Hash
    {
        public:
            std::size_t operator() (const T_ & t) const
            {
                return hash_internals::DefaultHash<T_, std::tr1::is_integral<T_>::value>::hash(t);
            }
    };

    template <>
    struct PALUDIS_VISIBLE Hash<std::string>
    {
        std::size_t operator() (const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    template <>
    struct PALUDIS_VISIBLE Hash<FSEntry>
    {
        std::size_t operator() (const FSEntry &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    template <typename T_, typename U_>
    struct Hash<std::pair<T_, U_> >
    {
        std::size_t operator() (const std::pair<T_, U_> & p) const
        {
            return Hash<T_>()(p.first) ^ Hash<U_>()(p.second);
        }
    };

    template <typename Tag_>
    struct Hash<WrappedValue<Tag_> >
    {
        std::size_t operator() (const WrappedValue<Tag_> & v) const
        {
            return Hash<typename WrappedValueTraits<Tag_>::UnderlyingType>()(v.value());
        }
    };

    template <typename T_>
    struct Hash<const T_>
    {
        std::size_t operator() (const T_ & t) const
        {
            return Hash<T_>()(t);
        }
    };

    template <typename T_>
    struct Hash<T_ &>
    {
        std::size_t operator() (const T_ & t) const
        {
            return Hash<T_>()(t);
        }
    };

    template <typename T_>
    struct Hash<T_ *>
    {
        std::size_t operator() (const T_ * const t) const
        {
            return Hash<T_>(*t);
        }
    };

    template <typename T_>
    struct Hash<std::tr1::shared_ptr<T_> >
    {
        std::size_t operator() (const std::tr1::shared_ptr<const T_> & t) const
        {
            return Hash<T_>()(*t);
        }
    };
}

#endif
