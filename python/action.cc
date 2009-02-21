/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include <python/paludis_python.hh>
#include <python/exception.hh>

#include <paludis/action.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/sequence.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/repository.hh>
#include <tr1/memory>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

template <typename A_>
class class_supports_action_test :
    public bp::class_<SupportsActionTest<A_>, bp::bases<SupportsActionTestBase> >
{
    public:
        class_supports_action_test(const std::string & action) :
            bp::class_<SupportsActionTest<A_>, bp::bases<SupportsActionTestBase> >(
                    ("Supports" + action + "ActionTest").c_str(),
                    "SupportsActionTest is used by PackageID::supports_action and\n"
                    "Repository::some_ids_might_support_action to query whether a\n"
                    "particular action is supported by that PackageID or potentially\n"
                    "supported by some IDs in that Repository.",
                    bp::init<>("__init__()")
                    )
        {
        }
};

namespace
{
    void dummy_used_this_for_config_protect(const std::string &)
    {
    }

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }

    std::tr1::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return make_shared_ptr(new StandardOutputManager);
    }

    void cannot_perform_uninstall(const std::tr1::shared_ptr<const PackageID> & id)
    {
        throw InternalError(PALUDIS_HERE, "Can't uninstall '" + stringify(*id) + "'");
    }

    InstallActionOptions * make_install_action_options(
            const std::tr1::shared_ptr<paludis::Repository> & r)
    {
        return new InstallActionOptions(make_named_values<InstallActionOptions>(
                    value_for<n::destination>(r),
                    value_for<n::make_output_manager>(&make_standard_output_manager),
                    value_for<n::perform_uninstall>(&cannot_perform_uninstall),
                    value_for<n::replacing>(make_shared_ptr(new PackageIDSequence)),
                    value_for<n::used_this_for_config_protect>(&dummy_used_this_for_config_protect),
                    value_for<n::want_phase>(&want_all_phases)
                    ));
    }

    UninstallActionOptions * make_uninstall_action_options(
            const std::string & c)
    {
        return new UninstallActionOptions(make_named_values<UninstallActionOptions>(
                    value_for<n::config_protect>(c),
                    value_for<n::make_output_manager>(&make_standard_output_manager)
                    ));
    }

    InfoActionOptions * make_info_action_options()
    {
        return new InfoActionOptions(make_named_values<InfoActionOptions>(
                    value_for<n::make_output_manager>(&make_standard_output_manager)
                    ));
    }

    ConfigActionOptions * make_config_action_options()
    {
        return new ConfigActionOptions(make_named_values<ConfigActionOptions>(
                    value_for<n::make_output_manager>(&make_standard_output_manager)
                    ));
    }

    PretendActionOptions * make_pretend_action_options()
    {
        return new PretendActionOptions(make_named_values<PretendActionOptions>(
                    value_for<n::make_output_manager>(&make_standard_output_manager)
                    ));
    }

    FetchActionOptions * make_fetch_action_options(
            const bool exclude_unmirrorable,
            const bool fetch_unneeded,
            const bool safe_resume
            )
    {
        return new FetchActionOptions(make_named_values<FetchActionOptions>(
                    value_for<n::exclude_unmirrorable>(exclude_unmirrorable),
                    value_for<n::fetch_unneeded>(fetch_unneeded),
                    value_for<n::make_output_manager>(&make_standard_output_manager),
                    value_for<n::safe_resume>(safe_resume)
                    ));
    }

}

