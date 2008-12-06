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

#include "dep_tag.hh"
#include <paludis/dep_spec.hh>
#include <paludis/dep_label.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/mutex.hh>
#include <sstream>

using namespace paludis;

template class ConstVisitor<DepTagVisitorTypes>;
template class ConstAcceptInterface<DepTagVisitorTypes>;

template class ConstAcceptInterfaceVisitsThis<DepTagVisitorTypes, GeneralSetDepTag>;
template class ConstAcceptInterfaceVisitsThis<DepTagVisitorTypes, GLSADepTag>;
template class ConstAcceptInterfaceVisitsThis<DepTagVisitorTypes, DependencyDepTag>;
template class ConstAcceptInterfaceVisitsThis<DepTagVisitorTypes, TargetDepTag>;

template class Visits<const GeneralSetDepTag>;
template class Visits<const GLSADepTag>;
template class Visits<const DependencyDepTag>;
template class Visits<const TargetDepTag>;

template class InstantiationPolicy<DepTagCategoryFactory, instantiation_method::SingletonTag>;

template class Set<DepTagEntry, DepTagEntryComparator>;
template class WrappedForwardIterator<Set<DepTagEntry, DepTagEntryComparator>::ConstIteratorTag, const DepTagEntry>;
template class WrappedOutputIterator<Set<DepTagEntry, DepTagEntryComparator>::InserterTag, DepTagEntry>;

template class PrivateImplementationPattern<GeneralSetDepTag>;
template class PrivateImplementationPattern<DependencyDepTag>;

namespace
{
    std::tr1::shared_ptr<DepTagCategory>
    make_glsa_dep_tag()
    {
        return std::tr1::shared_ptr<DepTagCategory>(new DepTagCategory(
                    true,
                    "glsa",
                    "Security advisories",
                    "Your system is potentially affected by these security issues:",
                    "Please read the advisories carefully and take appropriate action."));
    }

    std::tr1::shared_ptr<DepTagCategory>
    make_general_set_dep_tag()
    {
        return std::tr1::shared_ptr<DepTagCategory>(new DepTagCategory(
                    true,
                    "general",
                    "General sets",
                    "",
                    ""));
    }

    std::tr1::shared_ptr<DepTagCategory>
    make_dependency_set_dep_tag()
    {
        return std::tr1::shared_ptr<DepTagCategory>(new DepTagCategory(
                    false,
                    "dependency",
                    "Dependencies",
                    "",
                    ""));
    }

    std::tr1::shared_ptr<DepTagCategory>
    make_target_dep_tag()
    {
        return std::tr1::shared_ptr<DepTagCategory>(new DepTagCategory(
                    false,
                    "target",
                    "Targets",
                    "",
                    ""));
    }
}

DepTagCategory::DepTagCategory(
        bool vis,
        const std::string & our_id,
        const std::string & t, const std::string & pre,
        const std::string & post) :
    _visible(vis),
    _id(our_id),
    _title(t),
    _pre_text(pre),
    _post_text(post)
{
}

bool
DepTagCategory::visible() const
{
    return _visible;
}

std::string
DepTagCategory::id() const
{
    return _id;
}

std::string
DepTagCategory::title() const
{
    return _title;
}

std::string
DepTagCategory::pre_text() const
{
    return _pre_text;
}

std::string
DepTagCategory::post_text() const
{
    return _post_text;
}

DepTag::DepTag()
{
}

DepTag::~DepTag()
{
}

namespace
{
    struct DepSpecStringifier :
        ConstVisitor<DependencySpecTree>
    {
        std::ostringstream s;

        void
        visit_sequence(const AllDepSpec &,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            s << "( ";
            std::for_each(cur, end, accept_visitor(*this));
            s << ") ";
        }

        void
        visit_sequence(const AnyDepSpec &,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            s << "|| ( ";
            std::for_each(cur, end, accept_visitor(*this));
            s << ") ";
        }

        void
        visit_sequence(const ConditionalDepSpec & a,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            s << stringify(a) << " ( ";
            std::for_each(cur, end, accept_visitor(*this));
            s << ") ";
        }

        void
        visit_leaf(const PackageDepSpec & p)
        {
            s << p << " ";
        }

        void
        visit_leaf(const BlockDepSpec & b)
        {
            s << b << " ";
        }

        void
        visit_leaf(const DependencyLabelsDepSpec & l)
        {
            std::copy(indirect_iterator(l.begin()), indirect_iterator(l.end()),
                    std::ostream_iterator<DependencyLabelVisitorTypes::BasicNode>(s, ","));
            s << ":";
        }

        void
        visit_leaf(const NamedSetDepSpec & p)
        {
            s << p << " ";
        }
    };

