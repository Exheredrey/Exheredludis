/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_WORLD_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_WORLD_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/spec_tree.hh>
#include <paludis/environment-fwd.hh>
#include <memory>
#include <string>

namespace paludis
{
    namespace paludis_environment
    {
        class PALUDIS_VISIBLE World
        {
            private:
                Pimp<World> _imp;

                bool _add_string_to_world(const std::string &) const;
                bool _remove_string_from_world(const std::string &) const;

            public:
                World(const Environment * const, const std::shared_ptr<const FSPath> &);
                ~World();

                const std::shared_ptr<const SetSpecTree> world_set() const PALUDIS_ATTRIBUTE((warn_unused_result));

                bool add_to_world(const SetName &) const;
                bool add_to_world(const QualifiedPackageName &) const;

                bool remove_from_world(const SetName &) const;
                bool remove_from_world(const QualifiedPackageName &) const;

                std::shared_ptr<const FSPath> location_if_set() const;

                void update_config_files_for_package_move(
                        const PackageDepSpec &, const QualifiedPackageName &) const;
        };
    }
}

#endif
