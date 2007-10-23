/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_OPTIONS_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_OPTIONS_HH 1

#include <paludis/util/options-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>

/** \file
 * Declarations for the Options<> class.
 *
 * \ingroup g_data_structures
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Used by Options<> for underlying storage.
     *
     * Holds a collection of bits, similar to std::bitset<>, but with no fixed
     * underlying size.
     *
     * \see Options<>
     * \ingroup g_data_structures
     */
    class PALUDIS_VISIBLE OptionsStore :
        private PrivateImplementationPattern<OptionsStore>
    {
        public:
            ///\name Basic operations
            ///\{

            OptionsStore();
            OptionsStore(const OptionsStore &);
            const OptionsStore & operator= (const OptionsStore &);
            ~OptionsStore();

            ///\}

            ///\name Modifications
            ///\{

            /**
             * Set the specified bit.
             */
            void add(const unsigned);

            /**
             * Unset the specified bit.
             */
            void remove(const unsigned);

            /**
             * Set any bit that is set in the parameter.
             */
            void combine(const OptionsStore &);

            /**
             * Unset any bit that is set in the parameter.
             */
            void subtract(const OptionsStore &);

            ///\}

            ///\name Tests
            ///\{

            /**
             * Is a particular bit set?
             */
            bool test(const unsigned) const;

            /**
             * Is any bit set?
             */
            bool any() const;

            ///\}
    };

    /**
     * Holds a series of true/false values mapped on an enum type, like a
     * std::bitset<> without the static size requirement.
     *
     * \ingroup g_data_structures
     */
    template <typename E_>
    class Options
    {
        private:
            OptionsStore _store;

        public:
            /**
             * Return a copy of ourself with the specified bit enabled.
             */
            Options operator+ (const E_ & e) const
            {
                Options result(*this);
                result._store.add(static_cast<unsigned>(e));
                return result;
            }

            /**
             * Enable the specified bit.
             */
            Options & operator+= (const E_ & e)
            {
                _store.add(static_cast<unsigned>(e));
                return *this;
            }

            /**
             * Return a copy of ourself with the specified bit disabled.
             */
            Options operator- (const E_ & e) const
            {
                Options result(*this);
                result._store.remove(static_cast<unsigned>(e));
                return result;
            }

            /**
             * Disable the specified bit.
             */
            Options & operator-= (const E_ & e)
            {
                _store.remove(static_cast<unsigned>(e));
                return *this;
            }

            /**
             * Return a copy of ourself, bitwise 'or'ed with another Options set.
             */
            Options operator| (const Options<E_> & e) const
            {
                Options result(*this);
                result._store.combine(e._store);
                return result;
            }

            /**
             * Enable any bits that are enabled in the parameter.
             */
            Options & operator|= (const Options<E_> & e)
            {
                _store.combine(e._store);
                return *this;
            }

            /**
             * Disable any bits that are enabled in the parameter.
             */
            Options & subtract(const Options<E_> & e)
            {
                _store.subtract(e._store);
                return *this;
            }

            /**
             * Returns whether the specified bit is enabled.
             */
            bool operator[] (const E_ & e) const
            {
                return _store.test(static_cast<unsigned>(e));
            }

            /**
             * Returns whether any bit is enabled.
             */
            bool any() const
            {
                return _store.any();
            }

            /**
             * Returns whether all bits are disabled.
             */
            bool none() const
            {
                return ! _store.any();
            }
    };
}

#endif
