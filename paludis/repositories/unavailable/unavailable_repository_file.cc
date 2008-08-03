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

#include <paludis/repositories/unavailable/unavailable_repository_file.hh>
#include <paludis/repositories/unavailable/unavailable_repository.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/simple_parser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <list>
#include <fstream>

using namespace paludis;
using namespace paludis::unavailable_repository;

namespace paludis
{
    template <>
    struct Implementation<UnavailableRepositoryFile>
    {
        std::string repo_name, homepage, description;
        std::list<UnavailableRepositoryFileEntry> entries;
    };
}

UnavailableRepositoryFile::UnavailableRepositoryFile(const FSEntry & f) :
    PrivateImplementationPattern<UnavailableRepositoryFile>(new Implementation<UnavailableRepositoryFile>)
{
    _load(f);
}

UnavailableRepositoryFile::~UnavailableRepositoryFile()
{
}

UnavailableRepositoryFile::ConstIterator
UnavailableRepositoryFile::begin() const
{
    return ConstIterator(_imp->entries.begin());
}

UnavailableRepositoryFile::ConstIterator
UnavailableRepositoryFile::end() const
{
    return ConstIterator(_imp->entries.end());
}

void
UnavailableRepositoryFile::_load(const FSEntry & f)
{
    std::ifstream file(stringify(f).c_str());
    if (! file)
        throw UnavailableRepositoryConfigurationError("Cannot read '" + stringify(f) + "'");

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            break;

        std::string key, value;
        SimpleParser line_parser(line);
        if (line_parser.consume(
                    (+simple_parser::any_except(" \t") >> key) &
                    (*simple_parser::any_of(" \t")) &
                    (simple_parser::exact("=")) &
                    (*simple_parser::any_of(" \t")) &
                    (*simple_parser::any_except("") >> value)
                    ))
        {
            if (key == "format")
            {
                if (value != "unavailable-1")
                    throw UnavailableRepositoryConfigurationError(
                            "Unsupported format '" + value + "' in '" + stringify(f) + "'");
            }
            else if (key == "repo_name")
                _imp->repo_name = value;
            else if (key == "homepage")
                _imp->homepage = value;
            else if (key == "description")
                _imp->description = value;
            else
                Log::get_instance()->message("unavailable_repository.file.unknown_key", ll_warning, lc_context)
                    << "Ignoring unknown key '" << key << "' with value '" << value << "'";
        }
        else
            throw UnavailableRepositoryConfigurationError(
                    "Cannot parse header line '" + line + "' in '" + stringify(f) + "'");
    }

    CategoryNamePart category("x");
    PackageNamePart package("x");
    SlotName slot("x");
    while (std::getline(file, line))
    {
        SimpleParser line_parser(line);

        std::string token;
        if (line_parser.consume(
                    (+simple_parser::any_except(" \t/") >> token) &
                    (simple_parser::exact("/"))
                    ))
        {
            if (! line_parser.eof())
                throw UnavailableRepositoryConfigurationError(
                        "Cannot parse body category line '" + line + " in '" + stringify(f) + "'");

            category = CategoryNamePart(token);
        }
        else if (line_parser.consume(
                    (+simple_parser::any_of(" \t")) &
                    (+simple_parser::any_except(" \t/") >> token) &
                    (simple_parser::exact("/"))
                    ))
        {
            if (! line_parser.eof())
                throw UnavailableRepositoryConfigurationError(
                        "Cannot parse body package line '" + line + " in '" + stringify(f) + "'");

            package = PackageNamePart(token);
        }
        else if (line_parser.consume(
                    (+simple_parser::any_of(" \t")) &
                    (+simple_parser::exact(":")) &
                    (+simple_parser::any_except(" \t") >> token) &
                    (+simple_parser::any_of(" \t"))
                    ))
        {
            slot = SlotName(token);

            std::list<VersionSpec> versions;
            while (true)
            {
                if (line_parser.consume(
                            (+simple_parser::exact(";")) &
                            (*simple_parser::any_of(" \t"))
                            ))
                    break;
                else if (line_parser.consume(
                        (+simple_parser::any_except(" \t") >> token) &
                        (+simple_parser::any_of(" \t"))
                        ))
                    versions.push_back(VersionSpec(token));
                else
                    throw UnavailableRepositoryConfigurationError(
                            "Cannot parse body version line '" + line + " in '" + stringify(f) + "'");
            }

            if (versions.empty())
                throw UnavailableRepositoryConfigurationError(
                        "Cannot parse body version line '" + line + " in '" + stringify(f) + "'");

            if (! line_parser.consume(
                        (*simple_parser::any_of(" \t")) &
                        (+simple_parser::any_except("") >> token)
                        ))
                throw UnavailableRepositoryConfigurationError(
                        "Cannot parse body description line '" + line + " in '" + stringify(f) + "'");

            if (! line_parser.eof())
                throw UnavailableRepositoryConfigurationError(
                        "Cannot parse body description line '" + line + " in '" + stringify(f) + "'");

            std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > desc(
                    new LiteralMetadataValueKey<std::string>("DESCRIPTION", "Description", mkt_significant,
                        token));
            for (std::list<VersionSpec>::const_iterator v(versions.begin()), v_end(versions.end()) ;
                    v != v_end ; ++v)
                _imp->entries.push_back(make_named_values<UnavailableRepositoryFileEntry>(
                            value_for<n::description>(desc),
                            value_for<n::name>(category + package),
                            value_for<n::slot>(slot),
                            value_for<n::version>(*v)
                        ));
        }
        else
            throw UnavailableRepositoryConfigurationError(
                    "Cannot parse body line '" + line + " in '" + stringify(f) + "'");
    }
}

std::string
UnavailableRepositoryFile::repo_name() const
{
    return _imp->repo_name;
}

std::string
UnavailableRepositoryFile::homepage() const
{
    return _imp->homepage;
}

std::string
UnavailableRepositoryFile::description() const
{
    return _imp->description;
}

template class PrivateImplementationPattern<UnavailableRepositoryFile>;
template class WrappedForwardIterator<UnavailableRepositoryFile::ConstIteratorTag,
         const UnavailableRepositoryFileEntry>;

