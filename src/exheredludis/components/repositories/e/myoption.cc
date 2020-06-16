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

#include <paludis/repositories/e/myoption.hh>
#include <paludis/choice.hh>

using namespace paludis;
using namespace paludis::erepository;

MyOptionsError::MyOptionsError(const std::string & m) noexcept :
    Exception(m)
{
}

std::pair<UnprefixedChoiceName, bool>
paludis::erepository::parse_myoption(const std::string & s)
{
    if ((! s.empty()) && '-' == s.at(0))
        return std::make_pair(UnprefixedChoiceName(s.substr(1)), false);
    else
        return std::make_pair(UnprefixedChoiceName(s), true);
}

