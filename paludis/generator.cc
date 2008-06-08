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

#include <paludis/generator.hh>
#include <paludis/generator_handler.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/action_names.hh>
#include <paludis/action.hh>
#include <paludis/match_package.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <algorithm>
#include <tr1/functional>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<Generator>
    {
        std::tr1::shared_ptr<const GeneratorHandler> handler;

        Implementation(const std::tr1::shared_ptr<const GeneratorHandler> & h) :
            handler(h)
        {
        }
    };
}

Generator::Generator(const std::tr1::shared_ptr<const GeneratorHandler> & h) :
    PrivateImplementationPattern<Generator>(new Implementation<Generator>(h))
{
}

Generator::Generator(const Generator & other) :
    PrivateImplementationPattern<Generator>(new Implementation<Generator>(other._imp->handler))
{
}

Generator &
Generator::operator= (const Generator & other)
{
    if (this != &other)
        _imp->handler = other._imp->handler;
    return *this;
}

Generator::~Generator()
{
}

Generator::operator FilteredGenerator () const
{
    return FilteredGenerator(*this, filter::All());
}

std::tr1::shared_ptr<const RepositoryNameSet>
Generator::repositories(
        const Environment * const env) const
{
    return _imp->handler->repositories(env);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
Generator::categories(
        const Environment * const env,
        const std::tr1::shared_ptr<const RepositoryNameSet> & r) const
{
    return _imp->handler->categories(env, r);
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
Generator::packages(
        const Environment * const env,
        const std::tr1::shared_ptr<const RepositoryNameSet> & r,
        const std::tr1::shared_ptr<const CategoryNamePartSet> & c) const
{
    return _imp->handler->packages(env, r, c);
}

std::tr1::shared_ptr<const PackageIDSet>
Generator::ids(
        const Environment * const env,
        const std::tr1::shared_ptr<const RepositoryNameSet> & r,
        const std::tr1::shared_ptr<const QualifiedPackageNameSet> & q) const
{
    return _imp->handler->ids(env, r, q);
}

std::string
Generator::as_string() const
{
    return _imp->handler->as_string();
}

namespace
{
    struct RepositoryGeneratorHandler :
        AllGeneratorHandlerBase
    {
        const RepositoryName name;

        RepositoryGeneratorHandler(const RepositoryName & n) :
            name(n)
        {
        }

        virtual std::tr1::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env) const
        {
            using namespace std::tr1::placeholders;
            std::tr1::shared_ptr<RepositoryNameSet> result(new RepositoryNameSet);
            if (env->package_database()->has_repository_named(name))
                result->insert(name);
            return result;
        }

        virtual std::string as_string() const
        {
            return "packages with repository " + stringify(name);
        }
    };

    struct CategoryGeneratorHandler :
        AllGeneratorHandlerBase
    {
        const CategoryNamePart name;

        CategoryGeneratorHandler(const CategoryNamePart & n) :
            name(n)
        {
        }

        virtual std::tr1::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const env,
                const std::tr1::shared_ptr<const RepositoryNameSet> & repos
                ) const
        {
            std::tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);

            for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                    r != r_end ; ++r)
                if (env->package_database()->fetch_repository(*r)->has_category_named(name))
                {
                    result->insert(name);
                    break;
                }

            return result;
        }

        virtual std::string as_string() const
        {
            return "packages with category " + stringify(name);
        }
    };

    struct PackageGeneratorHandler :
        AllGeneratorHandlerBase
    {
        const QualifiedPackageName name;

        PackageGeneratorHandler(const QualifiedPackageName & n) :
            name(n)
        {
        }

        virtual std::tr1::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const env,
                const std::tr1::shared_ptr<const RepositoryNameSet> & repos
                ) const
        {
            std::tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);

            for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                    r != r_end ; ++r)
                if (env->package_database()->fetch_repository(*r)->has_category_named(name.category))
                {
                    result->insert(name.category);
                    break;
                }

            return result;
        }

        virtual std::tr1::shared_ptr<const QualifiedPackageNameSet> packages(
                const Environment * const env,
                const std::tr1::shared_ptr<const RepositoryNameSet> & repos,
                const std::tr1::shared_ptr<const CategoryNamePartSet> & cats) const
        {
            std::tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
            for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                    r != r_end ; ++r)
                for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
                        c != c_end ; ++c)
                    if (env->package_database()->fetch_repository(*r)->has_package_named(name))
                        result->insert(name);

            return result;
        }

        virtual std::string as_string() const
        {
            return "packages named " + stringify(name);
        }
    };

    struct MatchesGeneratorHandler :
        AllGeneratorHandlerBase
    {
        const PackageDepSpec spec;

        MatchesGeneratorHandler(const PackageDepSpec & s) :
            spec(s)
        {
        }

        virtual std::tr1::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env) const
        {
            if (! spec.repository_ptr())
                return AllGeneratorHandlerBase::repositories(env);

            std::tr1::shared_ptr<RepositoryNameSet> result(new RepositoryNameSet);
            if (env->package_database()->has_repository_named(*spec.repository_ptr()))
                result->insert(*spec.repository_ptr());

            return result;
        }

        virtual std::tr1::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const env,
                const std::tr1::shared_ptr<const RepositoryNameSet> & repos) const
        {
            if (spec.category_name_part_ptr())
            {
                std::tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
                for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                        r != r_end ; ++r)
                    if (env->package_database()->fetch_repository(*r)->has_category_named(*spec.category_name_part_ptr()))
                    {
                        result->insert(*spec.category_name_part_ptr());
                        break;
                    }

                return result;
            }
            else if (spec.package_name_part_ptr())
            {
                std::tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
                for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                        r != r_end ; ++r)
                {
                    std::tr1::shared_ptr<const CategoryNamePartSet> cats(
                        env->package_database()->fetch_repository(*r)
                        ->category_names_containing_package(*spec.package_name_part_ptr()));
                    std::copy(cats->begin(), cats->end(), result->inserter());
                }

                return result;
            }
            else if (spec.package_ptr())
            {
                std::tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
                for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                        r != r_end ; ++r)
                    if (env->package_database()->fetch_repository(*r)->has_category_named(spec.package_ptr()->category))
                    {
                        result->insert(spec.package_ptr()->category);
                        break;
                    }

                return result;
            }
            else
                return AllGeneratorHandlerBase::categories(env, repos);
        }

        virtual std::tr1::shared_ptr<const QualifiedPackageNameSet> packages(
                const Environment * const env,
                const std::tr1::shared_ptr<const RepositoryNameSet> & repos,
                const std::tr1::shared_ptr<const CategoryNamePartSet> & cats) const
        {
            if (spec.package_name_part_ptr())
            {
                std::tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
                for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                        r != r_end ; ++r)
                    for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
                            c != c_end ; ++c)
                        if (env->package_database()->fetch_repository(*r)->has_package_named(*c + *spec.package_name_part_ptr()))
                            result->insert(*c + *spec.package_name_part_ptr());

                return result;
            }
            else if (spec.package_ptr())
            {
                std::tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
                for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                        r != r_end ; ++r)
                    for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
                            c != c_end ; ++c)
                        if (env->package_database()->fetch_repository(*r)->has_package_named(*spec.package_ptr()))
                            result->insert(*spec.package_ptr());

                return result;
            }
            else
                return AllGeneratorHandlerBase::packages(env, repos, cats);
        }

        virtual std::tr1::shared_ptr<const PackageIDSet> ids(
                const Environment * const env,
                const std::tr1::shared_ptr<const RepositoryNameSet> & repos,
                const std::tr1::shared_ptr<const QualifiedPackageNameSet> & qpns) const
        {
            std::tr1::shared_ptr<PackageIDSet> result(new PackageIDSet);

            for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                    r != r_end ; ++r)
            {
                for (QualifiedPackageNameSet::ConstIterator q(qpns->begin()), q_end(qpns->end()) ;
                        q != q_end ; ++q)
                {
                    std::tr1::shared_ptr<const PackageIDSequence> id(
                            env->package_database()->fetch_repository(*r)->package_ids(*q));
                    for (PackageIDSequence::ConstIterator i(id->begin()), i_end(id->end()) ;
                            i != i_end ; ++i)
                        if (match_package(*env, spec, **i))
                            result->insert(*i);
                }
            }

            return result;
        }

        virtual std::string as_string() const
        {
            return "packages matching " + stringify(spec);
        }
    };

    struct IntersectionGeneratorHandler :
        GeneratorHandler
    {
        const Generator g1;
        const Generator g2;

        IntersectionGeneratorHandler(const Generator & h1, const Generator & h2) :
            g1(h1),
            g2(h2)
        {
        }

        virtual std::tr1::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env) const
        {
            std::tr1::shared_ptr<const RepositoryNameSet> r1(g1.repositories(env));
            if (r1->empty())
                return r1;

            std::tr1::shared_ptr<const RepositoryNameSet> r2(g2.repositories(env));
            if (r2->empty())
                return r2;

            std::tr1::shared_ptr<RepositoryNameSet> result(new RepositoryNameSet);
            std::set_intersection(
                    r1->begin(), r1->end(),
                    r2->begin(), r2->end(),
                    result->inserter(),
                    RepositoryNameComparator());
            return result;
        }

        virtual std::tr1::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const env,
                const std::tr1::shared_ptr<const RepositoryNameSet> & repos) const
        {
            std::tr1::shared_ptr<const CategoryNamePartSet> c1(g1.categories(env, repos));
            if (c1->empty())
                return c1;

            std::tr1::shared_ptr<const CategoryNamePartSet> c2(g2.categories(env, repos));
            if (c2->empty())
                return c2;

            std::tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
            std::set_intersection(
                    c1->begin(), c1->end(),
                    c2->begin(), c2->end(),
                    result->inserter());
            return result;
        }

        virtual std::tr1::shared_ptr<const QualifiedPackageNameSet> packages(
                const Environment * const env,
                const std::tr1::shared_ptr<const RepositoryNameSet> & repos,
                const std::tr1::shared_ptr<const CategoryNamePartSet> & cats) const
        {
            std::tr1::shared_ptr<const QualifiedPackageNameSet> q1(g1.packages(env, repos, cats));
            if (q1->empty())
                return q1;

            std::tr1::shared_ptr<const QualifiedPackageNameSet> q2(g2.packages(env, repos, cats));
            if (q2->empty())
                return q2;

            std::tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
            std::set_intersection(
                    q1->begin(), q1->end(),
                    q2->begin(), q2->end(),
                    result->inserter());
            return result;
        }

        virtual std::tr1::shared_ptr<const PackageIDSet> ids(
                const Environment * const env,
                const std::tr1::shared_ptr<const RepositoryNameSet> & repos,
                const std::tr1::shared_ptr<const QualifiedPackageNameSet> & qpns) const
        {
            std::tr1::shared_ptr<const PackageIDSet> i1(g1.ids(env, repos, qpns));
            if (i1->empty())
                return i1;

            std::tr1::shared_ptr<const PackageIDSet> i2(g2.ids(env, repos, qpns));
            if (i2->empty())
                return i2;

            std::tr1::shared_ptr<PackageIDSet> result(new PackageIDSet);
            std::set_intersection(
                    i1->begin(), i1->end(),
                    i2->begin(), i2->end(),
                    result->inserter(),
                    PackageIDSetComparator());
            return result;
        }

        virtual std::string as_string() const
        {
            return stringify(g1) + " intersected with " + stringify(g2);
        }
    };

    struct AllGeneratorHandler :
        AllGeneratorHandlerBase
    {
        virtual std::string as_string() const
        {
            return "all packages";
        }
    };

    template <typename A_>
    struct SomeIDsMightSupportActionGeneratorHandler :
        AllGeneratorHandlerBase
    {
        virtual std::tr1::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env) const
        {
            std::tr1::shared_ptr<RepositoryNameSet> result(new RepositoryNameSet);
            for (PackageDatabase::RepositoryConstIterator i(env->package_database()->begin_repositories()),
                    i_end(env->package_database()->end_repositories()) ; i != i_end ; ++i)
                if ((*i)->some_ids_might_support_action(SupportsActionTest<A_>()))
                    result->insert((*i)->name());

            return result;
        }

        virtual std::string as_string() const
        {
            return "packages that might support action " + stringify(ActionNames<A_>::value);
        }
    };
}

