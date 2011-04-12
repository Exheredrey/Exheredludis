/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011 Ciaran McCreesh
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

#include "select_format_for_spec.hh"
#include "format_user_config.hh"
#include <paludis/environment.hh>
#include <paludis/util/sequence.hh>
#include <paludis/selection.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/metadata_key.hh>

using namespace paludis;
using namespace cave;

template <typename T_>
T_
paludis::cave::select_format_for_spec(
        const std::shared_ptr<const Environment> & env,
        const PackageDepSpec & spec,
        const std::shared_ptr<const PackageID> & from_id,
        const T_ & if_installed,
        const T_ & if_installable,
        const T_ & if_unavailable
        )
{
    if (! (*env)[selection::SomeArbitraryVersion(generator::Matches(spec, from_id, { }) | filter::InstalledAtRoot(env->system_root_key()->parse_value()))]->empty())
        return if_installed;
    if (! (*env)[selection::SomeArbitraryVersion(generator::Matches(spec, from_id, { }) | filter::SupportsAction<InstallAction>()
                | filter::NotMasked())]->empty())
        return if_installable;
    return if_unavailable;
}

template std::string paludis::cave::select_format_for_spec(
        const std::shared_ptr<const Environment> & env,
        const PackageDepSpec & spec,
        const std::shared_ptr<const PackageID> & from_id,
        const std::string & if_installed,
        const std::string & if_installable,
        const std::string & if_unavailable
        );

template FormatString<'i', 's'> paludis::cave::select_format_for_spec(
        const std::shared_ptr<const Environment> & env,
        const PackageDepSpec & spec,
        const std::shared_ptr<const PackageID> & from_id,
        const FormatString<'i', 's'> & if_installed,
        const FormatString<'i', 's'> & if_installable,
        const FormatString<'i', 's'> & if_unavailable
        );

template FormatString<'a', 'i', 's'> paludis::cave::select_format_for_spec(
        const std::shared_ptr<const Environment> & env,
        const PackageDepSpec & spec,
        const std::shared_ptr<const PackageID> & from_id,
        const FormatString<'a', 'i', 's'> & if_installed,
        const FormatString<'a', 'i', 's'> & if_installable,
        const FormatString<'a', 'i', 's'> & if_unavailable
        );

template FormatString<'s'> paludis::cave::select_format_for_spec(
        const std::shared_ptr<const Environment> & env,
        const PackageDepSpec & spec,
        const std::shared_ptr<const PackageID> & from_id,
        const FormatString<'s'> & if_installed,
        const FormatString<'s'> & if_installable,
        const FormatString<'s'> & if_unavailable
        );

