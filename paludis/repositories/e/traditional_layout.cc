/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
 * Copyright (c) 2006 Danny van Dyk
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

#include <paludis/repositories/e/traditional_layout.hh>
#include <paludis/repositories/e/e_repository_entries.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/util/config_file.hh>
#include <paludis/package_id.hh>
#include <paludis/package_database.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/map.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/set.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/hashes.hh>
#include <tr1/functional>
#include <tr1/unordered_map>
#include <functional>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::tr1::unordered_map<CategoryNamePart, bool, Hash<CategoryNamePart> > CategoryMap;
typedef std::tr1::unordered_map<QualifiedPackageName, bool, Hash<QualifiedPackageName> > PackagesMap;
typedef std::tr1::unordered_map<QualifiedPackageName, std::tr1::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName>  > IDMap;

namespace paludis
{
    template<>
    struct Implementation<TraditionalLayout>
    {
        const ERepository * const repository;
        const FSEntry tree_root;

        mutable Mutex big_nasty_mutex;

        mutable bool has_category_names;
        mutable CategoryMap category_names;
        mutable PackagesMap package_names;
        mutable IDMap ids;

        mutable std::tr1::shared_ptr<CategoryNamePartSet> category_names_collection;
        std::tr1::shared_ptr<const ERepositoryEntries> entries;

        std::tr1::shared_ptr<FSEntrySequence> arch_list_files;
        std::tr1::shared_ptr<FSEntrySequence> repository_mask_files;
        std::tr1::shared_ptr<FSEntrySequence> profiles_desc_files;
        std::tr1::shared_ptr<FSEntrySequence> mirror_files;
        std::tr1::shared_ptr<FSEntrySequence> info_packages_files;
        std::tr1::shared_ptr<FSEntrySequence> info_variables_files;
        std::tr1::shared_ptr<UseDescFileInfoSequence> use_desc_files;

        Implementation(const ERepository * const r, const FSEntry & t,
                std::tr1::shared_ptr<const ERepositoryEntries> e) :
            repository(r),
            tree_root(t),
            has_category_names(false),
            entries(e),
            arch_list_files(new FSEntrySequence),
            repository_mask_files(new FSEntrySequence),
            profiles_desc_files(new FSEntrySequence),
            mirror_files(new FSEntrySequence),
            info_packages_files(new FSEntrySequence),
            info_variables_files(new FSEntrySequence),
            use_desc_files(new UseDescFileInfoSequence)
        {
        }
    };
}

TraditionalLayout::TraditionalLayout(const ERepository * const repo, const FSEntry & tree_root,
        std::tr1::shared_ptr<const ERepositoryEntries> e,
        std::tr1::shared_ptr<const FSEntry> f) :
    Layout(f),
    PrivateImplementationPattern<TraditionalLayout>(new Implementation<TraditionalLayout>(repo, tree_root, e))
{
    if (master_repository_location())
    {
        _imp->arch_list_files->push_back(*master_repository_location() / "profiles" / "arch.list");
        _imp->repository_mask_files->push_back(*master_repository_location() / "profiles" / "package.mask");
        _imp->profiles_desc_files->push_back(*master_repository_location() / "profiles" / "profiles.desc");
        _imp->mirror_files->push_back(*master_repository_location() / "profiles" / "thirdpartymirrors");
        _imp->info_variables_files->push_back(*master_repository_location() / "profiles" / "info_vars");

        _imp->use_desc_files->push_back(std::make_pair(*master_repository_location() / "profiles" / "use.desc", ""));
        _imp->use_desc_files->push_back(std::make_pair(*master_repository_location() / "profiles" / "use.local.desc", ""));
        FSEntry descs(*master_repository_location() / "profiles" / "desc");
        if (descs.is_directory_or_symlink_to_directory())
        {
            for (DirIterator d(descs), d_end ; d != d_end ; ++d)
            {
                if (! is_file_with_extension(*d, ".desc", IsFileWithOptions()))
                    continue;
                _imp->use_desc_files->push_back(std::make_pair(*d, strip_trailing_string(d->basename(), ".desc")));
            }
        }
    }

    _imp->arch_list_files->push_back(_imp->tree_root / "profiles" / "arch.list");
    _imp->repository_mask_files->push_back(_imp->tree_root / "profiles" / "package.mask");
    _imp->profiles_desc_files->push_back(_imp->tree_root / "profiles" / "profiles.desc");
    _imp->mirror_files->push_back(_imp->tree_root / "profiles" / "thirdpartymirrors");
    _imp->info_variables_files->push_back(_imp->tree_root / "profiles" / "info_vars");
    _imp->info_packages_files->push_back(_imp->tree_root / "profiles" / "info_pkgs");

    _imp->use_desc_files->push_back(std::make_pair(_imp->tree_root / "profiles" / "use.desc", ""));
    _imp->use_desc_files->push_back(std::make_pair(_imp->tree_root / "profiles" / "use.local.desc", ""));
    FSEntry descs(_imp->tree_root / "profiles" / "desc");
    if (descs.is_directory_or_symlink_to_directory())
    {
        for (DirIterator d(descs), d_end ; d != d_end ; ++d)
        {
            if (! is_file_with_extension(*d, ".desc", IsFileWithOptions()))
                continue;
            _imp->use_desc_files->push_back(std::make_pair(*d, strip_trailing_string(d->basename(), ".desc")));
        }
    }
}

