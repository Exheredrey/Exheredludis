/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "use_flag_pretty_printer.hh"
#include <paludis/version_metadata.hh>
#include <paludis/environment.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/collection_concrete.hh>
#include "colour.hh"
#include <iostream>
#include <set>

using namespace paludis;

UseFlagPrettyPrinter::UseFlagPrettyPrinter(const Environment * const e) :
    _env(e),
    _need_space(false),
    _new_flags(new UseFlagNameCollection::Concrete),
    _changed_flags(new UseFlagNameCollection::Concrete),
    _unchanged_flags(new UseFlagNameCollection::Concrete),
    _expand_prefixes(new UseFlagNameCollection::Concrete)
{
}

UseFlagPrettyPrinter::~UseFlagPrettyPrinter()
{
}

std::string::size_type
UseFlagPrettyPrinter::use_expand_delim_pos(const UseFlagName & u,
        const UseFlagNameCollection::ConstPointer c) const
{
    for (UseFlagNameCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
        if (0 == u.data().compare(0, i->data().length(), i->data(), 0, i->data().length()))
            return i->data().length();
    return std::string::npos;
}

void
UseFlagPrettyPrinter::print_package_flags(const PackageDatabaseEntry & pkg,
        const PackageDatabaseEntry * const old_pkg)
{
    std::set<UseFlagName> iuse, old_iuse;

    VersionMetadata::ConstPointer metadata(environment()->package_database()->
            fetch_repository(pkg.repository)->version_metadata(pkg.name, pkg.version));

    if (! metadata->ebuild_interface)
        return;

    WhitespaceTokeniser::get_instance()->tokenise(metadata->ebuild_interface->iuse,
            create_inserter<UseFlagName>(std::inserter(iuse, iuse.begin())));

    if (old_pkg)
    {
        VersionMetadata::ConstPointer old_metadata(environment()->package_database()->
                fetch_repository(old_pkg->repository)->version_metadata(old_pkg->name, old_pkg->version));
        if (old_metadata->ebuild_interface)
            WhitespaceTokeniser::get_instance()->tokenise(old_metadata->ebuild_interface->iuse,
                    create_inserter<UseFlagName>(std::inserter(old_iuse, old_iuse.begin())));
    }

    const RepositoryUseInterface * const use_interface(environment()->package_database()->
            fetch_repository(pkg.repository)->use_interface);

    /* first pass: only non-expand flags */
    for (std::set<UseFlagName>::const_iterator flag(iuse.begin()), flag_end(iuse.end()) ;
            flag != flag_end ; ++flag)
    {
        if (use_interface)
            if (std::string::npos != use_expand_delim_pos(*flag, use_interface->use_expand_prefixes()))
                continue;

        if (environment()->query_use(*flag, &pkg))
        {
            if (use_interface && use_interface->query_use_force(*flag, &pkg))
                output_flag(render_as_forced_flag(stringify(*flag)));
            else
                output_flag(render_as_enabled_flag(stringify(*flag)));
        }
        else
        {
            if (use_interface && use_interface->query_use_mask(*flag, &pkg))
                output_flag(render_as_masked_flag(stringify(*flag)));
            else
                output_flag(render_as_disabled_flag(stringify(*flag)));
        }

        if (old_pkg)
        {
            if (old_iuse.end() != old_iuse.find(*flag))
            {
                if (environment()->query_use(*flag, &pkg) != environment()->query_use(*flag, old_pkg))
                {
                    _changed_flags->insert(*flag);
                    output_flag_changed_mark();
                }
                else
                    _unchanged_flags->insert(*flag);
            }
            else
            {
                _new_flags->insert(*flag);
                output_flag_is_new_mark();
            }
        }
        else
            _new_flags->insert(*flag);
    }

    /* second pass: only expand flags */
    UseFlagName old_expand_name("OFTEN_NOT_BEEN_ON_BOATS");
    for (std::set<UseFlagName>::const_iterator flag(iuse.begin()), flag_end(iuse.end()) ;
            flag != flag_end ; ++flag)
    {
        std::string::size_type delim_pos(0);
        if (use_interface)
        {
            if (std::string::npos == ((delim_pos = use_expand_delim_pos(*flag,
                                use_interface->use_expand_prefixes()))))
                continue;
            if (use_interface->use_expand_hidden_prefixes()->count(UseFlagName(flag->data().substr(0, delim_pos))))
                continue;
        }

        UseFlagName expand_name(flag->data().substr(0, delim_pos)),
                    expand_value(flag->data().substr(delim_pos + 1));

        _expand_prefixes->insert(expand_name);

        if (expand_name != old_expand_name)
        {
            output_expand_prefix(stringify(expand_name));
            old_expand_name = expand_name;
        }

        if (environment()->query_use(*flag, &pkg))
        {
            if (use_interface && use_interface->query_use_force(*flag, &pkg))
                output_flag(render_as_forced_flag(stringify(expand_value)));
            else
                output_flag(render_as_enabled_flag(stringify(expand_value)));
        }
        else
        {
            if (use_interface && use_interface->query_use_mask(*flag, &pkg))
                output_flag(render_as_masked_flag(stringify(expand_value)));
            else
                output_flag(render_as_disabled_flag(stringify(expand_value)));
        }

        if (old_pkg)
        {
            if (old_iuse.end() != old_iuse.find(*flag))
            {
                if (environment()->query_use(*flag, &pkg) != environment()->query_use(*flag, old_pkg))
                {
                    _changed_flags->insert(*flag);
                    output_flag_changed_mark();
                }
                else
                    _unchanged_flags->insert(*flag);
            }
            else
            {
                _new_flags->insert(*flag);
                output_flag_is_new_mark();
            }
        }
        else
            _new_flags->insert(*flag);
    }
}

void
UseFlagPrettyPrinter::output_flag(const std::string & s)
{
    if (_need_space)
        output_stream() << " ";

    output_stream() << s;

    _need_space = true;
}

void
UseFlagPrettyPrinter::output_flag_changed_mark()
{
    output_stream() << "*";
}

void
UseFlagPrettyPrinter::output_flag_is_new_mark()
{
    output_stream() << "%";
}

void
UseFlagPrettyPrinter::output_expand_prefix(const std::string & s)
{
    if (_need_space)
        output_stream() << " ";

    output_stream() << s << ":";

    _need_space = true;
}

std::string
UseFlagPrettyPrinter::render_as_enabled_flag(const std::string & s) const
{
    return colour(cl_flag_on, s);
}

std::string
UseFlagPrettyPrinter::render_as_disabled_flag(const std::string & s) const
{
    return colour(cl_flag_off, "-" + s);
}

std::string
UseFlagPrettyPrinter::render_as_forced_flag(const std::string & s) const
{
    return colour(cl_flag_on, "(" + s + ")");
}

std::string
UseFlagPrettyPrinter::render_as_masked_flag(const std::string & s) const
{
    return colour(cl_flag_off, "(-" + s + ")");
}

std::ostream &
UseFlagPrettyPrinter::output_stream() const
{
    return std::cout;
}

bool
UseFlagPrettyPrinter::need_space() const
{
    return _need_space;
}

const Environment *
UseFlagPrettyPrinter::environment() const
{
    return _env;
}

UseFlagNameCollection::ConstPointer
UseFlagPrettyPrinter::new_flags() const
{
    return _new_flags;
}

UseFlagNameCollection::ConstPointer
UseFlagPrettyPrinter::changed_flags() const
{
    return _changed_flags;
}

UseFlagNameCollection::ConstPointer
UseFlagPrettyPrinter::unchanged_flags() const
{
    return _unchanged_flags;
}

UseFlagNameCollection::ConstPointer
UseFlagPrettyPrinter::expand_prefixes() const
{
    return _expand_prefixes;
}

