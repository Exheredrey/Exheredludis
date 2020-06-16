/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011, 2013 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DISTRIBUTION_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_DISTRIBUTION_IMPL_HH 1

#include <paludis/distribution.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/options.hh>
#include <paludis/util/pimp-impl.hh>
#include <unordered_map>
#include <mutex>

namespace paludis
{
    template <typename Data_>
    struct Imp<ExtraDistributionData<Data_> >
    {
        mutable std::mutex mutex;
        mutable std::unordered_map<std::string, std::shared_ptr<const Data_>, Hash<std::string> > values;
    };

    template <typename Data_>
    ExtraDistributionData<Data_>::ExtraDistributionData() :
        _imp()
    {
    }

    template <typename Data_>
    ExtraDistributionData<Data_>::~ExtraDistributionData() = default;

    template <typename T_>
    struct ExtraDistributionDataData;

    template <typename Data_>
    const std::shared_ptr<const Data_>
    ExtraDistributionData<Data_>::data_from_distribution(const Distribution & d) const
    {
        std::unique_lock<std::mutex> lock(this->_imp->mutex);
        typename std::unordered_map<std::string, std::shared_ptr<const Data_>, Hash<std::string> >::const_iterator v(
                this->_imp->values.find(d.name()));
        if (this->_imp->values.end() != v)
            return v->second;

        std::shared_ptr<KeyValueConfigFile> k(std::make_shared<KeyValueConfigFile>(d.extra_data_dir() / ExtraDistributionDataData<Data_>::config_file_name(),
                    KeyValueConfigFileOptions(), &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation));
        std::shared_ptr<const Data_> data(ExtraDistributionDataData<Data_>::make_data(k));
        this->_imp->values.insert(std::make_pair(d.name(), data));
        return data;
    }
}

#endif