TraditionalLayout::~TraditionalLayout()
{
}

FSEntry
TraditionalLayout::categories_file() const
{
    return _imp->tree_root / "profiles" / "categories";
}

void
TraditionalLayout::need_category_names() const
{
    Lock l(_imp->big_nasty_mutex);

    if (_imp->has_category_names)
        return;

    Context context("When loading category names for " + stringify(_imp->repository->name()) + ":");

    Log::get_instance()->message("e.traditional_layout.need_category_names", ll_debug, lc_context) << "need_category_names";

    bool found_one(false);

    std::list<FSEntry> cats_list;
    if (_imp->repository->params().master_repository)
        cats_list.push_back(_imp->repository->params().master_repository->layout()->categories_file());
    cats_list.push_back(categories_file());

    for (std::list<FSEntry>::const_iterator i(cats_list.begin()), i_end(cats_list.end()) ;
            i != i_end ; ++i)
    {
        if (! i->exists())
            continue;

        LineConfigFile cats(*i, LineConfigFileOptions() + lcfo_disallow_continuations);

        for (LineConfigFile::ConstIterator line(cats.begin()), line_end(cats.end()) ;
                line != line_end ; ++line)
        {
            try
            {
                _imp->category_names.insert(std::make_pair(CategoryNamePart(*line), false));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message("e.traditional_layout.categories.failure", ll_warning, lc_context)
                    << "Skipping line '" << *line << "' in '" << *i << "' due to exception '"
                    << e.message() << "' ('" << e.what() << ")";
            }
        }

        found_one = true;
    }

    if (! found_one)
    {
        Log::get_instance()->message("e.traditional_layout.categories.no_file", ll_qa, lc_context)
            << "No categories file for repository at '" << _imp->tree_root << "', faking it";
        for (DirIterator d(_imp->tree_root, DirIteratorOptions() + dio_inode_sort), d_end ; d != d_end ; ++d)
        {
            if (! d->is_directory_or_symlink_to_directory())
                continue;

            std::string n(d->basename());
            if (n == "CVS" || n == "distfiles" || n == "scripts" || n == "eclass" || n == "licenses"
                    || n == "packages")
                continue;

            try
            {
                _imp->category_names.insert(std::make_pair(CategoryNamePart(n), false));
            }
            catch (const NameError &)
            {
            }
        }
    }

    _imp->has_category_names = true;
}

