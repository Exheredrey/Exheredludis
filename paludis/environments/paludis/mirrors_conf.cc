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

#include "mirrors_conf.hh"
#include <paludis/environment.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/name.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <algorithm>
#include <vector>

using namespace paludis;
using namespace paludis::paludis_environment;

typedef MakeHashedMultiMap<std::string, std::string>::Type Mirrors;

namespace paludis
{
    template<>
    struct Implementation<MirrorsConf>
    {
        const PaludisEnvironment * const env;
        Mirrors mirrors;

        Implementation(const PaludisEnvironment * const e) :
            env(e)
        {
        }
    };
}

MirrorsConf::MirrorsConf(const PaludisEnvironment * const e) :
    PrivateImplementationPattern<MirrorsConf>(new Implementation<MirrorsConf>(e))
{
}

MirrorsConf::~MirrorsConf()
{
}

void
MirrorsConf::add(const FSEntry & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as a mirrors file:");

    tr1::shared_ptr<LineConfigFile> f(make_bashable_conf(filename));
    if (! f)
        return;

    for (LineConfigFile::ConstIterator line(f->begin()), line_end(f->end()) ;
            line != line_end ; ++line)
    {
        std::vector<std::string> tokens;
        tokenise_whitespace_quoted(*line, std::back_inserter(tokens));

        if (tokens.size() < 2)
            continue;

        for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                t != t_end ; ++t)
            _imp->mirrors.insert(std::make_pair(tokens.at(0), *t));
    }
}

tr1::shared_ptr<const MirrorsSequence>
MirrorsConf::query(const std::string & m) const
{
    tr1::shared_ptr<MirrorsSequence> result(new MirrorsSequence);
    std::pair<Mirrors::const_iterator, Mirrors::const_iterator> p(_imp->mirrors.equal_range(m));
    std::transform(p.first, p.second, result->back_inserter(),
            paludis::tr1::mem_fn(&std::pair<const std::string, std::string>::second));
    return result;
}


