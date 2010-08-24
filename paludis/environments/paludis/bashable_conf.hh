/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_BASHABLE_CONF_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_BASHABLE_CONF_HH 1

#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/config_file-fwd.hh>
#include <paludis/util/map-fwd.hh>
#include <memory>
#include <string>

namespace paludis
{
    namespace paludis_environment
    {
        std::shared_ptr<LineConfigFile> make_bashable_conf(
                const FSPath &,
                const LineConfigFileOptions &);

        std::shared_ptr<KeyValueConfigFile> make_bashable_kv_conf(
                const FSPath &,
                const std::shared_ptr<const Map<std::string, std::string> > &,
                const KeyValueConfigFileOptions &);
    }
}

#endif