void
TraditionalLayout::need_package_ids(const QualifiedPackageName & n) const
{
    Lock l(_imp->big_nasty_mutex);

    using namespace std::tr1::placeholders;

    if (_imp->package_names[n])
        return;

    Context context("When loading versions for '" + stringify(n) + "' in "
            + stringify(_imp->repository->name()) + ":");

    std::tr1::shared_ptr<PackageIDSequence> v(new PackageIDSequence);

    FSEntry path(_imp->tree_root / stringify(n.category) / stringify(n.package));

    for (DirIterator e(path, DirIteratorOptions() + dio_inode_sort), e_end ; e != e_end ; ++e)
    {
        if (! _imp->entries->is_package_file(n, *e))
            continue;

        try
        {
            std::tr1::shared_ptr<const PackageID> id(_imp->entries->make_id(n, *e));
            if (indirect_iterator(v->end()) != std::find_if(indirect_iterator(v->begin()), indirect_iterator(v->end()),
                        std::tr1::bind(std::equal_to<VersionSpec>(), id->version(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1))))
                Log::get_instance()->message("e.traditional_layout.id.duplicate", ll_warning, lc_context)
                    << "Ignoring entry '" << *e << "' for '" << n << "' in repository '" << _imp->repository->name()
                    << "' because another equivalent version already exists";
            else
                v->push_back(id);
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & ee)
        {
            Log::get_instance()->message("e.traditional_layout.id.failure", ll_warning, lc_context)
                << "Skipping entry '" << *e << "' for '" << n << "' in repository '"
                << _imp->repository->name() << "' due to exception '" << ee.message() << "' ("
                << ee.what() << ")'";
        }
    }

    _imp->ids.insert(std::make_pair(n, v));
    _imp->package_names[n] = true;
}

bool
TraditionalLayout::has_category_named(const CategoryNamePart & c) const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When checking for category '" + stringify(c) + "' in '" + stringify(_imp->repository->name()) + "':");

    need_category_names();
    return _imp->category_names.end() != _imp->category_names.find(c);
}

bool
TraditionalLayout::has_package_named(const QualifiedPackageName & q) const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When checking for package '" + stringify(q) + "' in '" + stringify(_imp->repository->name()) + ":");

    need_category_names();

    CategoryMap::iterator cat_iter(_imp->category_names.find(q.category));

    if (_imp->category_names.end() == cat_iter)
        return false;

    if (cat_iter->second)
    {
        /* this category's package names are fully loaded */
        return _imp->package_names.find(q) != _imp->package_names.end();
    }
    else
    {
        /* package names are only partially loaded or not loaded */
        if (_imp->package_names.find(q) != _imp->package_names.end())
            return true;

        FSEntry fs(_imp->tree_root);
        fs /= stringify(q.category);
        fs /= stringify(q.package);
        if (! fs.is_directory_or_symlink_to_directory())
            return false;
        _imp->package_names.insert(std::make_pair(q, false));
        return true;
    }
}

void
TraditionalLayout::need_category_names_collection() const
{
    Lock l(_imp->big_nasty_mutex);

    if (_imp->category_names_collection)
        return;

    need_category_names();

    _imp->category_names_collection.reset(new CategoryNamePartSet);
    std::transform(_imp->category_names.begin(), _imp->category_names.end(),
            _imp->category_names_collection->inserter(),
            std::tr1::mem_fn(&std::pair<const CategoryNamePart, bool>::first));
}

std::tr1::shared_ptr<const CategoryNamePartSet>
TraditionalLayout::category_names() const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When fetching category names in " + stringify(stringify(_imp->repository->name())) + ":");

    need_category_names_collection();
    return _imp->category_names_collection;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
