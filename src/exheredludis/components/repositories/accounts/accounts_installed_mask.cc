/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/repositories/accounts/accounts_installed_mask.hh>

using namespace paludis;
using namespace paludis::accounts_repository;

char
AccountsInstalledMask::key() const
{
    return 'I';
}

const std::string
AccountsInstalledMask::description() const
{
    return "already installed";
}

const std::string
AccountsInstalledMask::explanation() const
{
    return "Manipulating accounts that are already present is not supported";
}