void expose_action()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<ActionError>
        ("ActionError", "BaseException",
         "Parent class for action errors.");
    ExceptionRegister::get_instance()->add_exception<UnsupportedActionError>
        ("UnsupportedActionError", "ActionError",
         "Thrown if a PackageID is asked to perform an Action that it does not support.");
    ExceptionRegister::get_instance()->add_exception<InstallActionError>
        ("InstallActionError", "ActionError",
         "Thrown if an install fails.");
    ExceptionRegister::get_instance()->add_exception<FetchActionError>
        ("FetchActionError", "ActionError",
         "Thrown if a fetch fails.");
    ExceptionRegister::get_instance()->add_exception<UninstallActionError>
        ("UninstallActionError", "ActionError",
         "Thrown if an uninstall fails.");
    ExceptionRegister::get_instance()->add_exception<ConfigActionError>
        ("ConfigActionError", "ActionError",
         "Thrown if a configure fails.");

    /**
     * InstallActionOptions
     */
    bp::class_<InstallActionOptions>
        (
         "InstallActionOptions",
         "Options for InstallAction.",
         bp::no_init
        )

        .def("__init__",
                bp::make_constructor(&make_install_action_options),
                "__init__(Repository)"
            )

        .add_property("destination",
                &named_values_getter<InstallActionOptions, n::destination, std::tr1::shared_ptr<Repository>, &InstallActionOptions::destination>,
                &named_values_setter<InstallActionOptions, n::destination, std::tr1::shared_ptr<Repository>, &InstallActionOptions::destination>,
                "[rw] Repository"
                )
        ;

    /**
     * UninstallActionOptions
     */
    bp::class_<UninstallActionOptions>
        (
         "UninstallActionOptions",
         "Options for UninstallAction.",
         bp::no_init
        )

        .def("__init__",
                bp::make_constructor(&make_uninstall_action_options),
                "__init__(String)"
            )

        .add_property("config_protect",
                &named_values_getter<UninstallActionOptions, n::config_protect, std::string, &UninstallActionOptions::config_protect>,
                &named_values_setter<UninstallActionOptions, n::config_protect, std::string, &UninstallActionOptions::config_protect>,
                "[rw] String"
                )
        ;

    /**
     * ConfigActionOptions
     */
    bp::class_<ConfigActionOptions>
        (
         "ConfigActionOptions",
         "Options for ConfigAction.",
         bp::no_init
        )

        .def("__init__",
                bp::make_constructor(&make_config_action_options),
                "__init__()"
            )
        ;

    /**
     * InfoActionOptions
     */
    bp::class_<InfoActionOptions>
        (
         "InfoActionOptions",
         "Options for InfoAction.",
         bp::no_init
        )

        .def("__init__",
                bp::make_constructor(&make_info_action_options),
                "__init__()"
            )
        ;

    /**
     * PretendActionOptions
     */
    bp::class_<PretendActionOptions>
        (
         "PretendActionOptions",
         "Options for PretendAction.",
         bp::no_init
        )

        .def("__init__",
                bp::make_constructor(&make_pretend_action_options),
                "__init__()"
            )
        ;

    /**
     * FetchActionOptions
     */
    bp::class_<FetchActionOptions>
        (
         "FetchActionOptions",
         "Options for FetchAction.",
         bp::no_init
        )

        .def("__init__",
                bp::make_constructor(&make_fetch_action_options),
                "__init__(exclude_unmirrorable, fetch_uneeded, safe_resume)"
            )

        .add_property("exclude_unmirrorable",
                &named_values_getter<FetchActionOptions, n::exclude_unmirrorable, bool, &FetchActionOptions::exclude_unmirrorable>,
                &named_values_setter<FetchActionOptions, n::exclude_unmirrorable, bool, &FetchActionOptions::exclude_unmirrorable>,
                "[rw] bool"
                )

        .add_property("fetch_unneeded",
                &named_values_getter<FetchActionOptions, n::fetch_unneeded, bool, &FetchActionOptions::fetch_unneeded>,
                &named_values_setter<FetchActionOptions, n::fetch_unneeded, bool, &FetchActionOptions::fetch_unneeded>,
                "[rw] bool"
                )

        .add_property("safe_resume",
                &named_values_getter<FetchActionOptions, n::safe_resume, bool, &FetchActionOptions::safe_resume>,
                &named_values_setter<FetchActionOptions, n::safe_resume, bool, &FetchActionOptions::safe_resume>,
                "[rw] bool"
                )
        ;

   /**
     * Action
     */
    bp::class_<Action, boost::noncopyable>
        (
         "Action",
         "An Action represents an action that can be executed by a PackageID via\n"
         "PackageID::perform_action.",
         bp::no_init
        );

    /**
     * InstallAction
     */
    bp::class_<InstallAction, bp::bases<Action>, boost::noncopyable>
        (
         "InstallAction",
         "An InstallAction is used by InstallTask to perform a build / install on a\n"
         "PackageID.",
         bp::init<const InstallActionOptions &>("__init__(InstallActionOptions)")
        );

    /**
     * FetchAction
     */
    bp::class_<FetchAction, bp::bases<Action>, boost::noncopyable>
        (
         "FetchAction",
         "A FetchAction can be used to fetch source files for a PackageID using\n"
         "PackageID::perform_action.",
         bp::init<const FetchActionOptions &>("__init__(FetchActionOptions)")
        );

    /**
     * UninstallAction
     */
    bp::class_<UninstallAction, bp::bases<Action>, boost::noncopyable>
        (
         "UninstallAction",
         "An UninstallAction is used by UninstallTask to uninstall a PackageID.",
         bp::init<const UninstallActionOptions &>("__init__(UninstallActionOptions)")
        );

    /**
     * InsatlledAction
     */
    bp::class_<InstalledAction, bp::bases<Action>, boost::noncopyable>
        (
         "InstalledAction",
         "InstalledAction is a dummy action used by SupportsActionTest and\n"
         "query::SupportsAction to determine whether a PackageID is installed.",
         bp::init<>("__init__()")
        );

    /**
     * PretendAction
     */
    bp::class_<PretendAction, bp::bases<Action>, boost::noncopyable>
        (
         "PretendAction",
         "A PretendAction is used by InstallTask to handle install-pretend-phase\n"
         "checks on a PackageID.",
         bp::init<const PretendActionOptions &>("__init__(PretendActionOptions)")
        )
        .add_property("failed", &PretendAction::failed,
                "[ro] bool\n"
                "Did our pretend phase fail?"
                )
        ;

    /**
     * ConfigAction
     */
    bp::class_<ConfigAction, bp::bases<Action>, boost::noncopyable>
        (
         "ConfigAction",
         "A ConfigAction is used via PackageID::perform_action to execute\n"
         "post-install configuration (for example, via 'paludis --config')\n"
         "on a PackageID.",
         bp::init<ConfigActionOptions>("__init__(ConfigActionOptions)")
        );

    /**
     * InfoAction
     */
    bp::class_<InfoAction, bp::bases<Action>, boost::noncopyable>
        (
         "InfoAction",
         "An InfoAction is used via PackageID::perform_action to execute\n"
         "additional information (for example, via 'paludis --info')\n"
         "on a PackageID.",
         bp::init<InfoActionOptions>("__init__(InfoActionOptions)")
        );

    /**
     * SupportActionTestBase
     */
    bp::class_<SupportsActionTestBase, boost::noncopyable>
        (
         "SupportsActionTestBase",
         "Base class for SupportsActionTests.",
         bp::no_init
        );

    /**
     * SupportActionTests
     */
    class_supports_action_test<InstallAction>("Install");
    class_supports_action_test<FetchAction>("Fetch");
    class_supports_action_test<UninstallAction>("Uninstall");
    class_supports_action_test<InstalledAction>("Installed");
    class_supports_action_test<PretendAction>("Pretend");
    class_supports_action_test<ConfigAction>("Config");
    class_supports_action_test<InfoAction>("Info");
}