TraditionalLayout::package_names(const CategoryNamePart & c) const
{
    Lock l(_imp->big_nasty_mutex);

    using namespace std::tr1::placeholders;

    /* this isn't particularly fast because it isn't called very often. avoid
     * changing the data structures used to make this faster at the expense of
     * slowing down single item queries. */

    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(_imp->repository->name()) + ":");

    need_category_names();

    if (_imp->category_names.end() == _imp->category_names.find(c))
        return std::tr1::shared_ptr<QualifiedPackageNameSet>(new QualifiedPackageNameSet);

    if ((_imp->tree_root / stringify(c)).is_directory_or_symlink_to_directory())
        for (DirIterator d(_imp->tree_root / stringify(c), DirIteratorOptions() + dio_inode_sort), d_end ; d != d_end ; ++d)
        {
            try
            {
                if (! d->is_directory_or_symlink_to_directory())
                    continue;

                if (DirIterator() == std::find_if(DirIterator(*d, DirIteratorOptions() + dio_inode_sort), DirIterator(),
                            std::tr1::bind(&ERepositoryEntries::is_package_file, _imp->entries.get(),
                                c + PackageNamePart(d->basename()), _1)))
                    continue;

                _imp->package_names.insert(std::make_pair(c + PackageNamePart(d->basename()), false));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message("e.traditional_layout.packages.failure", ll_warning, lc_context) << "Skipping entry '" <<
                    d->basename() << "' in category '" << c << "' in repository '" <<
                    stringify(_imp->repository->name()) << "' (" << e.message() << ")";
            }
        }

    _imp->category_names[c] = true;

    std::tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);

    for (PackagesMap::const_iterator p(_imp->package_names.begin()), p_end(_imp->package_names.end()) ;
            p != p_end ; ++p)
        if (p->first.category == c)
            result->insert(p->first);

    return result;
}

std::tr1::shared_ptr<const PackageIDSequence>
TraditionalLayout::package_ids(const QualifiedPackageName & n) const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When fetching versions of '" + stringify(n) + "' in " + stringify(_imp->repository->name()) + ":");

    if (has_package_named(n))
    {
        need_package_ids(n);
        return _imp->ids.find(n)->second;
    }
    else
        return std::tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);
}

const std::tr1::shared_ptr<const FSEntrySequence>
TraditionalLayout::info_packages_files() const
{
    return _imp->info_packages_files;
}

const std::tr1::shared_ptr<const FSEntrySequence>
TraditionalLayout::info_variables_files() const
{
    return _imp->info_variables_files;
}

FSEntry
TraditionalLayout::package_directory(const QualifiedPackageName & qpn) const
{
    return _imp->tree_root / stringify(qpn.category) / stringify(qpn.package);
}

FSEntry
TraditionalLayout::category_directory(const CategoryNamePart & cat) const
{
    return _imp->tree_root / stringify(cat);
}

std::tr1::shared_ptr<const FSEntrySequence>
TraditionalLayout::arch_list_files() const
{
    return _imp->arch_list_files;
}

std::tr1::shared_ptr<const FSEntrySequence>
TraditionalLayout::repository_mask_files() const
{
    return _imp->repository_mask_files;
}

std::tr1::shared_ptr<const FSEntrySequence>
TraditionalLayout::profiles_desc_files() const
{
    return _imp->profiles_desc_files;
}

std::tr1::shared_ptr<const FSEntrySequence>
TraditionalLayout::mirror_files() const
{
    return _imp->mirror_files;
}

std::tr1::shared_ptr<const UseDescFileInfoSequence>
TraditionalLayout::use_desc_files() const
{
    return _imp->use_desc_files;
}

FSEntry
TraditionalLayout::profiles_base_dir() const
{
    if (master_repository_location())
        return *master_repository_location() / "profiles";
    else
        return _imp->tree_root / "profiles";
}

std::tr1::shared_ptr<const FSEntrySequence>
TraditionalLayout::exlibsdirs(const QualifiedPackageName & q) const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    std::tr1::shared_ptr<const FSEntrySequence> global(exlibsdirs_global());
    std::copy(global->begin(), global->end(), result->back_inserter());

    std::tr1::shared_ptr<const FSEntrySequence> category(exlibsdirs_category(q.category));
    std::copy(category->begin(), category->end(), result->back_inserter());

    std::tr1::shared_ptr<const FSEntrySequence> package(exlibsdirs_package(q));
    std::copy(package->begin(), package->end(), result->back_inserter());

    return result;
}

std::tr1::shared_ptr<const FSEntrySequence>
TraditionalLayout::exlibsdirs_global() const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    if (_imp->repository->params().master_repository)
    {
        std::tr1::shared_ptr<const FSEntrySequence> master(_imp->repository->params().master_repository->layout()->exlibsdirs_global());
        std::copy(master->begin(), master->end(), result->back_inserter());
    }
    result->push_back(_imp->tree_root / "exlibs");

    return result;
}

