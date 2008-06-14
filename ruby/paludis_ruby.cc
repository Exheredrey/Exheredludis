/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
 * Copyright (c) 2006, 2007, 2008 Richard Brown
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

#include <paludis/paludis.hh>
#include <paludis_ruby.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/kc.hh>
#include <paludis/dep_list_exceptions.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <ruby.h>
#include <list>
#include <ctype.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

template class InstantiationPolicy<RegisterRubyClass, instantiation_method::SingletonTag>;

namespace paludis
{
    template<>
    struct Implementation<RegisterRubyClass>
    {
        std::list<void (*)()> funcs;
    };
}

namespace
{
    static VALUE c_paludis_module;
    static VALUE c_name_error;
    static VALUE c_set_name_error;
    static VALUE c_category_name_part_error;
    static VALUE c_package_name_part_error;
    static VALUE c_bad_version_spec_error;
    static VALUE c_package_dep_spec_error;
    static VALUE c_package_database_error;
    static VALUE c_package_database_lookup_error;
    static VALUE c_ambiguous_package_name_error;
    static VALUE c_no_such_package_error;
    static VALUE c_no_such_repository_error;
    static VALUE c_configuration_error;
    static VALUE c_config_file_error;
    static VALUE c_dep_list_error;
    static VALUE c_all_masked_error;
    static VALUE c_block_error;
    static VALUE c_circular_dependency_error;
    static VALUE c_additional_requirements_not_met_error;
    static VALUE c_downgrade_not_allowed_error;
    static VALUE c_no_destination_error;
    static VALUE c_fetch_action_error;
    static VALUE c_info_action_error;
    static VALUE c_config_action_error;
    static VALUE c_install_action_error;
    static VALUE c_uninstall_action_error;
    static VALUE c_action_error;
    static VALUE c_bad_version_operator_error;

