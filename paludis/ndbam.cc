/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/options.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/kc.hh>
#include <paludis/util/hashes.hh>
#include <paludis/ndbam.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <tr1/functional>
#include <tr1/unordered_map>
#include <functional>
#include <vector>
#include <map>
#include <fstream>

using namespace paludis;

template class Sequence<std::tr1::shared_ptr<NDBAMEntry> >;

namespace
{
    struct CategoryContents;
    struct PackageContents;
    struct CategoryNamesContainingPackageEntry;
}

typedef std::tr1::unordered_map<CategoryNamePart, std::tr1::shared_ptr<CategoryContents>, Hash<CategoryNamePart> > CategoryContentsMap;
typedef std::tr1::unordered_map<QualifiedPackageName, std::tr1::shared_ptr<PackageContents>, Hash<QualifiedPackageName> > PackageContentsMap;
typedef std::tr1::unordered_map<PackageNamePart, std::tr1::shared_ptr<CategoryNamesContainingPackageEntry>, Hash<PackageNamePart> > CategoryNamesContainingPackage;

namespace
{
    struct CategoryContents
    {
        Mutex mutex;
        std::tr1::shared_ptr<QualifiedPackageNameSet> package_names;
        PackageContentsMap package_contents_map;
    };

    struct PackageContents
    {
        Mutex mutex;
        std::tr1::shared_ptr<NDBAMEntrySequence> entries;
    };

    struct CategoryNamesContainingPackageEntry
    {
        Mutex mutex;
        std::tr1::shared_ptr<CategoryNamePartSet> category_names_containing_package;
    };
}


namespace paludis
{
    template <>
    struct Implementation<NDBAM>
    {
        const FSEntry location;

        mutable Mutex category_names_mutex;
        mutable std::tr1::shared_ptr<CategoryNamePartSet> category_names;
        mutable CategoryContentsMap category_contents_map;

        mutable Mutex category_names_containing_package_mutex;
        mutable CategoryNamesContainingPackage category_names_containing_package;

        Implementation(const FSEntry & l) :
            location(l)
        {
        }
    };
}

NDBAM::NDBAM(const FSEntry & l,
        const std::tr1::function<bool (const std::string &)> & check_format,
        const std::string & preferred_format) :
    PrivateImplementationPattern<NDBAM>(new Implementation<NDBAM>(l))
{
    Context c("When checking NDBAM layout at '" + stringify(l) + "':");
    if ((l / "ndbam.conf").exists())
    {
        Context cc("When reading '" + stringify(l / "ndbam.conf") + "':");
        KeyValueConfigFile k(l / "ndbam.conf", KeyValueConfigFileOptions());
        if (k.get("ndbam_format") != "1")
            throw ConfigurationError("Unsupported NDBAM format '" + k.get("ndbam_format") + "'");
        if (! check_format(k.get("repository_format")))
            throw ConfigurationError("Unsupported NDBAM repository format '" + k.get("ndbam_format") + "'");
    }
    else if (DirIterator(l) != DirIterator())
        throw ConfigurationError("No NDBAM repository found at '" + stringify(l) +
                "', and it is not an empty directory");
    else
    {
        Context cc("When creating skeleton NDBAM layout at '" + stringify(l) + "':");
        (l / "indices").mkdir();
        (l / "indices" / "categories").mkdir();
        (l / "indices" / "packages").mkdir();
        (l / "data").mkdir();
        std::ofstream n(stringify(l / "ndbam.conf").c_str());
        n << "ndbam_format = 1" << std::endl;
        n << "repository_format = " << preferred_format << std::endl;
        if (! n)
            throw FSError("Could not write to '" + stringify(l / "ndbam.conf") + "'");
    }
}

NDBAM::~NDBAM()
{
}

