/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 David Leverton
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

#include <paludis/util/upper_lower.hh>
#include <cctype>
#include <algorithm>

namespace paludis
{
    std::string toupper(const std::string & s)
    {
        std::string r;
        std::transform(s.begin(), s.end(), std::back_inserter(r), static_cast<int (*)(int)>(std::toupper));
        return r;
    }

    std::string tolower(const std::string & s)
    {
        std::string r;
        std::transform(s.begin(), s.end(), std::back_inserter(r), static_cast<int (*)(int)>(std::tolower));
        return r;
    }
}