generator::All::All() :
    Generator(make_shared_ptr(new AllGeneratorHandler))
{
}

generator::Repository::Repository(const RepositoryName & n) :
    Generator(make_shared_ptr(new RepositoryGeneratorHandler(n)))
{
}

generator::Category::Category(const CategoryNamePart & n) :
    Generator(make_shared_ptr(new CategoryGeneratorHandler(n)))
{
}

generator::Package::Package(const QualifiedPackageName & n) :
    Generator(make_shared_ptr(new PackageGeneratorHandler(n)))
{
}

generator::Matches::Matches(const PackageDepSpec & spec) :
    Generator(make_shared_ptr(new MatchesGeneratorHandler(spec)))
{
}

generator::Intersection::Intersection(const Generator & g1, const Generator & g2) :
    Generator(make_shared_ptr(new IntersectionGeneratorHandler(g1, g2)))
{
}

template <typename A_>
generator::SomeIDsMightSupportAction<A_>::SomeIDsMightSupportAction() :
    Generator(make_shared_ptr(new SomeIDsMightSupportActionGeneratorHandler<A_>))
{
}

Generator
paludis::operator& (const Generator & g1, const Generator & g2)
{
    return generator::Intersection(g1, g2);
}

std::ostream &
paludis::operator<< (std::ostream & s, const Generator & g)
{
    s << g.as_string();
    return s;
}

template class PrivateImplementationPattern<Generator>;
template class generator::SomeIDsMightSupportAction<InstallAction>;
template class generator::SomeIDsMightSupportAction<InstalledAction>;
template class generator::SomeIDsMightSupportAction<UninstallAction>;
template class generator::SomeIDsMightSupportAction<PretendAction>;
template class generator::SomeIDsMightSupportAction<ConfigAction>;
template class generator::SomeIDsMightSupportAction<FetchAction>;
template class generator::SomeIDsMightSupportAction<InfoAction>;
template class generator::SomeIDsMightSupportAction<PretendFetchAction>;

