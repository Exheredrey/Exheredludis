/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh
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

#include <exception>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <sstream>
#include <test/test_framework.hh>

/** \file
 * Extra settings for test cases.
 *
 */

using namespace paludis;

namespace
{
    /**
     * Convert an exception to a debug string.
     *
     */
    std::string verbose_exception_to_debug_string(
            const std::exception & e) PALUDIS_ATTRIBUTE((noinline));

    /**
     * Avoid logging visibly.
     *
     */
    struct C
    {
        /// Dummy stream.
        std::stringstream s;

        /// Constructor.
        C()
        {
            test::set_exception_to_debug_string(&verbose_exception_to_debug_string);
            Log::get_instance()->set_log_stream(&s);
        }
    };

    /**
     * Avoid logging visibly.
     *
     */
    static const C my_c;

    std::string verbose_exception_to_debug_string(const std::exception & e)
    {
        const paludis::Exception * ee;
        if (0 != ((ee = dynamic_cast<const Exception *>(&e))))
            return stringify(ee->what()) + " (message " + ee->message() +
                (ee->empty() ? stringify("") : ", backtrace " + ee->backtrace(" -> ")) + ")";
        else
            return e.what();
    }
}

