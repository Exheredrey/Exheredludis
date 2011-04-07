/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/dep_spec_data.hh>
#include <paludis/package_dep_spec_requirement.hh>
#include <paludis/dep_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>

#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>

#include <istream>
#include <ostream>

using namespace paludis;

#include <paludis/dep_spec_data-se.cc>

ConditionalDepSpecData::~ConditionalDepSpecData() = default;

namespace paludis
{
    template <>
    struct Imp<PackageDepSpecData>
    {
        std::shared_ptr<const NameRequirement> package;
        std::shared_ptr<const PackageNamePartRequirement> package_name_part;
        std::shared_ptr<const CategoryNamePartRequirement> category_name_part;
        std::shared_ptr<VersionRequirementSequence> all_versions;
        std::shared_ptr<const AnySlotRequirement> any_slot;
        std::shared_ptr<const ExactSlotRequirement> exact_slot;
        std::shared_ptr<const InRepositoryRequirement> in_repository;
        std::shared_ptr<const FromRepositoryRequirement> from_repository;
        std::shared_ptr<const InstallableToRepositoryRequirement> installable_to_repository;
        std::shared_ptr<const InstalledAtPathRequirement> installed_at_path;
        std::shared_ptr<const InstallableToPathRequirement> installable_to_path;
        std::shared_ptr<KeyRequirementSequence> all_keys;
        std::shared_ptr<ChoiceRequirementSequence> all_choices;
        PackageDepSpecDataOptions options;

        Imp(const PackageDepSpecDataOptions & o) :
            options(o)
        {
        }

        Imp(const PackageDepSpecData & other) :
            package(other.package_name_requirement()),
            package_name_part(other.package_name_part_requirement()),
            category_name_part(other.category_name_part_requirement()),
            all_versions(other.all_version_requirements() ? new VersionRequirementSequence : 0),
            any_slot(other.any_slot_requirement()),
            exact_slot(other.exact_slot_requirement()),
            in_repository(other.in_repository_requirement()),
            from_repository(other.from_repository_requirement()),
            installable_to_repository(other.installable_to_repository_requirement()),
            installed_at_path(other.installed_at_path_requirement()),
            installable_to_path(other.installable_to_path_requirement()),
            all_keys(other.all_key_requirements() ? new KeyRequirementSequence : 0),
            all_choices(other.all_choice_requirements() ? new ChoiceRequirementSequence : 0),
            options(other.options())
        {
            if (all_versions)
                std::copy(other.all_version_requirements()->begin(), other.all_version_requirements()->end(),
                        all_versions->back_inserter());

            if (all_keys)
                std::copy(other.all_key_requirements()->begin(), other.all_key_requirements()->end(),
                        all_keys->back_inserter());

            if (all_choices)
                std::copy(other.all_choice_requirements()->begin(), other.all_choice_requirements()->end(),
                        all_choices->back_inserter());
        }
    };
}

PackageDepSpecData::PackageDepSpecData(const PackageDepSpecDataOptions & o) :
    _imp(o)
{
}

PackageDepSpecData::PackageDepSpecData(const PackageDepSpecData & o) :
    _imp(o)
{
}

PackageDepSpecData::~PackageDepSpecData() = default;

const std::shared_ptr<const NameRequirement>
PackageDepSpecData::package_name_requirement() const
{
    return _imp->package;
}

const std::shared_ptr<const PackageNamePartRequirement>
PackageDepSpecData::package_name_part_requirement() const
{
    return _imp->package_name_part;
}

const std::shared_ptr<const CategoryNamePartRequirement>
PackageDepSpecData::category_name_part_requirement() const
{
    return _imp->category_name_part;
}

const std::shared_ptr<const VersionRequirementSequence>
PackageDepSpecData::all_version_requirements() const
{
    return _imp->all_versions;
}

const std::shared_ptr<const ExactSlotRequirement>
PackageDepSpecData::exact_slot_requirement() const
{
    return _imp->exact_slot;
}

const std::shared_ptr<const AnySlotRequirement>
PackageDepSpecData::any_slot_requirement() const
{
    return _imp->any_slot;
}

const std::shared_ptr<const InRepositoryRequirement>
PackageDepSpecData::in_repository_requirement() const
{
    return _imp->in_repository;
}

const std::shared_ptr<const InstallableToRepositoryRequirement>
PackageDepSpecData::installable_to_repository_requirement() const
{
    return _imp->installable_to_repository;
}

const std::shared_ptr<const FromRepositoryRequirement>
PackageDepSpecData::from_repository_requirement() const
{
    return _imp->from_repository;
}

const std::shared_ptr<const InstalledAtPathRequirement>
PackageDepSpecData::installed_at_path_requirement() const
{
    return _imp->installed_at_path;
}

