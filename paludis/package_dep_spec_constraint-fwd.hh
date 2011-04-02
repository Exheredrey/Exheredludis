/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_CONSTRAINT_FWD_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_CONSTRAINT_FWD_HH 1

#include <paludis/util/pool-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/sequence-fwd.hh>
#include <iosfwd>
#include <memory>

namespace paludis
{
    class PackageDepSpecConstraint;

    class NameConstraint;
    typedef Pool<NameConstraint> NameConstraintPool;

    class PackageNamePartConstraint;
    typedef Pool<PackageNamePartConstraint> PackageNamePartConstraintPool;

    class CategoryNamePartConstraint;
    typedef Pool<CategoryNamePartConstraint> CategoryNamePartConstraintPool;

    class VersionConstraint;

    typedef Sequence<std::shared_ptr<const VersionConstraint> > VersionConstraintSequence;

    class InRepositoryConstraint;
    typedef Pool<InRepositoryConstraint> InRepositoryConstraintPool;

    class FromRepositoryConstraint;
    typedef Pool<FromRepositoryConstraint> FromRepositoryConstraintPool;

    class InstalledAtPathConstraint;
    typedef Pool<InstalledAtPathConstraint> InstalledAtPathConstraintPool;

    class InstallableToPathConstraint;
    typedef Pool<InstallableToPathConstraint> InstallableToPathConstraintPool;

    class InstallableToRepositoryConstraint;
    typedef Pool<InstallableToRepositoryConstraint> InstallableToRepositoryConstraintPool;

    class ExactSlotConstraint;
    typedef Pool<ExactSlotConstraint> ExactSlotConstraintPool;

    class AnySlotConstraint;
    typedef Pool<AnySlotConstraint> AnySlotConstraintPool;

    class KeyConstraint;
    typedef Pool<KeyConstraint> KeyConstraintPool;

    typedef Sequence<std::shared_ptr<const KeyConstraint> > KeyConstraintSequence;

#include <paludis/package_dep_spec_constraint-se.hh>

}

#endif