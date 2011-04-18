/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_dep_spec.cc "example_dep_spec.cc" .
 *
 * \ingroup g_dep_spec
 */

/** \example example_dep_spec.cc
 *
 * This example demonstrates how to handle dependency specs.
 *
 * See \ref example_dep_label.cc "example_dep_label.cc" for labels.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <list>
#include <map>
#include <sstream>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::setw;
using std::left;

int main(int argc, char * argv[])
{
    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_dep_spec", "EXAMPLE_DEP_SPEC_OPTIONS", "EXAMPLE_DEP_SPEC_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* For each command line parameter... */
        for (CommandLine::ParametersConstIterator q(CommandLine::get_instance()->begin_parameters()),
                q_end(CommandLine::get_instance()->end_parameters()) ; q != q_end ; ++q)
        {
            /* Create a PackageDepSpec from the parameter. For user-inputted
             * data, parse_user_package_dep_spec() should be used. If wildcards
             * are to be permitted, the updso_allow_wildcards option should be
             * included. If you might be getting a set, also include
             * updso_throw_if_set and catch the GotASetNotAPackageDepSpec
             * exception. If data about the spec is known at compile time,
             * make_package_dep_spec() should be used instead. */
            PackageDepSpec spec(parse_user_package_dep_spec(*q, env.get(), { updso_allow_wildcards }));

            /* Display information about the PackageDepSpec. */
            cout << "Information about '" << spec << "':" << endl;

            if (spec.package_name_requirement())
                cout << "    " << left << setw(24) << "Package:" << " " << spec.package_name_requirement()->name() << endl;

            if (spec.category_name_part_requirement())
                cout << "    " << left << setw(24) << "Category part:" << " " << spec.category_name_part_requirement()->name_part() << endl;

            if (spec.package_name_part_requirement())
                cout << "    " << left << setw(24) << "Package part:" << " " << spec.package_name_part_requirement()->name_part() << endl;

            if (spec.exact_slot_requirement())
                cout << "    " << left << setw(24) << "Slot:" << " " << spec.exact_slot_requirement()->name() << endl;

            if (spec.in_repository_requirement())
                cout << "    " << left << setw(24) << "In repository:" << " " <<
                    spec.in_repository_requirement()->name() << endl;

            if (spec.from_repository_requirement())
                cout << "    " << left << setw(24) << "From repository:" << " " <<
                    spec.from_repository_requirement()->name() << endl;

            if (spec.installed_at_path_requirement())
                cout << "    " << left << setw(24) << "Installed at path:" << " " <<
                    spec.installed_at_path_requirement()->path() << endl;

            if (spec.installable_to_path_requirement())
                cout << "    " << left << setw(24) << "Installable to path:" << " " <<
                    spec.installable_to_path_requirement()->path() << ", " <<
                    spec.installable_to_path_requirement()->include_masked() << endl;

            if (spec.installable_to_repository_requirement())
                cout << "    " << left << setw(24) << "Installable to repository:" << " " <<
                    spec.installable_to_repository_requirement()->name() << ", " <<
                    spec.installable_to_repository_requirement()->include_masked() << endl;

            /* And display packages matching that spec */
            cout << "    " << left << setw(24) << "Matches:" << " ";
            std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                        generator::Matches(spec, make_null_shared_ptr(), { }))]);
            bool need_indent(false);
            for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                    i != i_end ; ++i)
            {
                if (need_indent)
                    cout << "    " << left << setw(24) << "" << " ";
                cout << **i << endl;
                need_indent = true;
            }

            cout << endl;
        }
    }
    catch (const Exception & e)
    {
        /* Paludis exceptions can provide a handy human-readable backtrace and
         * an explanation message. Where possible, these should be displayed. */
        cout << endl;
        cout << "Unhandled exception:" << endl
            << "  * " << e.backtrace("\n  * ")
            << e.message() << " (" << e.what() << ")" << endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception & e)
    {
        cout << endl;
        cout << "Unhandled exception:" << endl
            << "  * " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cout << endl;
        cout << "Unhandled exception:" << endl
            << "  * Unknown exception type. Ouch..." << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