const std::shared_ptr<const InstallableToPathRequirement>
PackageDepSpecData::installable_to_path_requirement() const
{
    return _imp->installable_to_path;
}

const std::shared_ptr<const KeyRequirementSequence>
PackageDepSpecData::all_key_requirements() const
{
    return _imp->all_keys;
}

const std::shared_ptr<const ChoiceRequirementSequence>
PackageDepSpecData::all_choice_requirements() const
{
    return _imp->all_choices;
}

const PackageDepSpecDataOptions
PackageDepSpecData::options() const
{
    return _imp->options;
}

std::string
PackageDepSpecData::as_string() const
{
    std::ostringstream s;

    if (all_version_requirements())
    {
        if (all_version_requirements()->begin() == all_version_requirements()->end())
        {
        }
        else if (next(all_version_requirements()->begin()) == all_version_requirements()->end() &&
                ! options()[pdsdo_always_use_ranged_deps])
        {
            if ((*all_version_requirements()->begin())->version_operator() == vo_stupid_equal_star ||
                    (*all_version_requirements()->begin())->version_operator() == vo_nice_equal_star)
                s << "=";
            else
                s << (*all_version_requirements()->begin())->version_operator();
        }
    }

    if (package_name_requirement())
        s << package_name_requirement()->name();
    else
    {
        if (category_name_part_requirement())
            s << category_name_part_requirement()->name_part();
        else
            s << "*";

        s << "/";

        if (package_name_part_requirement())
            s << package_name_part_requirement()->name_part();
        else
            s << "*";
    }

    if (all_version_requirements())
    {
        if (all_version_requirements()->begin() == all_version_requirements()->end())
        {
        }
        else if (next(all_version_requirements()->begin()) == all_version_requirements()->end() &&
                ! options()[pdsdo_always_use_ranged_deps])
        {
            s << "-" << (*all_version_requirements()->begin())->version_spec();
            if ((*all_version_requirements()->begin())->version_operator() == vo_stupid_equal_star ||
                    (*all_version_requirements()->begin())->version_operator() == vo_nice_equal_star)
                s << "*";
        }
    }

    if (exact_slot_requirement())
    {
        if (exact_slot_requirement()->locked())
            s << ":=";
        else
            s << ":";

        s << stringify(exact_slot_requirement()->name());
    }

    if (any_slot_requirement())
    {
        if (any_slot_requirement()->locking())
            s << ":=";
        else
            s << ":*";
    }

    std::string left, right;
    bool need_arrow(false);

    if (from_repository_requirement())
        left = stringify(from_repository_requirement()->name());

    if (in_repository_requirement())
        right = stringify(in_repository_requirement()->name());

    if (installed_at_path_requirement())
    {
        if (! right.empty())
        {
            need_arrow = true;
            right.append("->");
        }
        right.append(stringify(installed_at_path_requirement()->path()));
    }

    if (installable_to_repository_requirement())
    {
        if (! right.empty())
        {
            need_arrow = true;
            right.append("->");
        }
        if (installable_to_repository_requirement()->include_masked())
            right.append(stringify(installable_to_repository_requirement()->name()) + "??");
        else
            right.append(stringify(installable_to_repository_requirement()->name()) + "?");
    }

    if (installable_to_path_requirement())
    {
        if (! right.empty())
        {
            need_arrow = true;
            right.append("->");
        }
        if (installable_to_path_requirement()->include_masked())
            right.append(stringify(installable_to_path_requirement()->path()) + "??");
        else
            right.append(stringify(installable_to_path_requirement()->path()) + "?");
    }

    if (need_arrow || ((! left.empty()) && (! right.empty())))
        s << "::" << left << "->" << right;
    else if (! right.empty())
        s << "::" << right;
    else if (! left.empty())
        s << "::" << left << "->";

    if (all_version_requirements())
    {
        if (all_version_requirements()->begin() == all_version_requirements()->end())
        {
        }
        else if (next(all_version_requirements()->begin()) == all_version_requirements()->end() &&
                ! options()[pdsdo_always_use_ranged_deps])
        {
        }
        else
        {
            bool need_op(false);
            s << "[";
            for (auto r(all_version_requirements()->begin()), r_end(all_version_requirements()->end()) ; r != r_end ; ++r)
            {
                if (need_op)
                {
                    do
                    {
                        switch ((*r)->combiner())
                        {
                            case vrc_and:
                                s << "&";
                                continue;

                            case vrc_or:
                                s << "|";
                                continue;

                            case last_vrc:
                                ;
                        }
                        throw InternalError(PALUDIS_HERE, "Bad version_requirements_mode");
                    } while (false);
                }

                if ((*r)->version_operator() == vo_stupid_equal_star || (*r)->version_operator() == vo_nice_equal_star)
                    s << "=";
                else
                    s << (*r)->version_operator();

                s << (*r)->version_spec();

                if ((*r)->version_operator() == vo_stupid_equal_star || (*r)->version_operator() == vo_nice_equal_star)
                    s << "*";

                need_op = true;
            }
            s << "]";
        }
    }

    if (all_choice_requirements())
        for (auto u(all_choice_requirements()->begin()), u_end(all_choice_requirements()->end()) ; u != u_end ; ++u)
            s << (*u)->as_raw_string();

    if (all_key_requirements())
        for (auto u(all_key_requirements()->begin()), u_end(all_key_requirements()->end()) ; u != u_end ; ++u)
            s << (*u)->as_raw_string();

    return s.str();
}