std::tr1::shared_ptr<const CategoryNamePartSet>
NDBAM::category_names()
{
    Lock l(_imp->category_names_mutex);
    if (! _imp->category_names)
    {
        Context context("When loading category names for NDBAM at '" + stringify(_imp->location) + "':");
        _imp->category_names.reset(new CategoryNamePartSet);
        for (DirIterator d(_imp->location / "indices" / "categories"), d_end ;
                d != d_end ; ++d)
        {
            if (! d->is_directory_or_symlink_to_directory())
                continue;
            if ('-' == d->basename().at(0))
                continue;

            try
            {
                CategoryNamePart c(d->basename());
                _imp->category_names->insert(c);
                /* Inserting into category_contents_map might return false if
                 * we're partially populated. That's ok. */
                _imp->category_contents_map.insert(std::make_pair(c, make_shared_ptr(new CategoryContents)));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message("ndbam.categories.skipping", ll_warning, lc_context) <<
                    "Skipping directory '" << *d << "' due to exception '" << e.message() << "' (" << e.what() << ")";
            }
        }
    }

    return _imp->category_names;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
NDBAM::package_names(const CategoryNamePart & c)
{
    if (! has_category_named(c))
        return make_shared_ptr(new QualifiedPackageNameSet);

    Lock l(_imp->category_names_mutex);
    CategoryContentsMap::iterator cc_i(_imp->category_contents_map.find(c));
    if (_imp->category_contents_map.end() == cc_i || ! cc_i->second)
        throw InternalError(PALUDIS_HERE, "has_category_named(" + stringify(c) + ") but got category_contents_map end or zero pointer");
    CategoryContents & cc(*cc_i->second);

    l.acquire_then_release_old(cc.mutex);

    if (! cc.package_names)
    {
        Context context("When loading package names in '" + stringify(c) + "' for NDBAM at '" + stringify(_imp->location) + "':");
        cc.package_names.reset(new QualifiedPackageNameSet);
        for (DirIterator d(_imp->location / "indices" / "categories" / stringify(c)), d_end ;
                d != d_end ; ++d)
        {
            if (! d->is_directory_or_symlink_to_directory())
                continue;
            if ('-' == d->basename().at(0))
                continue;

            try
            {
                QualifiedPackageName q(c + PackageNamePart(d->basename()));
                cc.package_names->insert(q);
                /* Inserting into package_contents_map might return false if
                 * we're partially populated. That's ok. */
                cc.package_contents_map.insert(std::make_pair(q, make_shared_ptr(new PackageContents)));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message("ndbam.packages.skipping", ll_warning, lc_context)
                    << "Skipping directory '" << *d << "' due to exception '" << e.message() << "' (" << e.what() << ")";
            }
        }
    }
    return cc.package_names;
}

bool
NDBAM::has_category_named(const CategoryNamePart & c)
{
    Lock l(_imp->category_names_mutex);
    CategoryContentsMap::const_iterator it(_imp->category_contents_map.find(c));
    if (it != _imp->category_contents_map.end())
        return it->second;

    if (! _imp->category_names)
    {
        if (FSEntry(_imp->location / "indices" / "categories" / stringify(c)).is_directory_or_symlink_to_directory())
        {
            _imp->category_contents_map.insert(std::make_pair(c, new CategoryContents));
            return true;
        }
        _imp->category_contents_map.insert(std::make_pair(c, std::tr1::shared_ptr<CategoryContents>()));
    }

    return false;
}

bool
NDBAM::has_package_named(const QualifiedPackageName & q)
{
    if (! has_category_named(q.category))
        return false;

    Lock l(_imp->category_names_mutex);
    CategoryContentsMap::iterator cc_i(_imp->category_contents_map.find(q.category));
    if (_imp->category_contents_map.end() == cc_i || ! cc_i->second)
        throw InternalError(PALUDIS_HERE, "has_category_named(" + stringify(q.category) + ") but got category_contents_map end or zero pointer");

    CategoryContents & cc(*cc_i->second);
    l.acquire_then_release_old(cc.mutex);

    PackageContentsMap::const_iterator it(cc.package_contents_map.find(q));
    if (it != cc.package_contents_map.end())
        return it->second;

    if (! cc.package_names)
    {
        if (FSEntry(_imp->location / "indices" / "categories" / stringify(q.category) / stringify(q.package)).is_directory_or_symlink_to_directory())
        {
            cc.package_contents_map.insert(std::make_pair(q, new PackageContents));
            return true;
        }
        cc.package_contents_map.insert(std::make_pair(q, std::tr1::shared_ptr<PackageContents>()));
    }

    return false;
}

namespace
{
    struct NDBAMEntryVersionComparator
    {
        bool operator() (const std::tr1::shared_ptr<const NDBAMEntry> & a, const std::tr1::shared_ptr<const NDBAMEntry> & b) const
        {
            return (*a)[k::version()] < (*b)[k::version()];
        }
    };
}

std::tr1::shared_ptr<NDBAMEntrySequence>
NDBAM::entries(const QualifiedPackageName & q)
{
    if (! has_package_named(q))
        return make_shared_ptr(new NDBAMEntrySequence);

    Lock l(_imp->category_names_mutex);
    CategoryContentsMap::iterator cc_i(_imp->category_contents_map.find(q.category));
    if (_imp->category_contents_map.end() == cc_i || ! cc_i->second)
        throw InternalError(PALUDIS_HERE, "has_package_named(" + stringify(q) + ") but got category_contents_map end or zero pointer");
    CategoryContents & cc(*cc_i->second);
    l.acquire_then_release_old(cc.mutex);

    PackageContentsMap::iterator pc_i(cc.package_contents_map.find(q));
    if (cc.package_contents_map.end() == pc_i || ! pc_i->second)
        throw InternalError(PALUDIS_HERE, "has_package_named(" + stringify(q) + ") but got package_contents_map end or zero pointer");
    PackageContents & pc(*pc_i->second);
    l.acquire_then_release_old(pc.mutex);

    if (! pc.entries)
    {
        pc.entries.reset(new NDBAMEntrySequence);
        Context context("When loading versions in '" + stringify(q) + "' for NDBAM at '" + stringify(_imp->location) + "':");
        pc.entries.reset(new NDBAMEntrySequence);
        for (DirIterator d(_imp->location / "indices" / "categories" / stringify(q.category) / stringify(q.package)), d_end ;
                d != d_end ; ++d)
        {
            if (! d->is_directory_or_symlink_to_directory())
                continue;
            if ('-' == d->basename().at(0))
                continue;

            try
            {
                std::vector<std::string> tokens;
                tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(d->basename(), ":", "", std::back_inserter(tokens));
                if (tokens.size() < 3)
                {
                    Log::get_instance()->message("ndbam.ids.ignoring", ll_warning, lc_context) << "Not using '" << *d <<
                        "', since it contains less than three ':'s";
                    continue;
                }

                VersionSpec v(tokens[0]);
                SlotName s(tokens[1]);
                std::string m(tokens[2]);
                pc.entries->push_back(make_shared_ptr(new NDBAMEntry(NDBAMEntry::named_create()
                                (k::name(), q)
                                (k::version(), v)
                                (k::slot(), s)
                                (k::fs_location(), d->realpath())
                                (k::package_id(), std::tr1::shared_ptr<PackageID>())
                                (k::magic(), m)
                                (k::mutex(), make_shared_ptr(new Mutex)))));
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                Log::get_instance()->message("ndbam.ids.skipping", ll_warning, lc_context) << "Skipping directory '" << *d << "' due to exception '"
                    << e.message() << "' (" << e.what() << ")";
            }
        }

        using namespace std::tr1::placeholders;
        pc.entries->sort(NDBAMEntryVersionComparator());
    }

    return pc.entries;
}

void
NDBAM::parse_contents(const PackageID & id,
        const std::tr1::function<void (const FSEntry &, const std::string & md5, const time_t mtime)> & on_file,
        const std::tr1::function<void (const FSEntry &)> & on_dir,
        const std::tr1::function<void (const FSEntry &, const std::string & target, const time_t mtime)> & on_sym
        ) const
{
    Context c("When fetching contents for '" + stringify(id) + "':");

    if (! id.fs_location_key())
        throw InternalError(PALUDIS_HERE, "No id.fs_location_key");

    FSEntry ff(id.fs_location_key()->value() / "contents");
    if (! ff.is_regular_file_or_symlink_to_regular_file())
    {
        Log::get_instance()->message("ndbam.contents.skipping", ll_warning, lc_context)
            << "Contents file '" << ff << "' not a regular file, skipping";
        return;
    }

    LineConfigFile f(ff, LineConfigFileOptions());
    for (LineConfigFile::ConstIterator line(f.begin()), line_end(f.end()) ;
            line != line_end ; ++line)
    {
        std::map<std::string, std::string> tokens;
        std::string::size_type p(0);
        bool error(false);
        while ((! error) && (p < line->length()) && (std::string::npos != p))
        {
            std::string::size_type q(line->find('=', p));
            if (std::string::npos == q)
            {
                Log::get_instance()->message("ndbam.contents.invalid", ll_warning, lc_context)
                    << "Malformed line '" << *line << "' in '" << ff << "'";
                error = true;
                continue;
            }

            std::string key(line->substr(p, q - p)), value;
            p = q + 1;
            while (p < line->length() && std::string::npos != p)
            {
                if ('\\' == (*line)[p])
                {
                    ++p;
                    if (p >= line->length() || std::string::npos == p)
                    {
                        Log::get_instance()->message("ndbam.contents.invalid", ll_warning, lc_context)
                            << "Malformed line '" << *line << "' in '" << ff << "'";
                        error = true;
                        break;
                    }
                    if ('n' == (*line)[p])
                        value.append("\n");
                    else
                        value.append(1, (*line)[p]);
                    ++p;
                }
                else if (' ' == (*line)[p])
                {
                    if (! tokens.insert(std::make_pair(key, value)).second)
                        Log::get_instance()->message("ndbam.contents.duplicate", ll_warning, lc_context)
                            << "Duplicate token '" << key << "' on line '" << *line << "' in '" << ff << "'";
                    key.clear();
                    value.clear();
                    ++p;
                    break;
                }
                else
                {
                    value.append(1, (*line)[p]);
                    ++p;
                }
            }

            if ((! error) && (! key.empty()))
            {
                if (! tokens.insert(std::make_pair(key, value)).second)
                    Log::get_instance()->message("ndbam.contents.duplicate", ll_warning, lc_context)
                        << "Duplicate token '" << key << "' on line '" << *line << "' in '" << ff << "'";
            }
        }

        if (error)
            continue;

        if (! tokens.count("type"))
        {
            Log::get_instance()->message("ndbam.contents.no_key.type", ll_warning, lc_context) <<
                "No key 'type' found on line '" << *line << "' in '" << ff << "'";
            continue;
        }
        std::string type(tokens.find("type")->second);

        if (! tokens.count("path"))
        {
            Log::get_instance()->message("ndbam.contents.no_key.path", ll_warning, lc_context) <<
                "No key 'path' found on line '" << *line << "' in '" << ff << "'";
            continue;
        }
        std::string path(tokens.find("path")->second);

        if ("file" == type)
        {
            if (! tokens.count("md5"))
            {
                Log::get_instance()->message("ndbam.contents.no_key.md5", ll_warning, lc_context) <<
                    "No key 'md5' found on sym line '" << *line << "' in '" << ff << "'";
                continue;
            }
            std::string md5(tokens.find("md5")->second);

            if (! tokens.count("mtime"))
            {
                Log::get_instance()->message("ndbam.contents.no_key.mtime", ll_warning, lc_context) <<
                    "No key 'mtime' found on sym line '" << *line << "' in '" << ff << "'";
                continue;
            }
            time_t mtime(destringify<time_t>(tokens.find("mtime")->second));

            on_file(path, md5, mtime);
        }
        else if ("dir" == type)
        {
            on_dir(path);
        }
        else if ("sym" == type)
        {
            if (! tokens.count("target"))
            {
                Log::get_instance()->message("ndbam.contents.no_key.target", ll_warning, lc_context) <<
                    "No key 'target' found on sym line '" << *line << "' in '" << ff << "'";
                continue;
            }
            std::string target(tokens.find("target")->second);

            if (! tokens.count("mtime"))
            {
                Log::get_instance()->message("ndbam.contents.no_key.mtime", ll_warning, lc_context) <<
                    "No key 'mtime' found on sym line '" << *line << "' in '" << ff << "'";
                continue;
            }
            time_t mtime(destringify<time_t>(tokens.find("mtime")->second));

            on_sym(path, target, mtime);
        }
        else
            Log::get_instance()->message("ndbam.contents.unknown_type", ll_warning, lc_context) <<
                "Unknown type '" << type << "' found on line '" << *line << "' in '" << ff << "'";
    }
}

std::tr1::shared_ptr<const CategoryNamePartSet>
NDBAM::category_names_containing_package(const PackageNamePart & p) const
{
    Lock l(_imp->category_names_containing_package_mutex);
    CategoryNamesContainingPackage::iterator cncp_i(_imp->category_names_containing_package.find(p));
    if (_imp->category_names_containing_package.end() == cncp_i)
        cncp_i = _imp->category_names_containing_package.insert(std::make_pair(p, new CategoryNamesContainingPackageEntry)).first;
    CategoryNamesContainingPackageEntry & cncp(*cncp_i->second);

    l.acquire_then_release_old(cncp.mutex);
    if (! cncp.category_names_containing_package)
    {
        Context c("When finding category names containing package '" + stringify(p) +
                "' in NDBAM at '" + stringify(_imp->location) + "':");

        cncp.category_names_containing_package.reset(new CategoryNamePartSet);
        FSEntry dd(_imp->location / "indices" / "packages" / stringify(p));
        if (dd.is_directory_or_symlink_to_directory())
        {
            for (DirIterator d(dd), d_end ;
                    d != d_end ; ++d)
            {
                if (! d->is_directory_or_symlink_to_directory())
                    continue;
                if ('-' == d->basename().at(0))
                    continue;

                try
                {
                    cncp.category_names_containing_package->insert(CategoryNamePart(d->basename()));
                }
                catch (const InternalError &)
                {
                    throw;
                }
                catch (const Exception & e)
                {
                    Log::get_instance()->message("ndbam.categories.skipping", ll_warning, lc_context)
                        << "Skipping directory '" << *d << "' due to exception '"
                        << e.message() << "' (" << e.what() << ")";
                }
            }
        }
    }

    return cncp.category_names_containing_package;
}

void
NDBAM::deindex(const QualifiedPackageName & q) const
{
    Context context("When deindexing '" + stringify(q) + "' in NDBAM at '" + stringify(_imp->location) + "':");

    FSEntry cp_index_sym(_imp->location / "indices" / "categories" / stringify(q.category) / stringify(q.package));
    cp_index_sym.unlink();

    FSEntry pc_index_sym(_imp->location / "indices" / "packages" / stringify(q.package) / stringify(q.category));
    pc_index_sym.unlink();
}

void
NDBAM::index(const QualifiedPackageName & q, const std::string & d) const
{
    Context context("When indexing '" + stringify(q) + "' to '" + stringify(d) +
            "' in NDBAM at '" + stringify(_imp->location) + "':");

    FSEntry cp_index_sym(_imp->location / "indices" / "categories" / stringify(q.category));
    cp_index_sym.mkdir();
    cp_index_sym /= stringify(q.package);
    if (! cp_index_sym.exists())
        cp_index_sym.symlink("../../../data/" + d);

    FSEntry pc_index_sym(_imp->location / "indices" / "packages" / stringify(q.package));
    pc_index_sym.mkdir();
    pc_index_sym /= stringify(q.category);
    if (! pc_index_sym.exists())
        pc_index_sym.symlink("../../../data/" + d);
}

