/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/repositories/unavailable/unavailable_repository_dependencies_key.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/dep_label.hh>
#include <paludis/comma_separated_dep_parser.hh>
#include <paludis/comma_separated_dep_printer.hh>
#include <memory>

using namespace paludis;
using namespace paludis::unavailable_repository;

namespace paludis
{
    template <>
    struct Implementation<UnavailableRepositoryDependenciesKey>
    {
        const Environment * const env;
        const std::shared_ptr<const DependencySpecTree> value;
        const std::shared_ptr<DependenciesLabelSequence> labels;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const Environment * const e,
                const std::string & r, const std::string & h, const MetadataKeyType t,
                const std::string & v) :
            env(e),
            value(CommaSeparatedDepParser::parse(e, v)),
            labels(new DependenciesLabelSequence),
            raw_name(r),
            human_name(h),
            type(t)
        {
            labels->push_back(std::make_shared<DependenciesBuildLabel>("build", return_literal_function(true)));
        }
    };
}

UnavailableRepositoryDependenciesKey::UnavailableRepositoryDependenciesKey(const Environment * const env,
        const std::string & r, const std::string & h, const MetadataKeyType t,
        const std::string & v) :
    PrivateImplementationPattern<UnavailableRepositoryDependenciesKey>(
            new Implementation<UnavailableRepositoryDependenciesKey>(env, r, h, t, v))
{
}

UnavailableRepositoryDependenciesKey::~UnavailableRepositoryDependenciesKey()
{
}

const std::shared_ptr<const DependencySpecTree>
UnavailableRepositoryDependenciesKey::value() const
{
    return _imp->value;
}

const std::string
UnavailableRepositoryDependenciesKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
UnavailableRepositoryDependenciesKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
UnavailableRepositoryDependenciesKey::type() const
{
    return _imp->type;
}

std::string
UnavailableRepositoryDependenciesKey::pretty_print(const DependencySpecTree::ItemFormatter & f) const
{
    CommaSeparatedDepPrinter p(_imp->env, f, false);
    _imp->value->root()->accept(p);
    return p.result();
}

std::string
UnavailableRepositoryDependenciesKey::pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
{
    CommaSeparatedDepPrinter p(_imp->env, f, true);
    _imp->value->root()->accept(p);
    return p.result();
}

const std::shared_ptr<const DependenciesLabelSequence>
UnavailableRepositoryDependenciesKey::initial_labels() const
{
    return _imp->labels;
}