    struct DepTagComparator :
        ConstVisitor<DepTagVisitorTypes>
    {
        std::string value;

        void visit(const GLSADepTag & t)
        {
            value = t.short_text();
        }

        void visit(const GeneralSetDepTag & t)
        {
            value = t.short_text();
        }

        void visit(const DependencyDepTag & t)
        {
            value = stringify(*t.package_id()) + "," + stringify(*t.dependency()) + ",";
            DepSpecStringifier s;
            t.conditions()->accept(s);
            value.append(s.s.str());
        }

        void visit(const TargetDepTag & t)
        {
            value = t.short_text();
        }
    };
}

bool
DepTag::operator== (const DepTag & other) const
{
    DepTagComparator c1, c2;
    accept(c1);
    other.accept(c2);
    return c1.value == c2.value;
}

bool
DepTag::operator< (const DepTag & other) const
{
    DepTagComparator c1, c2;
    accept(c1);
    other.accept(c2);
    return c1.value < c2.value;
}

GLSADepTag::GLSADepTag(const std::string & id, const std::string & our_glsa_title, const FSEntry& our_glsa_file) :
    _id(id),
    _glsa_title(our_glsa_title),
    _glsa_file(our_glsa_file)
{
}

GLSADepTag::~GLSADepTag()
{
}

std::string
GLSADepTag::short_text() const
{
    return "GLSA-" + _id;
}

std::string
GLSADepTag::category() const
{
    return "glsa";
}

const FSEntry
GLSADepTag::glsa_file() const
{
    return _glsa_file;
}

std::string
GLSADepTag::glsa_title() const
{
    return _glsa_title;
}

namespace paludis
{
    template <>
    struct Implementation<GeneralSetDepTag>
    {
        const SetName id;
        const std::string source;

        Implementation(const SetName & n, const std::string s) :
            id(n),
            source(s)
        {
        }
    };
}

GeneralSetDepTag::GeneralSetDepTag(const SetName & id, const std::string & r) :
    PrivateImplementationPattern<GeneralSetDepTag>(new Implementation<GeneralSetDepTag>(id, r))
{
}

GeneralSetDepTag::~GeneralSetDepTag()
{
}

std::string
GeneralSetDepTag::short_text() const
{
    return stringify(_imp->id);
}

std::string
GeneralSetDepTag::category() const
{
    return "general";
}

std::string
GeneralSetDepTag::source() const
{
    return _imp->source;
}

namespace paludis
{
    template <>
    struct Implementation<DependencyDepTag>
    {
        mutable Mutex mutex;
        mutable std::string str;

        std::tr1::shared_ptr<const PackageID> id;
        const std::tr1::shared_ptr<PackageDepSpec> spec;
        const std::tr1::shared_ptr<const DependencySpecTree::ConstItem> cond;

        Implementation(const std::tr1::shared_ptr<const PackageID> & i,
                const PackageDepSpec & d, const std::tr1::shared_ptr<const DependencySpecTree::ConstItem> & s) :
            id(i),
            spec(std::tr1::static_pointer_cast<PackageDepSpec>(d.clone())),
            cond(s)
        {
            spec->set_tag(std::tr1::shared_ptr<const DepTag>());
        }
    };
}

DependencyDepTag::DependencyDepTag(const std::tr1::shared_ptr<const PackageID> & i, const PackageDepSpec & d,
        const std::tr1::shared_ptr<const DependencySpecTree::ConstItem> & s) :
    PrivateImplementationPattern<DependencyDepTag>(new Implementation<DependencyDepTag>(i, d, s))
{
}

DependencyDepTag::~DependencyDepTag()
{
}

std::string
DependencyDepTag::short_text() const
{
    return stringify(*_imp->id);
}

std::string
DependencyDepTag::category() const
{
    return "dependency";
}

const std::tr1::shared_ptr<const PackageID>
DependencyDepTag::package_id() const
{
    return _imp->id;
}

const std::tr1::shared_ptr<const PackageDepSpec>
DependencyDepTag::dependency() const
{
    return _imp->spec;
}

const std::tr1::shared_ptr<const DependencySpecTree::ConstItem>
DependencyDepTag::conditions() const
{
    return _imp->cond;
}

TargetDepTag::TargetDepTag()
{
}

TargetDepTag::~TargetDepTag()
{
}

std::string
TargetDepTag::short_text() const
{
    return "target";
}

std::string
TargetDepTag::category() const
{
    return "target";
}

DepTagCategoryFactory::DepTagCategoryFactory()
{
}

const std::tr1::shared_ptr<DepTagCategory>
DepTagCategoryFactory::create(const std::string & s) const
{
    if (s == "glsa")
        return make_glsa_dep_tag();
    if (s == "general")
        return make_general_set_dep_tag();
    if (s == "dependency")
        return make_dependency_set_dep_tag();
    if (s == "target")
        return make_target_dep_tag();
    throw ConfigurationError("No dep tag category named '" + s + "'");
}

bool
DepTagEntryComparator::operator() (const DepTagEntry & l, const DepTagEntry & r) const
{
    return *l.tag() < *r.tag();
}

