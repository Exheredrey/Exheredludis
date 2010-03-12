/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include "cmd_info.hh"
#include "colour_formatter.hh"
#include "format_general.hh"
#include "formats.hh"
#include "exceptions.hh"
#include "select_format_for_spec.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/action.hh>
#include <paludis/about_metadata.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct InfoCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave info";
        }

        virtual std::string app_synopsis() const
        {
            return "Display a summary of configuration and package information.";
        }

        virtual std::string app_description() const
        {
            return "Displays a formatted summary of configuration and package information, e.g. for "
                "use when submitting bug reports. The package names of any relevant packages for bug "
                "reports should be passed as parameters.";
        }

        InfoCommandLine()
        {
            add_usage_line("[ spec ... ]");
        }
    };

    struct MetadataKeyComparator
    {
        bool operator() (const std::tr1::shared_ptr<const MetadataKey> & a, const std::tr1::shared_ptr<const MetadataKey> & b) const
        {
            bool a_is_section(simple_visitor_cast<const MetadataSectionKey>(*a));
            bool b_is_section(simple_visitor_cast<const MetadataSectionKey>(*b));
            if (a_is_section != b_is_section)
                return b_is_section;
            if (a->type() != b->type())
                return a->type() < b->type();
            return a->human_name() < b->human_name();
        }
    };

    struct ContentsDisplayer
    {
        const unsigned indent;
        std::stringstream s;

        ContentsDisplayer(const unsigned i) :
            indent(i)
        {
        }

        void visit(const ContentsFileEntry & e)
        {
            s << format_general_rhvib(f::show_contents_file(), stringify(e.location_key()->value()), e.location_key()->value().basename(),
                    "", indent, indent);
        }

        void visit(const ContentsDirEntry & e)
        {
            s << format_general_rhvib(f::show_contents_dir(), stringify(e.location_key()->value()), e.location_key()->value().basename(),
                    "", indent, indent);
        }

        void visit(const ContentsSymEntry & e)
        {
            s << format_general_rhvib(f::show_contents_sym(), stringify(e.location_key()->value()), e.location_key()->value().basename(),
                    e.target_key()->value(), indent, indent);
        }

        void visit(const ContentsOtherEntry & e)
        {
            s << format_general_rhvib(f::show_contents_other(), stringify(e.location_key()->value()), e.location_key()->value().basename(),
                    "", indent, indent);
        }
    };

    struct InfoDisplayer
    {
        const InfoCommandLine & cmdline;
        const int indent;

        InfoDisplayer(const InfoCommandLine & c, const int i) :
            cmdline(c),
            indent(i)
        {
        }

        void visit(const MetadataSectionKey & k)
        {
            cout << format_general_his(f::info_metadata_subsection(), k.human_name(), indent, k.human_name());
            std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(k.begin_metadata(), k.end_metadata());
            for (std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                    s(keys.begin()), s_end(keys.end()) ; s != s_end ; ++s)
            {
                InfoDisplayer i(cmdline, indent + 1);
                (*s)->accept(i);
            }
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, k.pretty_print_flat(f));
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, k.pretty_print_flat(f));
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, k.pretty_print_flat(f));
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, k.pretty_print_flat(f));
        }

        void visit(const MetadataCollectionKey<FSEntrySequence> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent,
                    join(k.value()->begin(), k.value()->end(), "  "));
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, k.pretty_print_flat(f));
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, k.pretty_print_flat(f));
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, k.pretty_print_flat(f));
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, k.pretty_print_flat(f));
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, k.pretty_print_flat(f));
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, k.pretty_print_flat(f));
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, stringify(k.value()));
        }

        void visit(const MetadataValueKey<SlotName> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, stringify(k.value()));
        }

        void visit(const MetadataValueKey<long> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, stringify(k.value()));
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, stringify(k.value()));
        }

        void visit(const MetadataValueKey<FSEntry> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, stringify(k.value()));
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, stringify(*k.value()));
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const Contents> > &)
        {
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const Choices> > &)
        {
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> > &)
        {
        }

        void visit(const MetadataTimeKey & k)
        {
            ColourFormatter f(indent);
            cout << format_general_his(f::info_metadata(), k.human_name(), indent, pretty_print_time(k.value().seconds()));
        }
    };

    void do_one_repository(
            const InfoCommandLine & cmdline,
            const std::tr1::shared_ptr<Environment> &,
            const std::tr1::shared_ptr<const Repository> & repo)
    {
        cout << format_general_s(f::info_repository_heading(), stringify(repo->name()));
        std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(repo->begin_metadata(), repo->end_metadata());
        for (std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                k(keys.begin()), k_end(keys.end()) ; k != k_end ; ++k)
        {
            if ((*k)->type() == mkt_internal)
                continue;

            InfoDisplayer i(cmdline, 1);
            (*k)->accept(i);
        }
        cout << endl;
    }

    void do_env(
            const InfoCommandLine & cmdline,
            const std::tr1::shared_ptr<Environment> & env)
    {
        cout << format_general_s(f::info_heading(), "Environment Information");
        std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(env->begin_metadata(), env->end_metadata());
        for (std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                k(keys.begin()), k_end(keys.end()) ; k != k_end ; ++k)
        {
            if ((*k)->type() == mkt_internal)
                continue;

            InfoDisplayer i(cmdline, 1);
            (*k)->accept(i);
        }
        cout << endl;
    }

    void do_about(
            const InfoCommandLine & cmdline,
            const std::tr1::shared_ptr<Environment> &)
    {
        cout << format_general_s(f::info_heading(), "Package Manager Information");
        std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(AboutMetadata::get_instance()->begin_metadata(),
                AboutMetadata::get_instance()->end_metadata());
        for (std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                k(keys.begin()), k_end(keys.end()) ; k != k_end ; ++k)
        {
            if ((*k)->type() == mkt_internal)
                continue;

            InfoDisplayer i(cmdline, 1);
            (*k)->accept(i);
        }
        cout << endl;
    }
}

bool
InfoCommand::important() const
{
    return true;
}

int
InfoCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    InfoCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_INFO_OPTIONS", "CAVE_INFO_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    do_about(cmdline, env);
    do_env(cmdline, env);

    for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
            r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
        do_one_repository(cmdline, env, *r);

    if (cmdline.begin_parameters() == cmdline.end_parameters())
    {
    }
    else
    {
        for (InfoCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
                p != p_end ; ++p)
        {
        }
    }

    return EXIT_SUCCESS;
}

std::tr1::shared_ptr<args::ArgsHandler>
InfoCommand::make_doc_cmdline()
{
    return make_shared_ptr(new InfoCommandLine);
}

