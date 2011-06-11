/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2011 Ciaran McCreesh
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

#include <paludis/elike_slot_requirement.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;

ELikeSlotExactRequirement::ELikeSlotExactRequirement(const SlotName & s, const bool e) :
    _s(s),
    _e(e)
{
}

const std::string
ELikeSlotExactRequirement::as_string() const
{
    return ":" + std::string(_e ? "=" : "") + stringify(_s);
}

const SlotName
ELikeSlotExactRequirement::slot() const
{
    return _s;
}

bool
ELikeSlotExactRequirement::from_any_locked() const
{
    return _e;
}

const std::string
ELikeSlotAnyUnlockedRequirement::as_string() const
{
    return ":*";
}

const std::string
ELikeSlotAnyLockedRequirement::as_string() const
{
    return ":=";
}