    /*
     * Document-method: match_package
     *
     * call-seq:
     *     match_package(environment, package_dep_spec, package_id) -> true or false
     *
     * Return whether the specified PackageID matches the specified PackageDepSpec.
     *
     */
    VALUE paludis_match_package(VALUE, VALUE en, VALUE a, VALUE t)
    {
        try
        {
            std::tr1::shared_ptr<Environment> env = value_to_environment(en);
            std::tr1::shared_ptr<const PackageDepSpec> spec = value_to_package_dep_spec(a);
            std::tr1::shared_ptr<const PackageID> target = value_to_package_id(t);
            return match_package(*env, *spec, *target) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

    }

    /*
     * Document-method: match_package_in_set
     *
     * call-seq:
     *     match_package_in_set(environment, set_spec_tree, package_id) -> true or false
     *
     * Return whether the specified PackageID matches the specified set.
     *
     */
    VALUE paludis_match_package_in_set(VALUE, VALUE en, VALUE a, VALUE t)
    {
        try
        {
            std::tr1::shared_ptr<Environment> env = value_to_environment(en);
            std::tr1::shared_ptr<const SetSpecTree::ConstItem> spec = value_to_dep_tree<SetSpecTree>(a);
            std::tr1::shared_ptr<const PackageID> target = value_to_package_id(t);
            return match_package_in_set(*env, *spec, *target) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

    }

    /*
     * Document-method: version_spec_comparator
     *
     * call-seq:
     *     version_spec_comparator(operator, left_version_spec, right_version_spec) -> true of false
     *
     * Applies operator to left_version_spec and right_version_spec
     */
    VALUE paludis_version_spec_comparator(VALUE, VALUE op, VALUE left, VALUE right)
    {
        try
        {
            const VersionOperator vo = VersionOperator(StringValuePtr(op));
            const VersionSpec l = value_to_version_spec(left);
            const VersionSpec r = value_to_version_spec(right);
            if (vo.as_version_spec_comparator()(l, r))
                return true;
            return false;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

}

RegisterRubyClass::RegisterRubyClass() :
    PrivateImplementationPattern<RegisterRubyClass>(new Implementation<RegisterRubyClass>)
{
}

RegisterRubyClass::~RegisterRubyClass()
{
}

void
RegisterRubyClass::execute() const
{
    for (std::list<void (*)()>::const_iterator f(_imp->funcs.begin()), f_end(_imp->funcs.end()) ;
            f != f_end ; ++f)
        (*f)();
}

RegisterRubyClass::Register::Register(void (* f)())
{
    RegisterRubyClass::get_instance()->_imp->funcs.push_back(f);
}

void paludis::ruby::exception_to_ruby_exception(const std::exception & ee)
{
    if (0 != dynamic_cast<const paludis::InternalError *>(&ee))
        rb_raise(rb_eRuntimeError, "Unexpected paludis::InternalError: %s (%s)",
                dynamic_cast<const paludis::InternalError *>(&ee)->message().c_str(), ee.what());
    else if (0 != dynamic_cast<const paludis::BadVersionSpecError *>(&ee))
        rb_raise(c_bad_version_spec_error, dynamic_cast<const paludis::BadVersionSpecError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::SetNameError *>(&ee))
        rb_raise(c_set_name_error, dynamic_cast<const paludis::SetNameError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::PackageNamePartError *>(&ee))
        rb_raise(c_package_name_part_error, dynamic_cast<const paludis::PackageNamePartError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::CategoryNamePartError *>(&ee))
        rb_raise(c_category_name_part_error, dynamic_cast<const paludis::CategoryNamePartError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::NameError *>(&ee))
        rb_raise(c_name_error, dynamic_cast<const paludis::NameError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::PackageDepSpecError *>(&ee))
        rb_raise(c_package_dep_spec_error, dynamic_cast<const paludis::PackageDepSpecError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::NoSuchRepositoryError *>(&ee))
        rb_raise(c_no_such_repository_error, dynamic_cast<const paludis::NoSuchRepositoryError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::AmbiguousPackageNameError *>(&ee))
    {
        VALUE ex_args[2];
        ex_args[0] = rb_str_new2(dynamic_cast<const paludis::AmbiguousPackageNameError *>(&ee)->message().c_str());
        ex_args[1] = rb_ary_new();
        const AmbiguousPackageNameError * e = dynamic_cast<const paludis::AmbiguousPackageNameError *>(&ee);
        for (AmbiguousPackageNameError::OptionsConstIterator f(e->begin_options()), f_end(e->end_options()) ;
                f != f_end ; ++f)
            rb_ary_unshift(ex_args[1], rb_str_new2(stringify(*f).c_str()));
        rb_exc_raise(rb_class_new_instance(2, ex_args, c_ambiguous_package_name_error));
    }
    else if (0 != dynamic_cast<const paludis::NoSuchPackageError *>(&ee))
        rb_raise(c_no_such_package_error, dynamic_cast<const paludis::NoSuchPackageError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::PackageDatabaseLookupError *>(&ee))
        rb_raise(c_package_database_lookup_error, dynamic_cast<const paludis::PackageDatabaseLookupError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::PackageDatabaseError *>(&ee))
        rb_raise(c_package_database_error, dynamic_cast<const paludis::PackageDatabaseError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::AllMaskedError *>(&ee))
    {
        VALUE ex_args[2];
        ex_args[0] = rb_str_new2(dynamic_cast<const paludis::AllMaskedError *>(&ee)->message().c_str());
        ex_args[1] = rb_str_new2(stringify(dynamic_cast<const paludis::AllMaskedError *>(&ee)->query()).c_str());
        rb_exc_raise(rb_class_new_instance(2, ex_args, c_all_masked_error));
    }
    else if (0 != dynamic_cast<const paludis::BlockError *>(&ee))
        rb_raise(c_block_error, dynamic_cast<const paludis::BlockError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::CircularDependencyError *>(&ee))
        rb_raise(c_circular_dependency_error, dynamic_cast<const paludis::CircularDependencyError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::AdditionalRequirementsNotMetError *>(&ee))
        rb_raise(c_additional_requirements_not_met_error, dynamic_cast<const paludis::AdditionalRequirementsNotMetError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::DowngradeNotAllowedError *>(&ee))
        rb_raise(c_downgrade_not_allowed_error, dynamic_cast<const paludis::DowngradeNotAllowedError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::NoDestinationError *>(&ee))
        rb_raise(c_no_destination_error, dynamic_cast<const paludis::NoDestinationError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::DepListError *>(&ee))
        rb_raise(c_dep_list_error, dynamic_cast<const paludis::DepListError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::ConfigFileError *>(&ee))
        rb_raise(c_config_file_error, dynamic_cast<const paludis::ConfigFileError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::ConfigurationError *>(&ee))
        rb_raise(c_configuration_error, dynamic_cast<const paludis::ConfigurationError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::FetchActionError *>(&ee))
    {
        VALUE ex_args[2];
        ex_args[0] = rb_str_new2(dynamic_cast<const paludis::FetchActionError *>(&ee)->message().c_str());
        ex_args[1] = rb_ary_new();
        const FetchActionError * e = dynamic_cast<const paludis::FetchActionError *>(&ee);
        for (Sequence<FetchActionFailure>::ConstIterator f(e->failures()->begin()), f_end(e->failures()->end()) ;
                f != f_end ; ++f)
            rb_ary_unshift(ex_args[1], fetch_action_failure_to_value(*f));
        rb_exc_raise(rb_class_new_instance(2, ex_args, c_fetch_action_error));

    }
    else if (0 != dynamic_cast<const paludis::InfoActionError *>(&ee))
        rb_raise(c_info_action_error, dynamic_cast<const paludis::InfoActionError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::ConfigActionError *>(&ee))
        rb_raise(c_config_action_error, dynamic_cast<const paludis::ConfigActionError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::InstallActionError *>(&ee))
        rb_raise(c_install_action_error, dynamic_cast<const paludis::InstallActionError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::UninstallActionError *>(&ee))
        rb_raise(c_uninstall_action_error, dynamic_cast<const paludis::UninstallActionError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::ActionError *>(&ee))
        rb_raise(c_action_error, dynamic_cast<const paludis::ActionError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::BadVersionOperatorError *>(&ee))
        rb_raise(c_bad_version_operator_error, dynamic_cast<const paludis::BadVersionOperatorError *>(&ee)->message().c_str());

    else if (0 != dynamic_cast<const paludis::Exception *>(&ee))
        rb_raise(rb_eRuntimeError, "Caught paludis::Exception: %s (%s)",
                dynamic_cast<const paludis::Exception *>(&ee)->message().c_str(), ee.what());
    else
        rb_raise(rb_eRuntimeError, "Unexpected std::exception: (%s)", ee.what());
}

std::string
paludis::ruby::value_case_to_RubyCase(const std::string & s)
{
    if (s.empty())
        return s;

    bool upper_next(true);
    std::string result;
    for (std::string::size_type p(0), p_end(s.length()) ; p != p_end ; ++p)
    {
        if ('_' == s[p] || ' ' == s[p])
            upper_next = true;
        else if (upper_next)
        {
            result.append(std::string(1, toupper(s[p])));
            upper_next = false;
        }
        else
            result.append(std::string(1, s[p]));
    }

    return result;
}

VALUE
paludis::ruby::paludis_module()
{
    return c_paludis_module;
}

static VALUE
has_query_property_error_init(int argc, VALUE* argv, VALUE self)
{
    VALUE query;

    query = (argc > 1) ? argv[--argc] : Qnil;
    rb_call_super(argc, argv);
    rb_iv_set(self, "query", query);
    return self;
}

/*
 * call-seq:
 *     query -> String or Nil
 *
 * Our query.
 */
VALUE
has_query_property_error_query(VALUE self)
{
    return rb_attr_get(self, rb_intern("query"));
}

static VALUE
fetch_action_error_init(int argc, VALUE* argv, VALUE self)
{
    VALUE failures;

    failures = (argc > 1) ? argv[--argc] : Qnil;
    rb_call_super(argc, argv);
    rb_iv_set(self, "failures", failures);
    return self;
}

/*
 * call-seq:
 *     failures -> Array
 *
 * Our failures
 */
VALUE
fetch_action_error_failures(VALUE self)
{
    return rb_attr_get(self, rb_intern("failures"));
}

static VALUE
ambiguous_package_name_error_init(int argc, VALUE* argv, VALUE self)
{
    VALUE options;

    options = (argc > 1) ? argv[--argc] : Qnil;
    rb_call_super(argc, argv);
    rb_iv_set(self, "options", options);
    return self;
}

/*
 * call-seq:
 *     options -> Array
 *
 * Our options
 */
VALUE
ambiguous_package_name_error_failures(VALUE self)
{
    return rb_attr_get(self, rb_intern("options"));
}

void PALUDIS_VISIBLE paludis::ruby::init()
{
    /*
     * Defined in create_ruby_doc.rb
     */
    c_paludis_module = rb_define_module("Paludis");
    c_name_error = rb_define_class_under(c_paludis_module, "NameError", rb_eRuntimeError);
    c_set_name_error = rb_define_class_under(c_paludis_module, "SetNameError", c_name_error);
    c_category_name_part_error = rb_define_class_under(c_paludis_module, "CategoryNamePartError", c_name_error);
    c_package_name_part_error = rb_define_class_under(c_paludis_module, "PackageNamePartError", c_name_error);
    c_bad_version_spec_error = rb_define_class_under(c_paludis_module, "BadVersionSpecError", c_name_error);
    c_package_dep_spec_error = rb_define_class_under(c_paludis_module, "PackageDepSpecError", rb_eRuntimeError);
    c_package_database_error = rb_define_class_under(c_paludis_module, "PackageDatabaseError", rb_eRuntimeError);
    c_package_database_lookup_error = rb_define_class_under(c_paludis_module, "PackageDatabaseLookupError", c_package_database_error);
    c_no_such_package_error = rb_define_class_under(c_paludis_module, "NoSuchPackageError", c_package_database_lookup_error);
    c_no_such_repository_error = rb_define_class_under(c_paludis_module, "NoSuchRepositoryError", c_package_database_lookup_error);
    c_configuration_error = rb_define_class_under(c_paludis_module, "ConfigurationError", rb_eRuntimeError);
    c_config_file_error = rb_define_class_under(c_paludis_module, "ConfigFileError", c_configuration_error);
    c_dep_list_error = rb_define_class_under(c_paludis_module, "DepListError", rb_eRuntimeError);

    /*
     * Document-class: Paludis::AllMaskedError
     *
     * Thrown if all versions of a particular spec are masked.
     */
    c_all_masked_error = rb_define_class_under(c_paludis_module, "AllMaskedError", c_dep_list_error);
    rb_define_method(c_all_masked_error, "initialize", RUBY_FUNC_CAST(&has_query_property_error_init), -1);
    rb_define_method(c_all_masked_error, "query", RUBY_FUNC_CAST(&has_query_property_error_query), 0);

    /*
     * Document-class: Paludis::BlockError
     *
     * Thrown if a block is encountered.
     */
    c_block_error = rb_define_class_under(c_paludis_module, "BlockError", c_dep_list_error);

    /*
     * Document-class: Paludis::CircularDependencyError
     *
     * Thrown if a circular dependency is encountered.
     */
    c_circular_dependency_error = rb_define_class_under(c_paludis_module, "CircularDependencyError", c_dep_list_error);

    /*
     * Document-class: Paludis::UseRequirementsNotMetError
     *
     * Thrown if all versions of a particular spec are masked, but would not be if use requirements were not in effect.
     */
    c_additional_requirements_not_met_error = rb_define_class_under(c_paludis_module, "UseRequirementsNotMetError", c_dep_list_error);
    rb_define_method(c_additional_requirements_not_met_error, "initialize", RUBY_FUNC_CAST(&has_query_property_error_init), -1);
    rb_define_method(c_additional_requirements_not_met_error, "query", RUBY_FUNC_CAST(&has_query_property_error_query), 0);
    c_downgrade_not_allowed_error = rb_define_class_under(c_paludis_module, "DowngradeNotAllowedError", c_dep_list_error);
    c_no_destination_error = rb_define_class_under(c_paludis_module, "NoDestinationError", c_dep_list_error);

    /*
     * Document-class: Paludis::ActionError
     *
     * Base class for Action related errors
     */
    c_action_error = rb_define_class_under(c_paludis_module, "ActionError", rb_eRuntimeError);

    /*
     * Document-class: Paludis::FetchActionError
     *
     * Thrown if a PackageID fails to perform a FetchAction.
     */
    c_fetch_action_error = rb_define_class_under(c_paludis_module, "FetchActionError", c_action_error);
    rb_define_method(c_fetch_action_error, "initialize", RUBY_FUNC_CAST(&fetch_action_error_init), -1);
    rb_define_method(c_fetch_action_error, "failures", RUBY_FUNC_CAST(&fetch_action_error_failures), 0);

    /*
     * Document-class: Paludis::AmbiguousPackageNameError
     *
     * Thrown if a PackageDatabase query results in more than one matching Package.
     */
    c_ambiguous_package_name_error = rb_define_class_under(c_paludis_module, "AmbiguousPackageNameError", c_package_database_lookup_error);
    rb_define_method(c_ambiguous_package_name_error, "initialize", RUBY_FUNC_CAST(&ambiguous_package_name_error_init), -1);
    rb_define_method(c_ambiguous_package_name_error, "options", RUBY_FUNC_CAST(&ambiguous_package_name_error_failures), 0);

    /*
     * Document-class: Paludis::InfoActionError
     *
     * Thrown if a PackageID fails to perform a InfoAction.
     */
    c_info_action_error = rb_define_class_under(c_paludis_module, "InfoActionError", c_action_error);

    /*
     * Document-class: Paludis::ConfigActionError
     *
     * Thrown if a PackageID fails to perform a ConfigAction.
     */
    c_config_action_error = rb_define_class_under(c_paludis_module, "ConfigActionError", c_action_error);

    /*
     * Document-class: Paludis::InstallActionError
     *
     * Thrown if a PackageID fails to perform a InstallAction.
     */
    c_install_action_error = rb_define_class_under(c_paludis_module, "InstallActionError", c_action_error);

    /*
     * Document-class: Paludis::UninstallActionError
     *
     * Thrown if a PackageID fails to perform a InstallAction.
     */
    c_uninstall_action_error = rb_define_class_under(c_paludis_module, "UninstallActionError", c_action_error);

    /*
     * Document-class: Paludis::BadVersionOperatorError
     *
     * Thrown if a bad version operator is encountered
     */
    c_bad_version_operator_error = rb_define_class_under(c_paludis_module, "BadVersionOperatorError", rb_eRuntimeError);

    rb_define_module_function(c_paludis_module, "match_package", RUBY_FUNC_CAST(&paludis_match_package), 3);
    rb_define_module_function(c_paludis_module, "match_package_in_set", RUBY_FUNC_CAST(&paludis_match_package_in_set), 3);
    rb_define_module_function(c_paludis_module, "version_spec_comparator", RUBY_FUNC_CAST(&paludis_version_spec_comparator), 3);

    rb_define_const(c_paludis_module, "Version", INT2FIX(PALUDIS_VERSION));
    rb_define_const(c_paludis_module, "VersionMajor", INT2FIX(PALUDIS_VERSION_MAJOR));
    rb_define_const(c_paludis_module, "VersionMinor", INT2FIX(PALUDIS_VERSION_MINOR));
    rb_define_const(c_paludis_module, "VersionMicro", INT2FIX(PALUDIS_VERSION_MICRO));
    rb_define_const(c_paludis_module, "VersionSuffix",
            rb_str_new2(stringify(PALUDIS_VERSION_SUFFIX).c_str()));
    rb_define_const(c_paludis_module, "GitHead",
            rb_str_new2(stringify(PALUDIS_GIT_HEAD).c_str()));
    RegisterRubyClass::get_instance()->execute();
}

#ifdef ENABLE_RUBY_QA
paludis::ruby::RubyQAReporter::RubyQAReporter(VALUE* ruby_reporter) {
    this->reporter = ruby_reporter;
}

void
paludis::ruby::RubyQAReporter::message(const QAMessage & msg)
{
    try
    {
        ID message_id = rb_intern("message");
        VALUE msg_val = qa_message_to_value(msg);
        rb_funcall(*(this->reporter), message_id, 1, msg_val);
    }
    catch (const std::exception & e)
    {
        exception_to_ruby_exception(e);
    }
}

void
paludis::ruby::RubyQAReporter::status(const std::string &)
{
}
#endif