MutablePackageDepSpecData::MutablePackageDepSpecData(const PackageDepSpecDataOptions & o) :
    PackageDepSpecData(o)
{
}

MutablePackageDepSpecData::MutablePackageDepSpecData(const PackageDepSpecData & o) :
    PackageDepSpecData(o)
{
}

MutablePackageDepSpecData::MutablePackageDepSpecData(const MutablePackageDepSpecData & o) :
    PackageDepSpecData(o)
{
}

MutablePackageDepSpecData::~MutablePackageDepSpecData() = default;

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_package(const QualifiedPackageName & name)
{
    _imp->package = NameRequirementPool::get_instance()->create(name);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_package()
{
    _imp->package.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_package_name_part(const PackageNamePart & part)
{
    _imp->package_name_part = PackageNamePartRequirementPool::get_instance()->create(part);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_package_name_part()
{
    _imp->package_name_part.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_category_name_part(const CategoryNamePart & part)
{
    _imp->category_name_part = CategoryNamePartRequirementPool::get_instance()->create(part);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_category_name_part()
{
    _imp->category_name_part.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_version(const VersionRequirementCombiner vc, const VersionOperator & vo, const VersionSpec & vs)
{
    if (! _imp->all_versions)
        _imp->all_versions = std::make_shared<VersionRequirementSequence>();
    _imp->all_versions->push_back(std::make_shared<VersionRequirement>(vs, vo, vc));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_versions()
{
    _imp->all_versions.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_exact_slot(const SlotName & n, const bool s)
{
    _imp->exact_slot = ExactSlotRequirementPool::get_instance()->create(n, s);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_exact_slot()
{
    _imp->exact_slot.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_in_repository(const RepositoryName & s)
{
    _imp->in_repository = InRepositoryRequirementPool::get_instance()->create(s);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_in_repository()
{
    _imp->in_repository.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_installable_to_path(const FSPath & s, const bool i)
{
    _imp->installable_to_path = InstallableToPathRequirementPool::get_instance()->create(s, i);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_installable_to_path()
{
    _imp->installable_to_path.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_installable_to_repository(const RepositoryName & n, const bool i)
{
    _imp->installable_to_repository = InstallableToRepositoryRequirementPool::get_instance()->create(n, i);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_installable_to_repository()
{
    _imp->installable_to_repository.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_from_repository(const RepositoryName & n)
{
    _imp->from_repository = FromRepositoryRequirementPool::get_instance()->create(n);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_from_repository()
{
    _imp->from_repository.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_installed_at_path(const FSPath & s)
{
    _imp->installed_at_path = InstalledAtPathRequirementPool::get_instance()->create(s);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_installed_at_path()
{
    _imp->installed_at_path.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_any_slot(const bool s)
{
    _imp->any_slot = AnySlotRequirementPool::get_instance()->create(s);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_any_slot()
{
    _imp->any_slot.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_choice(const std::shared_ptr<const ChoiceRequirement> & c)
{
    if (! _imp->all_choices)
        _imp->all_choices = std::make_shared<ChoiceRequirementSequence>();
    _imp->all_choices->push_back(c);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_choices()
{
    _imp->all_choices.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_key(const KeyRequirementKeyType t, const std::string & k, const KeyRequirementOperation o, const std::string & p)
{
    if (! _imp->all_keys)
        _imp->all_keys = std::make_shared<KeyRequirementSequence>();
    _imp->all_keys->push_back(KeyRequirementPool::get_instance()->create(t, k, o, p));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_keys()
{
    _imp->all_keys.reset();
    return *this;
}

MutablePackageDepSpecData::operator PackageDepSpec() const
{
    /* convoluted because it's private... */
    PackageDepSpecData * data(new MutablePackageDepSpecData(*this));
    return PackageDepSpec(std::shared_ptr<PackageDepSpecData>(data));
}
