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

#include "cmd_show.hh"
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
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/action.hh>
#include <paludis/mask.hh>
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
    struct ShowCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave show";
        }

        virtual std::string app_synopsis() const
        {
            return "Display a summary of a given object.";
        }

        virtual std::string app_description() const
        {
            return "Displays a formatted summary of a given object. If the object is a set, the set's "
                "contents are listed. If the object is a repository name, information about the repository "
                "is displayed. If the object is a package dep spec with wildcards, possible expansions "
                "are shown. If the object is a package dep spec without wildcards, information about matching "
                "IDs are shown.";
        }

        args::ArgsGroup g_object_options;
        args::EnumArg a_type;

        args::ArgsGroup g_key_options;
        args::SwitchArg a_complex_keys;
        args::SwitchArg a_internal_keys;

        args::ArgsGroup g_display_options;
        args::SwitchArg a_flat;

        ShowCommandLine() :
            g_object_options(this, "Object Options", "Alter how objects are interpreted."),
            a_type(&g_object_options, "type", '\0', "Specify the type of the specified objects.",
                    args::EnumArg::EnumArgOptions
                    ("auto",               "Automatically determine the type")
                    ("repository",         "Treat the objects as repository names")
                    ("set",                "Treat the objects as set names")
                    ("wildcard",           "Treat the objects as a wildcarded package spec")
                    ("package",            "Treat the objects as an unwildcarded package spec, showing all matches for wildcards"),
                    "auto"),
            g_key_options(this, "Key Options", "Control which keys are shown."),
            a_complex_keys(&g_key_options, "complex-keys", 'c',
                    "Show complex keys", true),
            a_internal_keys(&g_key_options, "internal-keys", 'i',
                    "Show keys regardless of importance, including internal-only values", true),
            g_display_options(this, "Display Options", "Controls the output format."),
            a_flat(&g_display_options, "flat", 'f',
                    "Do not spread key values over multiple lines", true)
        {
            add_usage_line("spec ...");
        }
    };

    struct SetDisplayer :
        ConstVisitor<SetSpecTree>
    {
        const std::tr1::shared_ptr<const Environment> env;

        SetDisplayer(const std::tr1::shared_ptr<const Environment> & e) :
            env(e)
        {
        }

        void visit_leaf(const PackageDepSpec & spec)
        {
            cout << format_general_s(select_format_for_spec(env, spec,
                        f::show_set_spec_installed(),
                        f::show_set_spec_installable(),
                        f::show_set_spec_unavailable()),
                    stringify(spec));
        }

        void visit_leaf(const NamedSetDepSpec & spec)
        {
            cout << format_general_s(f::show_set_set(), stringify(spec));
        }

        void visit_sequence(
                const AllDepSpec &,
                SetSpecTree::ConstSequenceIterator cur,
                SetSpecTree::ConstSequenceIterator end)
        {
            std::for_each(cur, end, accept_visitor(*this));
        }
    };

    void do_one_set(const std::tr1::shared_ptr<Environment> & env, const SetName & s)
    {
        cout << format_general_s(f::show_set_heading(), stringify(s));

        const std::tr1::shared_ptr<const SetSpecTree::ConstItem> set(env->set(s));
        if (! set)
            throw NoSuchSetError(stringify(s));

        SetDisplayer d(env);
        set->accept(d);

        cout << endl;
    }

    void do_one_wildcard(const std::tr1::shared_ptr<Environment> & env, const PackageDepSpec & s)
    {
        cout << format_general_s(f::show_wildcard_heading(), stringify(s));

        const std::tr1::shared_ptr<const PackageIDSequence> names((*env)[selection::BestVersionOnly(generator::Matches(s, MatchPackageOptions()))]);
        if (names->empty())
            throw NothingMatching(s);

        for (PackageIDSequence::ConstIterator i(names->begin()), i_end(names->end()) ;
                i != i_end ; ++i)
        {
            PackageDepSpec name_spec(make_package_dep_spec().package((*i)->name()));
            cout << format_general_s(select_format_for_spec(env, name_spec,
                        f::show_wildcard_spec_installed(),
                        f::show_wildcard_spec_installable(),
                        f::show_wildcard_spec_unavailable()
                        ),
                    stringify(name_spec));
        }

        cout << endl;
    }

    struct MetadataKeyComparator
    {
        bool operator() (const std::tr1::shared_ptr<const MetadataKey> & a, const std::tr1::shared_ptr<const MetadataKey> & b) const
        {
            bool a_is_section(visitor_cast<const MetadataSectionKey>(*a));
            bool b_is_section(visitor_cast<const MetadataSectionKey>(*b));
            if (a_is_section != b_is_section)
                return b_is_section;
            if (a->type() != b->type())
                return a->type() < b->type();
            return a->human_name() < b->human_name();
        }
    };

    struct ContentsDisplayer :
        ConstVisitor<ContentsVisitorTypes>
    {
        const unsigned indent;
        std::stringstream s;

        ContentsDisplayer(const unsigned i) :
            indent(i)
        {
        }

        void visit(const ContentsFileEntry & e)
        {
            s << format_general_rhvib(f::show_contents_file(), e.name(), FSEntry(e.name()).basename(),
                    "", indent, indent);
        }

        void visit(const ContentsDirEntry & e)
        {
            s << format_general_rhvib(f::show_contents_dir(), e.name(), FSEntry(e.name()).basename(),
                    "", indent, indent);
        }

        void visit(const ContentsMiscEntry & e)
        {
            s << format_general_rhvib(f::show_contents_misc(), e.name(), FSEntry(e.name()).basename(),
                    "", indent, indent);
        }

        void visit(const ContentsDevEntry & e)
        {
            s << format_general_rhvib(f::show_contents_dev(), e.name(), FSEntry(e.name()).basename(),
                    "", indent, indent);
        }

        void visit(const ContentsFifoEntry & e)
        {
            s << format_general_rhvib(f::show_contents_fifo(), e.name(), FSEntry(e.name()).basename(),
                    "", indent, indent);
        }

        void visit(const ContentsSymEntry & e)
        {
            s << format_general_rhvib(f::show_contents_sym(), e.name(), FSEntry(e.name()).basename(),
                    e.target(), indent, indent);
        }
    };

    struct InfoDisplayer :
        ConstVisitor<MetadataKeyVisitorTypes>
    {
        const ShowCommandLine & cmdline;
        const int indent;
        const bool important;

        InfoDisplayer(const ShowCommandLine & c, const int i, const bool m) :
            cmdline(c),
            indent(i),
            important(m)
        {
        }

        void visit(const MetadataSectionKey & k)
        {
            cout << format_general_rhvib(f::show_metadata_subsection(), k.raw_name(), k.human_name(), "",
                    indent, important);
            std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(k.begin_metadata(), k.end_metadata());
            for (std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                    s(keys.begin()), s_end(keys.end()) ; s != s_end ; ++s)
            {
                InfoDisplayer i(cmdline, indent + 1, ((*s)->type() == mkt_significant));
                if (cmdline.a_internal_keys.specified() || ((*s)->type() != mkt_internal))
                    accept_visitor(i)(**s);
            }
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                    k.pretty_print_flat(f), indent, important);
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            ColourFormatter f(indent);
            cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                    k.pretty_print_flat(f), indent, important);
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            ColourFormatter f(indent);
            cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                    k.pretty_print_flat(f), indent, important);
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                    k.pretty_print_flat(f), indent, important);
        }

        void visit(const MetadataCollectionKey<FSEntrySequence> & k)
        {
            cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                    join(k.value()->begin(), k.value()->end(), ", "), indent, important);
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            ColourFormatter f(indent);
            cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                    k.pretty_print_flat(f), indent, important);
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourFormatter f(indent);
                cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                        k.pretty_print_flat(f), indent, important);
            }
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourFormatter f(indent);
                if (cmdline.a_flat.specified())
                    cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                            k.pretty_print_flat(f), indent, important);
                else
                {
                    cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                            "", indent, important);
                    cout << k.pretty_print(f);
                }
            }
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourFormatter f(indent);
                if (cmdline.a_flat.specified())
                    cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                            k.pretty_print_flat(f), indent, important);
                else
                {
                    cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                            "", indent, important);
                    cout << k.pretty_print(f);
                }
            }
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourFormatter f(indent);
                cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                        k.pretty_print_flat(f), indent, important);
            }
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourFormatter f(indent);
                if (cmdline.a_flat.specified())
                    cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                            k.pretty_print_flat(f), indent, important);
                else
                {
                    cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                            "", indent, important);
                    cout << k.pretty_print(f);
                }
            }
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                    stringify(k.value()), indent, important);
        }

        void visit(const MetadataValueKey<long> & k)
        {
            cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                    stringify(k.value()), indent, important);
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                    stringify(k.value() ? "true" : "false"), indent, important);
        }

        void visit(const MetadataValueKey<FSEntry> & k)
        {
            cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                    stringify(k.value()), indent, important);
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > & k)
        {
            cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                    stringify(*k.value()), indent, important);
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const Contents> > & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                if (cmdline.a_flat.specified())
                {
                    ContentsDisplayer d(0);
                    std::for_each(indirect_iterator(k.value()->begin()),
                            indirect_iterator(k.value()->end()), accept_visitor(d));
                    cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                            d.s.str(), indent, important);
                }
                else
                {
                    ContentsDisplayer d(indent);
                    std::for_each(indirect_iterator(k.value()->begin()),
                            indirect_iterator(k.value()->end()), accept_visitor(d));
                    cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                            "", indent, important);
                    cout << d.s.str();
                }
            }
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const Choices> > &)
        {
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> > & k)
        {
            if (k.value())
            {
                cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                        stringify(k.value()->mask_file()), indent, important);
                for (Sequence<std::string>::ConstIterator i(k.value()->comment()->begin()), i_end(k.value()->comment()->end()) ;
                        i != i_end ; ++i)
                    cout << format_general_rhvib(f::show_metadata_continued_value(), k.raw_name(), k.human_name(),
                            *i, 0, important);
            }
        }

        void visit(const MetadataTimeKey & k)
        {
            char buf[255];
            time_t t(k.value());
            if (! strftime(buf, 255, "%c", gmtime(&t)))
                buf[0] = '\0';
            cout << format_general_rhvib(f::show_metadata_key_value(), k.raw_name(), k.human_name(),
                    std::string(buf), indent, important);
        }
    };

    struct MaskDisplayer :
        ConstVisitor<MaskVisitorTypes>
    {
        const ShowCommandLine & cmdline;
        const int indent;

        MaskDisplayer(const ShowCommandLine & c, const int i) :
            cmdline(c),
            indent(i)
        {
        }

        void visit(const UnacceptedMask & m)
        {
            if (m.unaccepted_key())
            {
                InfoDisplayer i(cmdline, indent, false);
                m.unaccepted_key()->accept(i);
            }
            else
            {
                cout << format_general_rhvib(f::show_metadata_key_value(), "Masked", "Masked",
                        "by " + m.description(), indent, false);
            }
        }

        void visit(const UnsupportedMask & m)
        {
            cout << format_general_rhvib(f::show_metadata_key_value(), stringify(m.key()),
                    m.description(), m.explanation(), indent, false);
        }

        void visit(const AssociationMask & m)
        {
            cout << format_general_rhvib(f::show_metadata_key_value(), stringify(m.key()),
                    "by " + m.description(), stringify(*m.associated_package()), indent, false);
        }

        void visit(const UserMask & m)
        {
            cout << format_general_rhvib(f::show_metadata_key_value(), stringify(m.key()),
                    "by " + m.description(), "", indent, false);
        }

        void visit(const RepositoryMask & m)
        {
            if (m.mask_key())
            {
                InfoDisplayer i(cmdline, indent, false);
                m.mask_key()->accept(i);
            }
            else
            {
                cout << format_general_rhvib(f::show_metadata_key_value(), stringify(m.key()),
                        "by " + m.description(), "", indent, false);
            }
        }
    };

    void do_one_repository(
            const ShowCommandLine & cmdline,
            const std::tr1::shared_ptr<Environment> & env,
            const RepositoryName & s)
    {
        cout << format_general_s(f::show_repository_heading(), stringify(s));

        const std::tr1::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(s));
        std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(repo->begin_metadata(), repo->end_metadata());
        for (std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                k(keys.begin()), k_end(keys.end()) ; k != k_end ; ++k)
        {
            InfoDisplayer i(cmdline, 0, ((*k)->type() == mkt_significant));
            if (cmdline.a_internal_keys.specified() || ((*k)->type() != mkt_internal))
                accept_visitor(i)(**k);
        }
        cout << endl;
    }

    void do_one_package_id(
            const ShowCommandLine & cmdline,
            const std::tr1::shared_ptr<Environment> &,
            const std::tr1::shared_ptr<const PackageID> & best)
    {
        cout << format_general_s(f::show_package_id_heading(), stringify(*best));
        std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(best->begin_metadata(), best->end_metadata());
        for (std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                k(keys.begin()), k_end(keys.end()) ; k != k_end ; ++k)
        {
            InfoDisplayer i(cmdline, 1, ((*k)->type() == mkt_significant));
            if (cmdline.a_internal_keys.specified() || ((*k)->type() != mkt_internal))
                accept_visitor(i)(**k);
        }

        if (best->masked())
        {
            cout << format_general_s(f::show_package_id_masks(), "Masked");
            MaskDisplayer d(cmdline, 2);
            std::for_each(indirect_iterator(best->begin_masks()), indirect_iterator(best->end_masks()), accept_visitor(d));
        }
    }

    void do_one_package(
            const ShowCommandLine & cmdline,
            const std::tr1::shared_ptr<Environment> & env,
            const PackageDepSpec & s)
    {
        cout << format_general_s(f::show_package_heading(), stringify(s));

        const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsGroupedBySlot(
                    generator::Matches(s, MatchPackageOptions()))]);
        if (ids->empty())
            throw NothingMatching(s);

        std::tr1::shared_ptr<const PackageID> best_installable, best_masked_installable;
        std::tr1::shared_ptr<PackageIDSequence> all_installed(new PackageIDSequence);
        std::set<RepositoryName, RepositoryNameComparator> repos;
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            if ((*i)->supports_action(SupportsActionTest<InstalledAction>()))
                all_installed->push_back(*i);
            else if ((*i)->supports_action(SupportsActionTest<InstallAction>()))
            {
                if ((*i)->masked())
                    best_masked_installable = *i;
                else
                    best_installable = *i;
            }

            repos.insert((*i)->repository()->name());
        }

        if (! best_installable)
            best_installable = best_masked_installable;

        for (std::set<RepositoryName, RepositoryNameComparator>::const_iterator r(repos.begin()), r_end(repos.end()) ;
                r != r_end ; ++r)
        {
            cout << format_general_s(f::show_package_repository(), stringify(*r));
            std::string slot_name;
            bool need_space(false);
            for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                    i != i_end ; ++i)
            {
                if ((*i)->repository()->name() != *r)
                    continue;

                if (slot_name != stringify((*i)->slot()))
                {
                    if (! slot_name.empty())
                        cout << format_general_s(f::show_package_slot(), slot_name);
                    slot_name = stringify((*i)->slot());
                }

                if (need_space)
                    cout << " ";
                need_space = true;

                if ((*i)->supports_action(SupportsActionTest<InstalledAction>()))
                    cout << format_general_s(f::show_package_version_installed(), stringify((*i)->canonical_form(idcf_version)));
                else if (! (*i)->masked())
                    cout << format_general_s(f::show_package_version_installable(), stringify((*i)->canonical_form(idcf_version)));
                else
                {
                    std::string rr;
                    for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                            m != m_end ; ++m)
                        rr.append(stringify((*m)->key()));
                    cout << format_general_sr(f::show_package_version_unavailable(), stringify((*i)->canonical_form(idcf_version)), rr);
                }

                if (best_installable && (**i == *best_installable))
                    cout << format_general_s(f::show_package_best(), "");
            }

            cout << format_general_s(f::show_package_slot(), slot_name);
            cout << endl;
        }

        for (PackageIDSequence::ConstIterator i(all_installed->begin()), i_end(all_installed->end()) ;
                i != i_end ; ++i)
            do_one_package_id(cmdline, env, *i);
        if (best_installable)
            do_one_package_id(cmdline, env, best_installable);

        cout << endl;
    }

    void do_all_packages(
            const ShowCommandLine & cmdline,
            const std::tr1::shared_ptr<Environment> & env,
            const PackageDepSpec & s)
    {
        const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(generator::Matches(s,
                        MatchPackageOptions()))]);
        if (ids->empty())
            throw NothingMatching(s);

        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
            do_one_package(cmdline, env, PartiallyMadePackageDepSpec(s).package((*i)->name()));
    }
}

