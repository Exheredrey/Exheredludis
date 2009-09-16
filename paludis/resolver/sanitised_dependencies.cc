/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/qpn_s.hh>
#include <paludis/resolver/serialise-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/spec_tree.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <set>
#include <list>

using namespace paludis;
using namespace paludis::resolver;

namespace
{
    template <typename T_>
    void list_push_back(std::list<T_> * const l, const T_ & t)
    {
        l->push_back(t);
    }

    struct MakeAnyOfStringVisitor
    {
        std::string result;

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & s)
        {
            result.append(" " + stringify(*s.spec()));
        }

        void visit(const DependencySpecTree::NodeType<DependencyLabelsDepSpec>::Type & s)
        {
            result.append(" " + stringify(*s.spec()));
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & s)
        {
            result.append(" " + stringify(*s.spec()));
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & s)
        {
            result.append(" " + stringify(*s.spec()));
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (node.spec()->condition_met())
            {
                MakeAnyOfStringVisitor v;
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(v));
                result.append(" " + stringify(*node.spec()) + " ( " + v.result + ")");
            }
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            MakeAnyOfStringVisitor v;
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(v));
            result.append(" || ( " + v.result + ")");
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            MakeAnyOfStringVisitor v;
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(v));
            result.append(" ( " + v.result + ")");
        }
    };

    struct AnyDepSpecChildHandler
    {
        const Resolver & resolver;
        const QPN_S our_qpn_s;
        const std::tr1::function<SanitisedDependency (const PackageOrBlockDepSpec &)> parent_make_sanitised;

        bool super_complicated, nested;

        std::list<std::list<PackageOrBlockDepSpec> > child_groups;
        std::list<PackageOrBlockDepSpec> * active_sublist;

        bool seen_any;

        AnyDepSpecChildHandler(const Resolver & r, const QPN_S & q,
                const std::tr1::function<SanitisedDependency (const PackageOrBlockDepSpec &)> & f) :
            resolver(r),
            our_qpn_s(q),
            parent_make_sanitised(f),
            super_complicated(false),
            nested(false),
            active_sublist(0),
            seen_any(false)
        {
        }

        void visit_package_or_block_spec(const PackageOrBlockDepSpec & spec)
        {
            seen_any = true;

            std::tr1::shared_ptr<DependencySpecTree> if_rewritten(resolver.rewrite_if_special(spec, our_qpn_s));
            if (if_rewritten)
                if_rewritten->root()->accept(*this);
            else
            {
                if (active_sublist)
                    active_sublist->push_back(spec);
                else
                {
                    std::list<PackageOrBlockDepSpec> l;
                    l.push_back(spec);
                    child_groups.push_back(l);
                }
            }
        }

        void visit_package_spec(const PackageDepSpec & spec)
        {
            if (spec.package_ptr())
                visit_package_or_block_spec(PackageOrBlockDepSpec(spec));
            else
                super_complicated = true;
        }

        void visit_block_spec(const BlockDepSpec & spec)
        {
            if (spec.blocking().package_ptr())
                visit_package_or_block_spec(PackageOrBlockDepSpec(spec));
            else
                super_complicated = true;
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            visit_package_spec(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node)
        {
            visit_block_spec(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (node.spec()->condition_met())
            {
                nested = true;

                if (active_sublist)
                    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                else
                {
                    Save<std::list<PackageOrBlockDepSpec> *> save_active_sublist(&active_sublist, 0);
                    active_sublist = &*child_groups.insert(child_groups.end(), std::list<PackageOrBlockDepSpec>());
                    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                }
            }
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            nested = true;

            if (active_sublist)
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            else
            {
                Save<std::list<PackageOrBlockDepSpec> *> save_active_sublist(&active_sublist, 0);
                active_sublist = &*child_groups.insert(child_groups.end(), std::list<PackageOrBlockDepSpec>());
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            }
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            AnyDepSpecChildHandler h(resolver, our_qpn_s, parent_make_sanitised);
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(h));
            std::list<SanitisedDependency> l;
            h.commit(
                    parent_make_sanitised,
                    std::tr1::bind(&list_push_back<SanitisedDependency>, &l, std::tr1::placeholders::_1)
                    );

            if (active_sublist)
            {
                for (std::list<SanitisedDependency>::const_iterator i(l.begin()), i_end(l.end()) ;
                        i != i_end ; ++i)
                    visit_package_or_block_spec(i->spec());
            }
            else
            {
                Save<std::list<PackageOrBlockDepSpec> *> save_active_sublist(&active_sublist, 0);
                active_sublist = &*child_groups.insert(child_groups.end(), std::list<PackageOrBlockDepSpec>());
                for (std::list<SanitisedDependency>::const_iterator i(l.begin()), i_end(l.end()) ;
                        i != i_end ; ++i)
                    visit_package_or_block_spec(i->spec());
            }
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type &)
        {
            super_complicated = true;
        }

        void visit(const DependencySpecTree::NodeType<DependencyLabelsDepSpec>::Type &)
        {
            super_complicated = true;
        }

        void commit(
                const std::tr1::function<SanitisedDependency (const PackageOrBlockDepSpec &)> & make_sanitised,
                const std::tr1::function<void (const SanitisedDependency &)> & apply)
        {
            if (! seen_any)
            {
            }
            else if (super_complicated)
                throw InternalError(PALUDIS_HERE, "can't");
            else
            {
                /* we've got a choice of groups of packages. pick the best score, left to right. */
                std::list<std::list<PackageOrBlockDepSpec> >::const_iterator g_best(child_groups.end());
                int best_score(-1);

                for (std::list<std::list<PackageOrBlockDepSpec> >::const_iterator g(child_groups.begin()),
                        g_end(child_groups.end()) ;
                        g != g_end ; ++g)
                {
                    int worst_score(-1);

                    if (g->empty())
                        throw InternalError(PALUDIS_HERE, "why did that happen?");

                    /* score of a group is the score of the worst child. */
                    for (std::list<PackageOrBlockDepSpec>::const_iterator h(g->begin()), h_end(g->end()) ;
                            h != h_end ; ++h)
                    {
                        int score(resolver.find_any_score(our_qpn_s, make_sanitised(PackageOrBlockDepSpec(*h))));
                        if ((-1 == worst_score) || (score < worst_score))
                            worst_score = score;
                    }

                    if ((best_score == -1) || (worst_score > best_score))
                    {
                        best_score = worst_score;
                        g_best = g;
                    }
                }

                if (g_best == child_groups.end())
                    throw InternalError(PALUDIS_HERE, "why did that happen?");
                for (std::list<PackageOrBlockDepSpec>::const_iterator h(g_best->begin()), h_end(g_best->end()) ;
                        h != h_end ; ++h)
                    apply(make_sanitised(*h));
            }
        }
    };

    struct Finder
    {
        const Resolver & resolver;
        const QPN_S our_qpn_s;
        SanitisedDependencies & sanitised_dependencies;
        const std::string raw_name;
        const std::string human_name;
        std::string original_specs_as_string;
        std::list<std::tr1::shared_ptr<ActiveDependencyLabels> > labels_stack;

        Finder(
                const Resolver & r,
                const QPN_S & q,
                SanitisedDependencies & s,
                const std::tr1::shared_ptr<const DependencyLabelSequence> & l,
                const std::string & rn,
                const std::string & hn,
                const std::string & a) :
            resolver(r),
            our_qpn_s(q),
            sanitised_dependencies(s),
            raw_name(rn),
            human_name(hn),
            original_specs_as_string(a)
        {
            labels_stack.push_front(make_shared_ptr(new ActiveDependencyLabels(*l)));
        }


        void add(const SanitisedDependency & dep)
        {
            std::tr1::shared_ptr<DependencySpecTree> if_rewritten(resolver.rewrite_if_special(dep.spec(), our_qpn_s));
            if (if_rewritten)
                if_rewritten->root()->accept(*this);
            else
                sanitised_dependencies.add(dep);
        }

        SanitisedDependency make_sanitised(const PackageOrBlockDepSpec & spec)
        {
            std::stringstream adl;
            for (DependencySystemLabelSequence::ConstIterator i((*labels_stack.begin())->system_labels()->begin()),
                    i_end((*labels_stack.begin())->system_labels()->end()) ;
                    i != i_end ; ++i)
                adl << (adl.str().empty() ? "" : ", ") << stringify(**i);

            for (DependencyTypeLabelSequence::ConstIterator i((*labels_stack.begin())->type_labels()->begin()),
                    i_end((*labels_stack.begin())->type_labels()->end()) ;
                    i != i_end ; ++i)
                adl << (adl.str().empty() ? "" : ", ") << stringify(**i);

            for (DependencyABIsLabelSequence::ConstIterator i((*labels_stack.begin())->abi_labels()->begin()),
                    i_end((*labels_stack.begin())->abi_labels()->end()) ;
                    i != i_end ; ++i)
                adl << (adl.str().empty() ? "" : ", ") << stringify(**i);

            for (DependencySuggestLabelSequence::ConstIterator i((*labels_stack.begin())->suggest_labels()->begin()),
                    i_end((*labels_stack.begin())->suggest_labels()->end()) ;
                    i != i_end ; ++i)
                adl << (adl.str().empty() ? "" : ", ") << stringify(**i);

            return make_named_values<SanitisedDependency>(
                    value_for<n::active_dependency_labels>(*labels_stack.begin()),
                    value_for<n::active_dependency_labels_as_string>(adl.str()),
                    value_for<n::metadata_key_human_name>(human_name),
                    value_for<n::metadata_key_raw_name>(raw_name),
                    value_for<n::original_specs_as_string>(original_specs_as_string),
                    value_for<n::spec>(spec)
                    );
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            add(make_sanitised(*node.spec()));
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node)
        {
            add(make_sanitised(*node.spec()));
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (node.spec()->condition_met())
            {
                labels_stack.push_front(make_shared_ptr(new ActiveDependencyLabels(**labels_stack.begin())));
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                labels_stack.pop_front();
            }
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            labels_stack.push_front(make_shared_ptr(new ActiveDependencyLabels(**labels_stack.begin())));
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            labels_stack.pop_front();
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            Save<std::string> save_original_specs_as_string(&original_specs_as_string);

            {
                MakeAnyOfStringVisitor v;
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(v));
                original_specs_as_string = "|| (" + v.result + " )";
            }

            AnyDepSpecChildHandler h(resolver, our_qpn_s, std::tr1::bind(&Finder::make_sanitised, this, std::tr1::placeholders::_1));
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(h));
            h.commit(
                    std::tr1::bind(&Finder::make_sanitised, this, std::tr1::placeholders::_1),
                    std::tr1::bind(&Finder::add, this, std::tr1::placeholders::_1)
                    );
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type &) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "not implemented");
        }

        void visit(const DependencySpecTree::NodeType<DependencyLabelsDepSpec>::Type & node)
        {
            labels_stack.begin()->reset(new ActiveDependencyLabels(**labels_stack.begin(), *node.spec()));
        }
    };
}

