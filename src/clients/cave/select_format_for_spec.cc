/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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
#include <paludis/environment.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/selection.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>

using namespace paludis;
using namespace cave;

std::string
paludis::cave::select_format_for_spec(
        const std::shared_ptr<const Environment> & env,
        const PackageDepSpec & spec,
        const std::string & if_installed,
        const std::string & if_installable,
        const std::string & if_unavailable
        )
{
    if (! (*env)[selection::SomeArbitraryVersion(generator::Matches(spec, MatchPackageOptions()) | filter::InstalledAtRoot(FSEntry("/")))]->empty())
        return if_installed;
    if (! (*env)[selection::SomeArbitraryVersion(generator::Matches(spec, MatchPackageOptions()) | filter::SupportsAction<InstallAction>()
                | filter::NotMasked())]->empty())
        return if_installable;
    return if_unavailable;
}

