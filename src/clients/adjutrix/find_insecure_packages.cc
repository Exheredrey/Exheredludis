/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include "find_insecure_packages.hh"
#include "command_line.hh"

#include <paludis/util/tokeniser.hh>
#include <paludis/query.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_tag.hh>
#include <paludis/package_id.hh>
#include <paludis/package_database.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/iterator_funcs.hh>

#include <set>
#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    const unsigned col_width_package = 30;
    const unsigned col_width_id      = 40;

    void
    write_repository_header(const RepositoryName & repo)
    {
        std::string s("Insecure packages from advisories in repository '" + stringify(repo) + "'");
        cout << std::string(s.length(), '=') << endl;
        cout << s << endl;
        cout << std::string(s.length(), '=') << endl;
        cout << endl;

        cout << std::left
            << std::setw(col_width_package) << "package"
            << std::setw(col_width_id) << "GLSA IDs"
            << endl;

        cout
            << std::string(col_width_package - 1, '-') << " "
            << std::string(col_width_id - 1, '-') << " "
            << endl;
    }

    class ListInsecureVisitor :
        public ConstVisitor<SetSpecTree>,
        public ConstVisitor<SetSpecTree>::VisitConstSequence<ListInsecureVisitor, AllDepSpec>
    {
        private:
            const Environment & _env;
            std::multimap<std::tr1::shared_ptr<const PackageID>, std::string, PackageIDSetComparator> _found;
            std::set<SetName> recursing_sets;

        public:
            using ConstVisitor<SetSpecTree>::VisitConstSequence<ListInsecureVisitor, AllDepSpec>::visit;

            ListInsecureVisitor(const Environment & e) :
                _env(e)
            {
            }

            void visit_leaf(const PackageDepSpec & a)
            {
                std::tr1::shared_ptr<const PackageIDSequence> insecure(
                        _env.package_database()->query(query::Matches(a), qo_order_by_version));
                for (PackageIDSequence::ConstIterator i(insecure->begin()),
                        i_end(insecure->end()) ; i != i_end ; ++i)
                    if (a.tag())
                        _found.insert(std::make_pair(*i, a.tag()->short_text()));
                    else
                        throw InternalError(PALUDIS_HERE, "didn't get a tag");
            }

            void visit_leaf(const NamedSetDepSpec & s)
            {
                Context context("When expanding named set '" + stringify(s) + "':");

                std::tr1::shared_ptr<const SetSpecTree::ConstItem> set(_env.set(s.name()));

                if (! set)
                {
                    Log::get_instance()->message("adjutrix.find_insecure_packages.unknown_set", ll_warning, lc_context)
                        << "Unknown set '" << s.name() << "'";
                    return;
                }

                if (! recursing_sets.insert(s.name()).second)
                {
                    Log::get_instance()->message("adjutrix.find_insecure_packages.recursive_set", ll_warning, lc_context)
                        << "Recursively defined set '" << s.name() << "'";
                    return;
                }

                set->accept(*this);

                recursing_sets.erase(s.name());
            }

            friend std::ostream & operator<< (std::ostream &, const ListInsecureVisitor &);
    };

    std::ostream & operator<< (std::ostream & s, const ListInsecureVisitor & v)
    {
        QualifiedPackageName old_name("dormouse/teapot");
        for (std::multimap<std::tr1::shared_ptr<const PackageID>, std::string, PackageIDSetComparator>::const_iterator
                f(v._found.begin()), f_end(v._found.end()) ; f != f_end ; ++f)
        {
            if (f->first->name() != old_name)
                s << std::setw(col_width_package) << (stringify(f->first->name()) + " ") << endl;
            old_name = f->first->name();
            s << std::setw(col_width_package) << ("  " + stringify(f->first->canonical_form(idcf_version)) + " ")
                << f->second;
            while (next(f) != f_end)
            {
                if (*next(f)->first != *f->first)
                    break;
                cout << " " << f->second;
                ++f;
            }
            cout << endl;
        }

        return s;
    }
}

void do_find_insecure_packages(const NoConfigEnvironment & env)
{
    Context context("When performing find-insecure-packages action:");

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
            r(env.package_database()->begin_repositories()),
            r_end(env.package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (r->name() == RepositoryName("virtuals"))
            continue;
        if (env.master_repository() && r->name() == env.master_repository()->name())
            continue;

        if (! (*r)[k::sets_interface()])
            continue;

        write_repository_header(r->name());

        std::tr1::shared_ptr<const SetSpecTree::ConstItem> all_insecure((*r)[k::sets_interface()]->package_set(SetName("insecurity")));
        if (! all_insecure)
            continue;
        ListInsecureVisitor v(env);
        all_insecure->accept(v);
        cout << v << endl;
    }
}

