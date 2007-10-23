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

#include <paludis/util/graph.hh>
#include <paludis/util/graph-impl.hh>

using namespace paludis;

GraphError::GraphError(const std::string & msg) throw () :
    Exception(msg)
{
}

NoGraphTopologicalOrderExistsError::NoGraphTopologicalOrderExistsError(
        tr1::shared_ptr<const RemainingNodes> r) throw () :
    GraphError("No topological order exists"),
    _remaining_nodes(r)
{
}

tr1::shared_ptr<const NoGraphTopologicalOrderExistsError::RemainingNodes>
NoGraphTopologicalOrderExistsError::remaining_nodes() const
{
    return _remaining_nodes;
}

NoGraphTopologicalOrderExistsError::~NoGraphTopologicalOrderExistsError() throw ()
{
}

