/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "vdb_version_metadata.hh"
#include <paludis/portage_dep_parser.hh>

using namespace paludis;

VDBVersionMetadata::VDBVersionMetadata() :
    VersionMetadata(
        VersionMetadataBase(SlotName("unset"), "", "", "UNKNOWN"),
        VersionMetadataCapabilities::create()
        .deps_interface(this)
        .origins_interface(this)
        .ebuild_interface(this)
        .license_interface(this)
        .virtual_interface(0)
        .cran_interface(0)
        ),
    VersionMetadataDepsInterface(&PortageDepParser::parse_depend),
    VersionMetadataLicenseInterface(&PortageDepParser::parse_license)
{
}

VDBVersionMetadata::~VDBVersionMetadata()
{
}

VDBVirtualVersionMetadata::VDBVirtualVersionMetadata(const SlotName & s,
        const PackageDatabaseEntry & e) :
    VersionMetadata(
        VersionMetadataBase(s, "", "", "UNKNOWN"),
        VersionMetadataCapabilities::create()
        .deps_interface(this)
        .origins_interface(this)
        .ebuild_interface(this)
        .license_interface(this)
        .virtual_interface(this)
        .cran_interface(0)
        ),
    VersionMetadataDepsInterface(&PortageDepParser::parse_depend),
    VersionMetadataLicenseInterface(&PortageDepParser::parse_license),
    VersionMetadataVirtualInterface(e)
{
}

VDBVirtualVersionMetadata::~VDBVirtualVersionMetadata()
{
}

