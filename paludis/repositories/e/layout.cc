/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2006 Danny van Dyk
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

#include <paludis/repositories/e/layout.hh>
#include <paludis/repositories/e/traditional_layout.hh>
#include <paludis/repositories/e/exheres_layout.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/fs_path.hh>

using namespace paludis;
using namespace paludis::erepository;

Layout::Layout(const std::shared_ptr<const FSPathSequence> & l) :
    _master_repositories_locations(l)
{
}

Layout::~Layout() = default;

const std::shared_ptr<const FSPathSequence>
Layout::master_repositories_locations() const
{
    return _master_repositories_locations;
}

FSPath
Layout::sync_filter_file() const
{
    return FSPath("/dev/null");
}

namespace
{
    template <typename T_>
    std::shared_ptr<Layout>
    make_layout(const Environment * const e, const ERepository * const n, const FSPath & b,
            std::shared_ptr<const FSPathSequence> f)
    {
        return std::make_shared<T_>(e, n, b, f);
    }
}

LayoutFactory::LayoutFactory()
{
}

const std::shared_ptr<Layout>
LayoutFactory::create(
        const std::string & s,
        const Environment * const e,
        const ERepository * const r,
        const FSPath & f,
        const std::shared_ptr<const FSPathSequence> & ff) const
{
    if (s == "traditional")
        return make_layout<TraditionalLayout>(e, r, f, ff);
    if (s == "exheres")
        return make_layout<ExheresLayout>(e, r, f, ff);
    throw ConfigurationError("Unrecognised layout '" + s + "'");
}

namespace paludis
{
    template class Singleton<LayoutFactory>;

    template class Map<FSPath, std::string, FSPathComparator>;
    template class WrappedForwardIterator<Map<FSPath, std::string, FSPathComparator>::ConstIteratorTag, const std::pair<const FSPath, std::string> >;
}
