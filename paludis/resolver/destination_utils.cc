/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/resolver/destination_utils.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/generator_handler.hh>
#include <paludis/filter.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>

using namespace paludis;
using namespace paludis::resolver;

bool
paludis::resolver::can_make_binary_for(const std::shared_ptr<const PackageID> & id)
{
    if (! id->behaviours_key())
        return true;
    return id->behaviours_key()->value()->end() == id->behaviours_key()->value()->find("unbinaryable");
}

bool
paludis::resolver::is_already_binary(const std::shared_ptr<const PackageID> & id)
{
    if (! id->behaviours_key())
        return false;
    return id->behaviours_key()->value()->end() != id->behaviours_key()->value()->find("binary");
}

bool
paludis::resolver::can_chroot(const std::shared_ptr<const PackageID> & id)
{
    if (! id->behaviours_key())
        return true;
    return id->behaviours_key()->value()->end() == id->behaviours_key()->value()->find("unchrootable");
}

namespace
{
    struct BinaryDestinationGeneratorHandler :
        AllGeneratorHandlerBase
    {
        virtual std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env,
                const RepositoryContentMayExcludes &) const
        {
            using namespace std::placeholders;
            std::shared_ptr<RepositoryNameSet> result(std::make_shared<RepositoryNameSet>());
            for (auto r(env->package_database()->begin_repositories()),
                    r_end(env->package_database()->end_repositories()) ;
                    r != r_end ; ++r)
                if (! (*r)->installed_root_key())
                    if ((*r)->destination_interface())
                        result->insert((*r)->name());

            return result;
        }

        virtual std::string as_string() const
        {
            return "binary destination repositories";
        }
    };

    struct BinaryDestinationGenerator :
        Generator
    {
        BinaryDestinationGenerator() :
            Generator(std::make_shared<BinaryDestinationGeneratorHandler>())
        {
        }
    };
}

FilteredGenerator
paludis::resolver::destination_filtered_generator(
        const Environment * const env,
        const DestinationType t,
        const Generator & g)
{
    switch (t)
    {
        case dt_install_to_slash:
            return g | filter::InstalledAtRoot(env->system_root_key()->value());

        case dt_install_to_chroot:
            return g | filter::InstalledNotAtRoot(env->system_root_key()->value());

        case dt_create_binary:
            return g & BinaryDestinationGenerator();

        case last_dt:
            break;
    }

    throw InternalError(PALUDIS_HERE, "unhandled dt");
}

