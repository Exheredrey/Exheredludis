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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_ACTION_QUEUE_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_ACTION_QUEUE_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <tr1/functional>

namespace paludis
{
    /**
     * An ActionQueue consists of a number of threads that take tasks from a
     * queue.
     *
     * If threads are disabled, enqueueing an item executes it immediately.
     *
     * \ingroup g_threads
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ActionQueue :
        private PrivateImplementationPattern<ActionQueue>
    {
        public:
            ///\name Basic operations
            ///\{

            ActionQueue(const unsigned n_threads = 1, const bool nice = false, const bool limit_size = true);
            ~ActionQueue();

            ///\}

            /**
             * Enqueue an item.
             */
            void enqueue(const std::tr1::function<void () throw ()> &);

            /**
             * Complete any pending tasks.
             */
            void complete_pending();

            /**
             * Forget any pending tasks.
             */
            void forget_pending();

            /**
             * How many threads do we have?
             */
            unsigned number_of_threads() const;
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<ActionQueue>;
#endif

}

#endif
