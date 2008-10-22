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

#ifndef PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_E_EXTRA_DISTRIBUTION_DATA_HH
#define PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_E_EXTRA_DISTRIBUTION_DATA_HH 1

#include <paludis/distribution-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/set.hh>
#include <tr1/memory>
#include <string>

namespace paludis
{
    namespace n
    {
        struct default_buildroot;
        struct default_distdir;
        struct default_eapi_when_unknown;
        struct default_eapi_when_unspecified;
        struct default_layout;
        struct default_names_cache;
        struct default_profile_eapi;
        struct default_provides_cache;
        struct default_write_cache;
        struct qa_category_dir_checks;
        struct qa_eclass_file_contents_checks;
        struct qa_package_dir_checks;
        struct qa_package_id_checks;
        struct qa_package_id_file_contents_checks;
        struct qa_tree_checks;
    }

    namespace erepository
    {
        struct EDistribution
        {
            NamedValue<n::default_buildroot, std::string> default_buildroot;
            NamedValue<n::default_distdir, std::string> default_distdir;
            NamedValue<n::default_eapi_when_unknown, std::string> default_eapi_when_unknown;
            NamedValue<n::default_eapi_when_unspecified, std::string> default_eapi_when_unspecified;
            NamedValue<n::default_layout, std::string> default_layout;
            NamedValue<n::default_names_cache, std::string> default_names_cache;
            NamedValue<n::default_profile_eapi, std::string> default_profile_eapi;
            NamedValue<n::default_provides_cache, std::string> default_provides_cache;
            NamedValue<n::default_write_cache, std::string> default_write_cache;
            NamedValue<n::qa_category_dir_checks, std::tr1::shared_ptr<const Set<std::string> > > qa_category_dir_checks;
            NamedValue<n::qa_eclass_file_contents_checks, std::tr1::shared_ptr<const Set<std::string> > > qa_eclass_file_contents_checks;
            NamedValue<n::qa_package_dir_checks, std::tr1::shared_ptr<const Set<std::string> > > qa_package_dir_checks;
            NamedValue<n::qa_package_id_checks, std::tr1::shared_ptr<const Set<std::string> > > qa_package_id_checks;
            NamedValue<n::qa_package_id_file_contents_checks, std::tr1::shared_ptr<const Set<std::string> > > qa_package_id_file_contents_checks;
            NamedValue<n::qa_tree_checks, std::tr1::shared_ptr<const Set<std::string> > > qa_tree_checks;
        };

        typedef ExtraDistributionData<EDistribution> EExtraDistributionData;
    }
}

#endif