namespace paludis
{
    template <>
    struct Implementation<SanitisedDependencies>
    {
        std::list<SanitisedDependency> sanitised_dependencies;
    };
}

SanitisedDependencies::SanitisedDependencies() :
    PrivateImplementationPattern<SanitisedDependencies>(new Implementation<SanitisedDependencies>)
{
}

SanitisedDependencies::~SanitisedDependencies()
{
}

void
SanitisedDependencies::_populate_one(
        const Resolver & resolver,
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > (PackageID::* const pmf) () const
        )
{
    Context context("When finding dependencies for '" + stringify(*id) + "' from key '" + ((*id).*pmf)()->raw_name() + "':");

    Finder f(resolver, QPN_S(id), *this, ((*id).*pmf)()->initial_labels(), ((*id).*pmf)()->raw_name(),
            ((*id).*pmf)()->human_name(), "");
    ((*id).*pmf)()->value()->root()->accept(f);
}

void
SanitisedDependencies::populate(
        const Resolver & resolver,
        const std::tr1::shared_ptr<const PackageID> & id)
{
    Context context("When finding dependencies for '" + stringify(*id) + "':");

    if (id->dependencies_key())
        _populate_one(resolver, id, &PackageID::dependencies_key);
    else
    {
        if (id->build_dependencies_key())
            _populate_one(resolver, id, &PackageID::build_dependencies_key);
        if (id->run_dependencies_key())
            _populate_one(resolver, id, &PackageID::run_dependencies_key);
        if (id->post_dependencies_key())
            _populate_one(resolver, id, &PackageID::post_dependencies_key);
        if (id->suggested_dependencies_key())
            _populate_one(resolver, id, &PackageID::suggested_dependencies_key);
    }
}