std::tr1::shared_ptr<const FSEntrySequence>
TraditionalLayout::exlibsdirs_category(const CategoryNamePart & c) const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    if (_imp->repository->params().master_repository)
    {
        std::tr1::shared_ptr<const FSEntrySequence> master(_imp->repository->params().master_repository->layout()->exlibsdirs_category(c));
        std::copy(master->begin(), master->end(), result->back_inserter());
    }
    result->push_back(category_directory(c) / "exlibs");

    return result;
}

std::tr1::shared_ptr<const FSEntrySequence>
TraditionalLayout::exlibsdirs_package(const QualifiedPackageName & q) const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    if (_imp->repository->params().master_repository)
    {
        std::tr1::shared_ptr<const FSEntrySequence> master(_imp->repository->params().master_repository->layout()->exlibsdirs_package(q));
        std::copy(master->begin(), master->end(), result->back_inserter());
    }
    result->push_back(package_directory(q));

    return result;
}

std::tr1::shared_ptr<const FSEntrySequence>
TraditionalLayout::licenses_dirs() const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    if (_imp->repository->params().master_repository)
    {
        std::tr1::shared_ptr<const FSEntrySequence> master(_imp->repository->params().master_repository->layout()->licenses_dirs());
        std::copy(master->begin(), master->end(), result->back_inserter());
    }
    result->push_back(_imp->tree_root / "licenses");

    return result;
}

namespace
{
    void aux_files_helper(const FSEntry & d,
            std::tr1::shared_ptr<Map<FSEntry, std::string> > & m,
            const QualifiedPackageName & qpn)
    {
        if (! d.exists())
            return;

        std::list<FSEntry> files((DirIterator(d, DirIteratorOptions() + dio_inode_sort)),
                DirIterator());
        for (std::list<FSEntry>::iterator f(files.begin()) ;
                f != files.end() ; ++f)
        {
            if (f->is_directory())
            {
                if ("CVS" != f->basename())
                    aux_files_helper((*f), m, qpn);
            }
            else
            {
                if (! f->is_regular_file())
                    continue;
                if (is_file_with_prefix_extension((*f),
                            ("digest-"+stringify(qpn.package)), "",
                            IsFileWithOptions()))
                    continue;
                m->insert((*f), "AUX");
            }
        }
    }
}

std::tr1::shared_ptr<Map<FSEntry, std::string> >
TraditionalLayout::manifest_files(const QualifiedPackageName & qpn) const
{
    std::tr1::shared_ptr<Map<FSEntry, std::string> > result(new Map<FSEntry, std::string>);
    FSEntry package_dir = _imp->repository->layout()->package_directory(qpn);

    std::list<FSEntry> package_files((DirIterator(package_dir, DirIteratorOptions() + dio_inode_sort)),
            DirIterator());
    for (std::list<FSEntry>::iterator f(package_files.begin()) ;
            f != package_files.end() ; ++f)
    {
        if (! (*f).is_regular_file() || ((*f).basename() == "Manifest") )
            continue;

        std::string file_type("MISC");
        if (_imp->entries->is_package_file(qpn, (*f)))
            file_type=_imp->entries->get_package_file_manifest_key((*f), qpn);

        result->insert((*f), file_type);
    }

    aux_files_helper((package_dir / "files"), result, qpn);

    return result;
}

FSEntry
TraditionalLayout::sync_filter_file() const
{
    return FSEntry(DATADIR "/paludis/traditional.exclude");
}

void
TraditionalLayout::invalidate_masks()
{
    Lock l(_imp->big_nasty_mutex);

    for (IDMap::iterator it(_imp->ids.begin()), it_end(_imp->ids.end()); it_end != it; ++it)
        for (PackageIDSequence::ConstIterator it2(it->second->begin()), it2_end(it->second->end());
             it2_end != it2; ++it2)
            (*it2)->invalidate_masks();
}

FSEntry
TraditionalLayout::binary_ebuild_location(const QualifiedPackageName & q, const VersionSpec & v,
        const std::string & eapi) const
{
    return package_directory(q) / _imp->entries->binary_ebuild_name(q, v, eapi);
}

