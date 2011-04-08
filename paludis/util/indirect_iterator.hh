/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_INDIRECT_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_INDIRECT_ITERATOR_HH 1

#include <paludis/util/indirect_iterator-fwd.hh>
#include <paludis/util/operators.hh>
#include <memory>
#include <functional>
#include <type_traits>

namespace paludis
{
    template <typename T_>
    struct IndirectIteratorValueType
    {
        typedef typename std::iterator_traits<T_>::value_type Type;
    };

    template <typename T_>
    struct IndirectIteratorValueType<T_ *>
    {
        typedef T_ Type;
    };

    template <typename T_>
    struct IndirectIteratorValueType<std::shared_ptr<T_> >
    {
        typedef T_ Type;
    };

    template <typename T_>
    struct IndirectIteratorValueType<std::shared_ptr<const T_> >
    {
        typedef const T_ Type;
    };

    template <typename T_>
    struct IndirectIteratorValueType<const T_>
    {
        typedef typename IndirectIteratorValueType<T_>::Type Type;
    };

    template <typename T_>
    struct IndirectIteratorValueType<T_ &>
    {
        typedef typename IndirectIteratorValueType<T_>::Type Type;
    };

    /**
     * An IndirectIterator turns an iterator over T_ * or std::shared_ptr<T_> into an iterator
     * over T_.
     *
     * \ingroup g_iterator
     * \see indirect_iterator
     */
    template <typename Iter_, typename Value_>
    class IndirectIterator :
        public relational_operators::HasRelationalOperators
    {
        friend bool operator== <> (const IndirectIterator &, const IndirectIterator &);
        friend bool operator< <> (const IndirectIterator &, const IndirectIterator &);

        private:
            Iter_ _iter;

        public:
            ///\name Basic operations
            ///\{

            IndirectIterator();
            IndirectIterator(const IndirectIterator &);
            IndirectIterator(const Iter_ &);

            IndirectIterator & operator= (const IndirectIterator &);

            ///\}

            ///\name Standard library typedefs
            ///\{

            typedef typename std::remove_reference<Value_>::type & value_type;
            typedef typename std::remove_reference<Value_>::type & reference;
            typedef typename std::remove_reference<Value_>::type * pointer;
            typedef std::ptrdiff_t difference_type;
            typedef std::forward_iterator_tag iterator_category;

            ///\}

            ///\name Additional typedefs
            ///\{

            typedef Iter_ underlying_iterator_type;

            ///\}

            ///\name Increment
            ///\{

            IndirectIterator & operator++ ();
            IndirectIterator operator++ (int);

            ///\}

            ///\name Dereference
            ///\{

            pointer operator-> () const;
            reference operator* () const;

            underlying_iterator_type underlying_iterator();

            ///\}
    };

    /**
     * Construct an IndirectIterator from another iterator.
     *
     * \see IndirectIterator
     * \ingroup g_iterator
     */
    template <typename Iter_>
    IndirectIterator<Iter_> indirect_iterator(const Iter_ & t);
}

#endif