void
SanitisedDependencies::add(const SanitisedDependency & dep)
{
    _imp->sanitised_dependencies.push_back(dep);
}

SanitisedDependencies::ConstIterator
SanitisedDependencies::begin() const
{
    return ConstIterator(_imp->sanitised_dependencies.begin());
}

SanitisedDependencies::ConstIterator
SanitisedDependencies::end() const
{
    return ConstIterator(_imp->sanitised_dependencies.end());
}

std::ostream &
paludis::resolver::operator<< (std::ostream & s, const PackageOrBlockDepSpec & d)
{
    if (d.if_package())
        return s << *d.if_package();
    else
        return s << *d.if_block();
}

PackageOrBlockDepSpec::PackageOrBlockDepSpec(const BlockDepSpec & s) :
    if_block(value_for<n::if_block>(make_shared_ptr(new BlockDepSpec(s)))),
    if_package(value_for<n::if_package>(make_null_shared_ptr()))
{
}

PackageOrBlockDepSpec::PackageOrBlockDepSpec(const PackageDepSpec & s) :
    if_block(value_for<n::if_block>(make_null_shared_ptr())),
    if_package(value_for<n::if_package>(make_shared_ptr(new PackageDepSpec(s))))
{
}

void
PackageOrBlockDepSpec::serialise(Serialiser & s) const
{
    if (if_block())
    {
        s.object("PackageOrBlockDepSpec")
            .member(SerialiserFlags<>(), "block", true)
            .member(SerialiserFlags<>(), "spec", stringify(if_block()->blocking()))
            .member(SerialiserFlags<>(), "strong", if_block()->strong())
            .member(SerialiserFlags<>(), "text", if_block()->text())
            ;
    }
    else
    {
        s.object("PackageOrBlockDepSpec")
            .member(SerialiserFlags<>(), "block", false)
            .member(SerialiserFlags<>(), "spec", stringify(*if_package()))
            ;
    }
}

