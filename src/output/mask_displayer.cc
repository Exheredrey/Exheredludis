/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include "mask_displayer.hh"
#include "colour.hh"
#include "colour_pretty_printer.hh"
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/name.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/dep_spec.hh>
#include <sstream>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<MaskDisplayer>
    {
        std::ostringstream s;

        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        const bool want_description;

        Imp(const Environment * const e, const std::shared_ptr<const PackageID> & i,
                const bool w) :
            env(e),
            id(i),
            want_description(w)
        {
        }
    };
}

namespace
{
    struct KeyPrettyPrinter
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        std::ostringstream s;

        KeyPrettyPrinter(const Environment * const e, const std::shared_ptr<const PackageID> & i) :
            env(e),
            id(i)
        {
        }

        void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k)
        {
            s << *k.value();
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            s << k.value();
        }

        void visit(const MetadataValueKey<SlotName> & k)
        {
            s << k.value();
        }

        void visit(const MetadataValueKey<long> & k)
        {
            s << k.value();
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            s << k.value();
        }

        void visit(const MetadataSectionKey & k)
        {
            s << "(";

            bool need_comma(false);
            for (MetadataSectionKey::MetadataConstIterator m(k.begin_metadata()), m_end(k.end_metadata()) ;
                    m != m_end ; ++m)
            {
                if (need_comma)
                    s << ", ";
                else
                    s << " ";

                KeyPrettyPrinter p(env, id);
                (*m)->accept(p);
                s << p.s.str();
                need_comma = true;
            }
            s << " )";
        }

        void visit(const MetadataTimeKey & k)
        {
            s << pretty_print_time(k.value().seconds());
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Contents> > &)
        {
        }

        void visit(const MetadataValueKey<std::shared_ptr<const RepositoryMaskInfo> > & k)
        {
            s << (*k.value()).mask_file() << ": " <<
                join((*k.value()).comment()->begin(), (*k.value()).comment()->end(), " ");
        }

        void visit(const MetadataValueKey<FSPath> & k)
        {
            s << k.value();
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataCollectionKey<Map<std::string, std::string> > & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataCollectionKey<FSPathSequence> & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & k)
        {
            ColourPrettyPrinter printer(env, id);
            s << k.pretty_print_value(printer, { });
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Choices> > & k)
        {
            s << k.human_name();
        }
    };
}

MaskDisplayer::MaskDisplayer(const Environment * const e, const std::shared_ptr<const PackageID> & id,
        const bool want_description) :
    _imp(e, id, want_description)
{
}

MaskDisplayer::~MaskDisplayer()
{
}

std::string
MaskDisplayer::result() const
{
    return _imp->s.str();
}

void
MaskDisplayer::visit(const UnacceptedMask & m)
{
    if (_imp->want_description)
        _imp->s << m.description() << " (";

    if (! m.unaccepted_key_name().empty())
    {
        KeyPrettyPrinter k(_imp->env, _imp->id);
        (*_imp->id->find_metadata(m.unaccepted_key_name()))->accept(k);
        _imp->s << k.s.str();
    }

    if (_imp->want_description)
        _imp->s << ")";
}

void
MaskDisplayer::visit(const UserMask & m)
{
    _imp->s << m.description();
}

void
MaskDisplayer::visit(const RepositoryMask & m)
{
    if (_imp->want_description)
    {
        _imp->s << m.description();

        if (! m.mask_key_name().empty())
        {
            KeyPrettyPrinter k(_imp->env, _imp->id);
            (*_imp->id->find_metadata(m.mask_key_name()))->accept(k);
            _imp->s << " (" << k.s.str() << ")";
        }
    }
    else
    {
        if (! m.mask_key_name().empty())
        {
            KeyPrettyPrinter k(_imp->env, _imp->id);
            (*_imp->id->find_metadata(m.mask_key_name()))->accept(k);
            _imp->s << k.s.str();
        }
        else
            _imp->s << m.description();
    }
}

void
MaskDisplayer::visit(const UnsupportedMask & m)
{
    if (_imp->want_description)
        _imp->s << m.description() << " (" << m.explanation() << ")";
    else
        _imp->s << m.explanation();
}

void
MaskDisplayer::visit(const AssociationMask & m)
{
    if (_imp->want_description)
        _imp->s << m.description() << " (associated package '" << m.associated_package_spec() << "')";
    else
        _imp->s << m.associated_package_spec();
}

