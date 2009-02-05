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

#include <paludis/repositories/unwritten/unwritten_repository_file.hh>
#include <paludis/repositories/unwritten/unwritten_repository.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/simple_parser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/join.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/formatter.hh>
#include <list>
#include <fstream>

using namespace paludis;
using namespace paludis::unwritten_repository;

namespace paludis
{
    template <>
    struct Implementation<UnwrittenRepositoryFile>
    {
        std::list<UnwrittenRepositoryFileEntry> entries;
    };
}

UnwrittenRepositoryFile::UnwrittenRepositoryFile(const FSEntry & f) :
    PrivateImplementationPattern<UnwrittenRepositoryFile>(new Implementation<UnwrittenRepositoryFile>)
{
    _load(f);
}

UnwrittenRepositoryFile::~UnwrittenRepositoryFile()
{
}

UnwrittenRepositoryFile::ConstIterator
UnwrittenRepositoryFile::begin() const
{
    return ConstIterator(_imp->entries.begin());
}

UnwrittenRepositoryFile::ConstIterator
UnwrittenRepositoryFile::end() const
{
    return ConstIterator(_imp->entries.end());
}

namespace
{
    struct UnwrittenHomepagePrinter
    {
        std::stringstream s;
        const SimpleURISpecTree::ItemFormatter & formatter;

        UnwrittenHomepagePrinter(const SimpleURISpecTree::ItemFormatter & f) :
            formatter(f)
        {
        }

        void visit(const SimpleURISpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const SimpleURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const SimpleURISpecTree::NodeType<SimpleURIDepSpec>::Type & node)
        {
            if (! s.str().empty())
                s << " ";
            s << formatter.format(*node.spec(), format::Plain());
        }
    };

    struct UnwrittenHomepageKey :
        MetadataSpecTreeKey<SimpleURISpecTree>
    {
        const std::tr1::shared_ptr<const SimpleURISpecTree> vv;

        const std::string _raw_name;
        const std::string _human_name;
        const MetadataKeyType _type;

        UnwrittenHomepageKey(const std::string & r, const std::string & h, const MetadataKeyType t,
                const std::tr1::shared_ptr<const SimpleURISpecTree> & v) :
            vv(v),
            _raw_name(r),
            _human_name(h),
            _type(t)
        {
        }

        const std::tr1::shared_ptr<const SimpleURISpecTree> value() const
        {
            return vv;
        }

        std::string pretty_print(const SimpleURISpecTree::ItemFormatter & f) const
        {
            UnwrittenHomepagePrinter p(f);
            value()->root()->accept(p);
            return p.s.str();
        }

        std::string pretty_print_flat(const SimpleURISpecTree::ItemFormatter & f) const
        {
            UnwrittenHomepagePrinter p(f);
            value()->root()->accept(p);
            return p.s.str();
        }

        virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return _raw_name;
        }

        virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return _human_name;
        }

        virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return _type;
        }
    };
}