PackageOrBlockDepSpec
PackageOrBlockDepSpec::deserialise(Deserialisation & d, const std::tr1::shared_ptr<const PackageID> & for_id)
{
    Context context("When deserialising '" + d.raw_string() + "':");

    Deserialisator v(d, "PackageOrBlockDepSpec");

    bool block(v.member<bool>("block"));
    PackageDepSpec spec(parse_elike_package_dep_spec(v.member<std::string>("spec"),
                ELikePackageDepSpecOptions() + epdso_allow_tilde_greater_deps + epdso_nice_equal_star +
                epdso_allow_ranged_deps + epdso_allow_use_deps + epdso_allow_use_deps_portage +
                epdso_allow_use_dep_defaults + epdso_allow_repository_deps + epdso_allow_slot_star_deps +
                epdso_allow_slot_equal_deps + epdso_allow_slot_deps + epdso_allow_key_requirements,
                VersionSpecOptions() + vso_flexible_dashes + vso_flexible_dots + vso_ignore_case +
                vso_letters_anywhere + vso_dotted_suffixes,
                for_id));

    if (block)
    {
        bool strong(v.member<bool>("strong"));
        std::string text(v.member<std::string>("text"));
        return PackageOrBlockDepSpec(BlockDepSpec(text, spec, strong));
    }
    else
        return PackageOrBlockDepSpec(spec);
}

void
SanitisedDependency::serialise(Serialiser & s) const
{
    s.object("SanitisedDependency")
        .member(SerialiserFlags<>(), "active_dependency_labels_as_string", active_dependency_labels_as_string())
        .member(SerialiserFlags<>(), "metadata_key_human_name", metadata_key_human_name())
        .member(SerialiserFlags<>(), "metadata_key_raw_name", metadata_key_raw_name())
        .member(SerialiserFlags<>(), "original_specs_as_string", original_specs_as_string())
        .member(SerialiserFlags<>(), "spec", spec())
        ;
}

SanitisedDependency
SanitisedDependency::deserialise(Deserialisation & d, const std::tr1::shared_ptr<const PackageID> & from_id)
{
    Context context("When deserialising '" + d.raw_string() + "':");

    Deserialisator v(d, "SanitisedDependency");

    return make_named_values<SanitisedDependency>(
            value_for<n::active_dependency_labels>(make_null_shared_ptr()),
            value_for<n::active_dependency_labels_as_string>(v.member<std::string>("active_dependency_labels_as_string")),
            value_for<n::metadata_key_human_name>(v.member<std::string>("metadata_key_human_name")),
            value_for<n::metadata_key_raw_name>(v.member<std::string>("metadata_key_raw_name")),
            value_for<n::original_specs_as_string>(v.member<std::string>("original_specs_as_string")),
            value_for<n::spec>(PackageOrBlockDepSpec::deserialise(*v.find_remove_member("spec"),
                    from_id))
            );
}

template class WrappedForwardIterator<SanitisedDependencies::ConstIteratorTag, const SanitisedDependency>;