int
ShowCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    ShowCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_SHOW_OPTIONS", "CAVE_SHOW_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() == cmdline.end_parameters())
        throw args::DoHelp("show requires at least one parameter");

    for (ShowCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
            p != p_end ; ++p)
    {
        if (cmdline.a_type.argument() == "set")
            do_one_set(env, SetName(*p));
        else if (cmdline.a_type.argument() == "repository")
            do_one_repository(cmdline, env, RepositoryName(*p));
        else if (cmdline.a_type.argument() == "wildcard")
            do_one_wildcard(env, parse_user_package_dep_spec(
                        *p, env.get(), UserPackageDepSpecOptions() + updso_allow_wildcards));
        else if (cmdline.a_type.argument() == "package")
            do_all_packages(cmdline, env, parse_user_package_dep_spec(
                        *p, env.get(), UserPackageDepSpecOptions() + updso_allow_wildcards));
        else if (cmdline.a_type.argument() == "auto")
        {
            try
            {
                RepositoryName repo_name(*p);
                if (env->package_database()->has_repository_named(repo_name))
                {
                    do_one_repository(cmdline, env, repo_name);
                    continue;
                }
            }
            catch (const RepositoryNameError &)
            {
            }

            try
            {
                PackageDepSpec spec(parse_user_package_dep_spec(*p, env.get(), UserPackageDepSpecOptions() +
                            updso_throw_if_set + updso_allow_wildcards));
                if ((! spec.package_ptr()))
                    do_one_wildcard(env, spec);
                else
                    do_one_package(cmdline, env, spec);
            }
            catch (const GotASetNotAPackageDepSpec &)
            {
                do_one_set(env, SetName(*p));
            }

            continue;
        }
        else
            throw args::DoHelp("bad value '" + cmdline.a_type.argument() + "' for --" + cmdline.a_type.long_name());
    }

    return EXIT_SUCCESS;
}

std::tr1::shared_ptr<args::ArgsHandler>
ShowCommand::make_doc_cmdline()
{
    return make_shared_ptr(new ShowCommandLine);
}

