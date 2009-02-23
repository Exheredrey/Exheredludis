/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#include <paludis/repositories/unwritten/unwritten_repository_store.hh>
#include <paludis/repositories/unwritten/unwritten_repository_file.hh>
#include <paludis/repositories/unwritten/unwritten_id.hh>
#include <paludis/repositories/unwritten/unwritten_mask.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <tr1/functional>
#include <tr1/unordered_map>
#include <algorithm>
#include <set>

using namespace paludis;
using namespace paludis::unwritten_repository;

typedef std::tr1::unordered_map<CategoryNamePart,
        std::tr1::shared_ptr<QualifiedPackageNameSet>,
        Hash<CategoryNamePart> > PackageNames;

typedef std::tr1::unordered_map<QualifiedPackageName,
        std::tr1::shared_ptr<PackageIDSequence>,
        Hash<QualifiedPackageName> > IDs;

namespace paludis
{
    template <>
    struct Implementation<UnwrittenRepositoryStore>
    {
        const UnwrittenRepository * const repo;
        mutable std::tr1::shared_ptr<CategoryNamePartSet> categories;
        mutable PackageNames package_names;
        mutable IDs ids;

        Implementation(const UnwrittenRepository * const r) :
            repo(r),
            categories(new CategoryNamePartSet)
        {
        }
    };
}

UnwrittenRepositoryStore::UnwrittenRepositoryStore(
        const Environment * const env,
        const UnwrittenRepository * const repo,
        const FSEntry & f) :
    PrivateImplementationPattern<UnwrittenRepositoryStore>(new Implementation<UnwrittenRepositoryStore>(repo))
{
    _populate(env, f);
}

UnwrittenRepositoryStore::~UnwrittenRepositoryStore()
{
}

void
UnwrittenRepositoryStore::_populate(const Environment * const env, const FSEntry & f)
{
    Context context("When populating UnwrittenRepository from directory '" + stringify(f) + "':");

    using namespace std::tr1::placeholders;
    std::for_each(DirIterator(f), DirIterator(), std::tr1::bind(
                &UnwrittenRepositoryStore::_populate_one, this, env, _1));
}

void
UnwrittenRepositoryStore::_populate_one(const Environment * const env, const FSEntry & f)
{
    if (! is_file_with_extension(f, ".conf", IsFileWithOptions()))
        return;

    Context context("When populating UnwrittenRepository from file '" + stringify(f) + "':");

    UnwrittenRepositoryFile file(f);

    std::tr1::shared_ptr<Mask> mask(new UnwrittenMask);

    QualifiedPackageName old_name("x/x");
    std::tr1::shared_ptr<QualifiedPackageNameSet> pkgs;
    std::tr1::shared_ptr<PackageIDSequence> ids;
    for (UnwrittenRepositoryFile::ConstIterator i(file.begin()), i_end(file.end()) ;
            i != i_end ; ++i)
    {
        if (old_name.category() != (*i).name().category())
        {
            _imp->categories->insert((*i).name().category());
            PackageNames::iterator p(_imp->package_names.find((*i).name().category()));
            if (_imp->package_names.end() == p)
                p = _imp->package_names.insert(std::make_pair((*i).name().category(),
                            make_shared_ptr(new QualifiedPackageNameSet))).first;
            pkgs = p->second;
        }

        if (old_name != (*i).name())
        {
            pkgs->insert((*i).name());
            IDs::iterator p(_imp->ids.find((*i).name()));
            if (_imp->ids.end() == p)
                p = _imp->ids.insert(std::make_pair((*i).name(),
                            make_shared_ptr(new PackageIDSequence))).first;

            ids = p->second;
        }

        ids->push_back(make_shared_ptr(new UnwrittenID(make_named_values<UnwrittenIDParams>(
                            value_for<n::added_by>((*i).added_by()),
                            value_for<n::bug_ids>((*i).bug_ids()),
                            value_for<n::comment>((*i).comment()),
                            value_for<n::description>((*i).description()),
                            value_for<n::environment>(env),
                            value_for<n::homepage>((*i).homepage()),
                            value_for<n::mask>(mask),
                            value_for<n::name>((*i).name()),
                            value_for<n::remote_ids>((*i).remote_ids()),
                            value_for<n::repository>(_imp->repo),
                            value_for<n::slot>((*i).slot()),
                            value_for<n::version>((*i).version())
                        ))));

        old_name = (*i).name();
    }
}

bool
UnwrittenRepositoryStore::has_category_named(const CategoryNamePart & c) const
{
    return _imp->categories->end() != _imp->categories->find(c);
}

bool
UnwrittenRepositoryStore::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->ids.end() != _imp->ids.find(q);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
UnwrittenRepositoryStore::category_names() const
{
    return _imp->categories;
}

std::tr1::shared_ptr<const CategoryNamePartSet>
UnwrittenRepositoryStore::unimportant_category_names() const
{
    std::tr1::shared_ptr<CategoryNamePartSet> result(make_shared_ptr(new CategoryNamePartSet));
    result->insert(CategoryNamePart("virtual"));
    return result;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
UnwrittenRepositoryStore::package_names(const CategoryNamePart & c) const
{
    PackageNames::iterator p(_imp->package_names.find(c));
    if (_imp->package_names.end() == p)
        return make_shared_ptr(new QualifiedPackageNameSet);
    else
        return p->second;
}

std::tr1::shared_ptr<const PackageIDSequence>
UnwrittenRepositoryStore::package_ids(const QualifiedPackageName & p) const
{
    IDs::iterator i(_imp->ids.find(p));
    if (_imp->ids.end() == i)
        return make_shared_ptr(new PackageIDSequence);
    else
        return i->second;
}

template class PrivateImplementationPattern<unwritten_repository::UnwrittenRepositoryStore>;