void
UnwrittenRepositoryFile::_load(const FSEntry & f)
{
    std::ifstream file(stringify(f).c_str());
    if (! file)
        throw UnwrittenRepositoryConfigurationError("Cannot read '" + stringify(f) + "'");

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            break;

        std::string key, value;
        SimpleParser line_parser(line);
        if (line_parser.consume(
                    (+simple_parser::any_except(" \t") >> key) &
                    (*simple_parser::any_of(" \t")) &
                    (simple_parser::exact("=")) &
                    (*simple_parser::any_of(" \t")) &
                    (*simple_parser::any_except("") >> value)
                    ))
        {
            if (key == "format")
            {
                if (value != "unwritten-1")
                    throw UnwrittenRepositoryConfigurationError(
                            "Unsupported format '" + value + "' in '" + stringify(f) + "'");
            }
            else
                Log::get_instance()->message("unwritten_repository.file.unknown_key", ll_warning, lc_context)
                    << "Ignoring unknown key '" << key << "' with value '" << value << "'";
        }
        else
            throw UnwrittenRepositoryConfigurationError(
                    "Cannot parse header line '" + line + "' in '" + stringify(f) + "'");
    }

    CategoryNamePart category("x");
    PackageNamePart package("x");
    std::tr1::shared_ptr<MetadataValueKey<SlotName> > slot;
    VersionSpec version("0");
    std::tr1::shared_ptr<UnwrittenRepositoryFileEntry> entry;
    while (std::getline(file, line))
    {
        SimpleParser line_parser(line);

        std::string token, token2;
        if (line.empty())
        {
        }
        else if (line_parser.consume(
                    (*simple_parser::any_of(" \t")) &
                    (simple_parser::exact("#")) &
                    (*simple_parser::any_except(""))))
        {
        }
        else if (line_parser.consume(
                    (+simple_parser::any_except(" \t/") >> token) &
                    (simple_parser::exact("/"))
                    ))
        {
            if (! line_parser.eof())
                throw UnwrittenRepositoryConfigurationError(
                        "Cannot parse body category line '" + line + "' in '" + stringify(f) + "'");

            category = CategoryNamePart(token);
        }
        else if (line_parser.consume(
                    (+simple_parser::any_of(" \t")) &
                    (+simple_parser::any_except(" \t/") >> token) &
                    (simple_parser::exact("/"))
                    ))
        {
            if (! line_parser.eof())
                throw UnwrittenRepositoryConfigurationError(
                        "Cannot parse body package line '" + line + " in '" + stringify(f) + "'");

            package = PackageNamePart(token);
        }
        else if (line_parser.consume(
                    (+simple_parser::any_of(" \t")) &
                    (+simple_parser::exact(":")) &
                    (+simple_parser::any_except(" \t") >> token) &
                    (+simple_parser::any_of(" \t"))
                    ))
        {
            slot.reset(new LiteralMetadataValueKey<SlotName>("SLOT", "Slot", mkt_internal, SlotName(token)));

            if (line_parser.consume(
                        (+simple_parser::any_except(" \t") >> token)
                        ))
                version = VersionSpec(token);
            else
                throw UnwrittenRepositoryConfigurationError(
                        "Cannot parse body slot+version line '" + line + " in '" + stringify(f) + "'");

            if (! line_parser.eof())
                throw UnwrittenRepositoryConfigurationError(
                        "Cannot parse body slot+version line '" + line + " in '" + stringify(f) + "'");

            if (entry)
            {
                _imp->entries.push_back(*entry);
                entry.reset();
            }
        }
        else if (line_parser.consume(
                    (+simple_parser::any_of(" \t")) &
                    (+simple_parser::any_except(" \t") >> token) &
                    (+simple_parser::any_of(" \t")) &
                    (+simple_parser::exact("=")) &
                    (+simple_parser::any_of(" \t")) &
                    (+simple_parser::any_except("") >> token2)
                    ))
        {
            if (! line_parser.eof())
                throw UnwrittenRepositoryConfigurationError(
                        "Cannot parse body key = value line '" + line + " in '" + stringify(f) + "'");

            if (! entry)
                entry.reset(new UnwrittenRepositoryFileEntry(make_named_values<UnwrittenRepositoryFileEntry>(
                                value_for<n::added_by>(std::tr1::shared_ptr<const MetadataValueKey<std::string> >()),
                                value_for<n::bug_ids>(std::tr1::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > >()),
                                value_for<n::comment>(std::tr1::shared_ptr<const MetadataValueKey<std::string> >()),
                                value_for<n::description>(std::tr1::shared_ptr<const MetadataValueKey<std::string> >()),
                                value_for<n::homepage>(std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >()),
                                value_for<n::name>(category + package),
                                value_for<n::remote_ids>(std::tr1::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > >()),
                                value_for<n::slot>(slot),
                                value_for<n::version>(version)
                                )));

            if (token == "description")
                entry->description().reset(new LiteralMetadataValueKey<std::string>("description", "Description", mkt_significant, token2));
            else if (token == "homepage")
            {
                std::tr1::shared_ptr<AllDepSpec> all_spec(new AllDepSpec);
                std::tr1::shared_ptr<SimpleURISpecTree> tree(new SimpleURISpecTree(all_spec));
                std::list<std::string> tokens;
                tokenise_whitespace(token2, std::back_inserter(tokens));
                for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    tree->root()->append(make_shared_ptr(new SimpleURIDepSpec(*t)));
                entry->homepage().reset(new UnwrittenHomepageKey("homepage", "Homepage", mkt_normal, tree));
            }
            else if (token == "comment")
                entry->comment().reset(new LiteralMetadataValueKey<std::string>("comment", "Comment", mkt_normal, token2));
            else if (token == "added-by")
                entry->added_by().reset(new LiteralMetadataValueKey<std::string>("added-by", "Added by", mkt_author, token2));
            else if (token == "bug-ids")
            {
                std::tr1::shared_ptr<Sequence<std::string> > seq(new Sequence<std::string>);
                std::list<std::string> tokens;
                tokenise_whitespace(token2, std::back_inserter(tokens));
                for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    seq->push_back(*t);
                entry->bug_ids().reset(new LiteralMetadataStringSequenceKey("bug-ids", "Bug IDs", mkt_normal, seq));
            }
            else if (token == "remote-ids")
            {
                std::tr1::shared_ptr<Sequence<std::string> > seq(new Sequence<std::string>);
                std::list<std::string> tokens;
                tokenise_whitespace(token2, std::back_inserter(tokens));
                for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    seq->push_back(*t);
                entry->remote_ids().reset(new LiteralMetadataStringSequenceKey("remote-ids", "Remote IDs", mkt_internal, seq));
            }
            else
                Log::get_instance()->message("unwritten_repository.file.unknown_key", ll_warning, lc_context)
                    << "Ignoring unknown key '" << token << "' with value '" << token2 << "'";
        }
        else
            throw UnwrittenRepositoryConfigurationError(
                    "Cannot parse body line '" + line + " in '" + stringify(f) + "'");
    }

    if (entry)
        _imp->entries.push_back(*entry);
}

template class PrivateImplementationPattern<UnwrittenRepositoryFile>;
template class WrappedForwardIterator<UnwrittenRepositoryFile::ConstIteratorTag,
         const UnwrittenRepositoryFileEntry>;


