/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/elike_choices.hh>
#include <paludis/environment.hh>
#include <paludis/permitted_choice_value_parameter_values.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/util/map.hh>
#include <limits>
#include <istream>
#include <ostream>

using namespace paludis;

#include <paludis/elike_choices-se.cc>

namespace
{
    struct CommonValues :
        Singleton<CommonValues>
    {
        const std::shared_ptr<const PermittedChoiceValueParameterIntegerValue> permitted_jobs_values;

        const std::shared_ptr<Map<std::string, std::string> > permitted_symbols;
        const std::shared_ptr<const PermittedChoiceValueParameterEnumValue> permitted_symbols_values;

        CommonValues() :
            permitted_jobs_values(std::make_shared<PermittedChoiceValueParameterIntegerValue>(1, std::numeric_limits<int>::max())),
            permitted_symbols(std::make_shared<Map<std::string, std::string> >()),
            permitted_symbols_values(std::make_shared<PermittedChoiceValueParameterEnumValue>(permitted_symbols))
        {
            for (EnumIterator<ELikeSymbolsChoiceValueParameter> e, e_end(last_escvp) ;
                    e != e_end ; ++e)
            {
                switch (*e)
                {
                    case escvp_split:
                        permitted_symbols->insert("split", "Split debug symbols");
                        continue;
                    case escvp_preserve:
                        permitted_symbols->insert("preserve", "Preserve debug symbols");
                        continue;
                    case escvp_strip:
                        permitted_symbols->insert("strip", "Strip debug symbols");
                        continue;
                    case escvp_compress:
                        permitted_symbols->insert("compress", "Split and compress debug symbols");
                        continue;
                    case last_escvp:
                        break;
                }

                throw InternalError(PALUDIS_HERE, "Unhandled ELikeSymbolsChoiceValueParameter");
            }
        }
    };
}

const UnprefixedChoiceName
ELikeStripChoiceValue::canonical_unprefixed_name()
{
    return UnprefixedChoiceName("strip");
}

const ChoiceNameWithPrefix
ELikeStripChoiceValue::canonical_name_with_prefix()
{
    return ChoiceNameWithPrefix(stringify(canonical_build_options_prefix()) + ":" + stringify(canonical_unprefixed_name()));
}

ELikeStripChoiceValue::ELikeStripChoiceValue(const std::shared_ptr<const PackageID> & id,
        const Environment * const env, const std::shared_ptr<const Choice> & choice, const Tribool forced_value) :
    _enabled(forced_value.is_indeterminate() ? ! env->want_choice_enabled(id, choice, canonical_unprefixed_name()).is_false() :
            forced_value.is_true() ? true : false),
    _forced(! forced_value.is_indeterminate())
{
}

const UnprefixedChoiceName
ELikeStripChoiceValue::unprefixed_name() const
{
    return canonical_unprefixed_name();
}

const ChoiceNameWithPrefix
ELikeStripChoiceValue::name_with_prefix() const
{
    return canonical_name_with_prefix();
}

bool
ELikeStripChoiceValue::enabled() const
{
    return _enabled;
}

bool
ELikeStripChoiceValue::enabled_by_default() const
{
    return true;
}

bool
ELikeStripChoiceValue::locked() const
{
    return false;
}

const std::string
ELikeStripChoiceValue::description() const
{
    return "Strip binaries and libraries before installation";
}

bool
ELikeStripChoiceValue::explicitly_listed() const
{
    return true;
}

const std::string
ELikeStripChoiceValue::parameter() const
{
    return "";
}

const std::shared_ptr<const PermittedChoiceValueParameterValues>
ELikeStripChoiceValue::permitted_parameter_values() const
{
    return make_null_shared_ptr();
}

const UnprefixedChoiceName
ELikeSplitChoiceValue::canonical_unprefixed_name()
{
    return UnprefixedChoiceName("split");
}

const ChoiceNameWithPrefix
ELikeSplitChoiceValue::canonical_name_with_prefix()
{
    return ChoiceNameWithPrefix(stringify(canonical_build_options_prefix()) + ":" + stringify(canonical_unprefixed_name()));
}

ELikeSplitChoiceValue::ELikeSplitChoiceValue(const std::shared_ptr<const PackageID> & id,
        const Environment * const env, const std::shared_ptr<const Choice> & choice,
        const Tribool forced_value) :
    _enabled(forced_value.is_indeterminate() ? ! env->want_choice_enabled(id, choice, canonical_unprefixed_name()).is_false() :
            forced_value.is_true() ? true : false),
    _forced(! forced_value.is_indeterminate())
{
}

const UnprefixedChoiceName
ELikeSplitChoiceValue::unprefixed_name() const
{
    return canonical_unprefixed_name();
}

const ChoiceNameWithPrefix
ELikeSplitChoiceValue::name_with_prefix() const
{
    return canonical_name_with_prefix();
}

bool
ELikeSplitChoiceValue::enabled() const
{
    return _enabled;
}

bool
ELikeSplitChoiceValue::enabled_by_default() const
{
    return true;
}

bool
ELikeSplitChoiceValue::locked() const
{
    return false;
}

const std::string
ELikeSplitChoiceValue::description() const
{
    return "Split debugging information out from binaries and libraries before installation";
}

bool
ELikeSplitChoiceValue::explicitly_listed() const
{
    return true;
}

const std::string
ELikeSplitChoiceValue::parameter() const
{
    return "";
}

const std::shared_ptr<const PermittedChoiceValueParameterValues>
ELikeSplitChoiceValue::permitted_parameter_values() const
{
    return make_null_shared_ptr();
}

const UnprefixedChoiceName
ELikeOptionalTestsChoiceValue::canonical_unprefixed_name()
{
    return UnprefixedChoiceName("optional_tests");
}

const ChoiceNameWithPrefix
ELikeOptionalTestsChoiceValue::canonical_name_with_prefix()
{
    return ChoiceNameWithPrefix(stringify(canonical_build_options_prefix()) + ":" + stringify(canonical_unprefixed_name()));
}

ELikeOptionalTestsChoiceValue::ELikeOptionalTestsChoiceValue(const std::shared_ptr<const PackageID> & id,
        const Environment * const env, const std::shared_ptr<const Choice> & choice,
        const bool l) :
    _enabled((! l) && (env->want_choice_enabled(id, choice, canonical_unprefixed_name()).is_true())),
    _mask(l)
{
}

const UnprefixedChoiceName
ELikeOptionalTestsChoiceValue::unprefixed_name() const
{
    return canonical_unprefixed_name();
}

const ChoiceNameWithPrefix
ELikeOptionalTestsChoiceValue::name_with_prefix() const
{
    return canonical_name_with_prefix();
}

bool
ELikeOptionalTestsChoiceValue::enabled() const
{
    return _enabled;
}

bool
ELikeOptionalTestsChoiceValue::enabled_by_default() const
{
    return false;
}

bool
ELikeOptionalTestsChoiceValue::locked() const
{
    return _mask;
}

const std::string
ELikeOptionalTestsChoiceValue::description() const
{
    return "Run tests considered by the package to be optional";
}

bool
ELikeOptionalTestsChoiceValue::explicitly_listed() const
{
    return true;
}

const std::string
ELikeOptionalTestsChoiceValue::parameter() const
{
    return "";
}

const std::shared_ptr<const PermittedChoiceValueParameterValues>
ELikeOptionalTestsChoiceValue::permitted_parameter_values() const
{
    return make_null_shared_ptr();
}

const UnprefixedChoiceName
ELikeRecommendedTestsChoiceValue::canonical_unprefixed_name()
{
    return UnprefixedChoiceName("recommended_tests");
}

const ChoiceNameWithPrefix
ELikeRecommendedTestsChoiceValue::canonical_name_with_prefix()
{
    return ChoiceNameWithPrefix(stringify(canonical_build_options_prefix()) + ":" + stringify(canonical_unprefixed_name()));
}

ELikeRecommendedTestsChoiceValue::ELikeRecommendedTestsChoiceValue(const std::shared_ptr<const PackageID> & id,
        const Environment * const env, const std::shared_ptr<const Choice> & choice,
        const bool l) :
    _enabled((! l) && (! env->want_choice_enabled(id, choice, canonical_unprefixed_name()).is_false())),
    _mask(l)
{
}

const UnprefixedChoiceName
ELikeRecommendedTestsChoiceValue::unprefixed_name() const
{
    return canonical_unprefixed_name();
}

const ChoiceNameWithPrefix
ELikeRecommendedTestsChoiceValue::name_with_prefix() const
{
    return canonical_name_with_prefix();
}

bool
ELikeRecommendedTestsChoiceValue::enabled() const
{
    return _enabled;
}

bool
ELikeRecommendedTestsChoiceValue::enabled_by_default() const
{
    return true;
}

bool
ELikeRecommendedTestsChoiceValue::locked() const
{
    return _mask;
}

const std::string
ELikeRecommendedTestsChoiceValue::description() const
{
    return "Run tests considered by the package to be recommended";
}

bool
ELikeRecommendedTestsChoiceValue::explicitly_listed() const
{
    return true;
}

const std::string
ELikeRecommendedTestsChoiceValue::parameter() const
{
    return "";
}

const std::shared_ptr<const PermittedChoiceValueParameterValues>
ELikeRecommendedTestsChoiceValue::permitted_parameter_values() const
{
    return make_null_shared_ptr();
}

const ChoicePrefixName
paludis::canonical_build_options_prefix()
{
    return ChoicePrefixName("build_options");
}

const std::string
paludis::canonical_build_options_raw_name()
{
    return "build_options";
}

const std::string
paludis::canonical_build_options_human_name()
{
    return "Build Options";
}

const UnprefixedChoiceName
ELikeExpensiveTestsChoiceValue::canonical_unprefixed_name()
{
    return UnprefixedChoiceName("expensive_tests");
}

const ChoiceNameWithPrefix
ELikeExpensiveTestsChoiceValue::canonical_name_with_prefix()
{
    return ChoiceNameWithPrefix(stringify(canonical_build_options_prefix()) + ":" + stringify(canonical_unprefixed_name()));
}

ELikeExpensiveTestsChoiceValue::ELikeExpensiveTestsChoiceValue(const std::shared_ptr<const PackageID> & id,
        const Environment * const env, const std::shared_ptr<const Choice> & choice,
        const bool l) :
    _enabled((! l) && (env->want_choice_enabled(id, choice, canonical_unprefixed_name()).is_true())),
    _mask(l)
{
}

const UnprefixedChoiceName
ELikeExpensiveTestsChoiceValue::unprefixed_name() const
{
    return canonical_unprefixed_name();
}

const ChoiceNameWithPrefix
ELikeExpensiveTestsChoiceValue::name_with_prefix() const
{
    return canonical_name_with_prefix();
}

bool
ELikeExpensiveTestsChoiceValue::enabled() const
{
    return _enabled;
}

bool
ELikeExpensiveTestsChoiceValue::enabled_by_default() const
{
    return false;
}

bool
ELikeExpensiveTestsChoiceValue::locked() const
{
    return _mask;
}

const std::string
ELikeExpensiveTestsChoiceValue::description() const
{
    return "Run tests considered by the package to be useful, but expensive";
}

bool
ELikeExpensiveTestsChoiceValue::explicitly_listed() const
{
    return true;
}

const std::string
ELikeExpensiveTestsChoiceValue::parameter() const
{
    return "";
}

const std::shared_ptr<const PermittedChoiceValueParameterValues>
ELikeExpensiveTestsChoiceValue::permitted_parameter_values() const
{
    return make_null_shared_ptr();
}

const UnprefixedChoiceName
ELikeJobsChoiceValue::canonical_unprefixed_name()
{
    return UnprefixedChoiceName("jobs");
}

const ChoiceNameWithPrefix
ELikeJobsChoiceValue::canonical_name_with_prefix()
{
    return ChoiceNameWithPrefix(stringify(canonical_build_options_prefix()) + ":" + stringify(canonical_unprefixed_name()));
}

namespace
{
    std::string get_jobs(const std::shared_ptr<const PackageID> & id,
            const std::string & env_value)
    {
        if (env_value.empty())
            return "1";

        try
        {
            return stringify(destringify<unsigned>(env_value));
        }
        catch (const DestringifyError &)
        {
            Context context("When getting value of the jobs option for '" + stringify(*id) + "':");
            Log::get_instance()->message("elike_jobs_choice_value.invalid", ll_warning, lc_context)
                << "Value '" << env_value << "' is not an unsigned integer, using \"1\" instead";
            return "1";
        }
    }
}

ELikeJobsChoiceValue::ELikeJobsChoiceValue(const std::shared_ptr<const PackageID> & id,
        const Environment * const env, const std::shared_ptr<const Choice> & choice) :
    _enabled(env->want_choice_enabled(id, choice, canonical_unprefixed_name()).is_true()),
    _parameter(get_jobs(id, env->value_for_choice_parameter(id, choice, canonical_unprefixed_name())))
{
}

const UnprefixedChoiceName
ELikeJobsChoiceValue::unprefixed_name() const
{
    return canonical_unprefixed_name();
}

const ChoiceNameWithPrefix
ELikeJobsChoiceValue::name_with_prefix() const
{
    return canonical_name_with_prefix();
}

bool
ELikeJobsChoiceValue::enabled() const
{
    return _enabled;
}

bool
ELikeJobsChoiceValue::enabled_by_default() const
{
    return false;
}

bool
ELikeJobsChoiceValue::locked() const
{
    return false;
}

const std::string
ELikeJobsChoiceValue::description() const
{
    return "How many jobs the package's build system should use, where supported";
}

bool
ELikeJobsChoiceValue::explicitly_listed() const
{
    return true;
}

const std::string
ELikeJobsChoiceValue::parameter() const
{
    return _parameter;
}

const std::shared_ptr<const PermittedChoiceValueParameterValues>
ELikeJobsChoiceValue::permitted_parameter_values() const
{
    return CommonValues::get_instance()->permitted_jobs_values;
}

const UnprefixedChoiceName
ELikeTraceChoiceValue::canonical_unprefixed_name()
{
    return UnprefixedChoiceName("trace");
}

const ChoiceNameWithPrefix
ELikeTraceChoiceValue::canonical_name_with_prefix()
{
    return ChoiceNameWithPrefix(stringify(canonical_build_options_prefix()) + ":" +
            stringify(canonical_unprefixed_name()));
}

ELikeTraceChoiceValue::ELikeTraceChoiceValue(const std::shared_ptr<const PackageID> & id,
        const Environment * const env, const std::shared_ptr<const Choice> & choice) :
    _enabled(env->want_choice_enabled(id, choice, canonical_unprefixed_name()).is_true())
{
}

const UnprefixedChoiceName
ELikeTraceChoiceValue::unprefixed_name() const
{
    return canonical_unprefixed_name();
}

const ChoiceNameWithPrefix
ELikeTraceChoiceValue::name_with_prefix() const
{
    return canonical_name_with_prefix();
}

bool
ELikeTraceChoiceValue::enabled() const
{
    return _enabled;
}

bool
ELikeTraceChoiceValue::enabled_by_default() const
{
    return false;
}

bool
ELikeTraceChoiceValue::locked() const
{
    return false;
}

const std::string
ELikeTraceChoiceValue::description() const
{
    return "Trace actions executed by the package (very noisy, for debugging broken builds only)";
}

bool
ELikeTraceChoiceValue::explicitly_listed() const
{
    return true;
}

const std::string
ELikeTraceChoiceValue::parameter() const
{
    return "";
}

const std::shared_ptr<const PermittedChoiceValueParameterValues>
ELikeTraceChoiceValue::permitted_parameter_values() const
{
    return make_null_shared_ptr();
}

const UnprefixedChoiceName
ELikePreserveWorkChoiceValue::canonical_unprefixed_name()
{
    return UnprefixedChoiceName("preserve_work");
}

const ChoiceNameWithPrefix
ELikePreserveWorkChoiceValue::canonical_name_with_prefix()
{
    return ChoiceNameWithPrefix(stringify(canonical_build_options_prefix()) + ":" +
            stringify(canonical_unprefixed_name()));
}

ELikePreserveWorkChoiceValue::ELikePreserveWorkChoiceValue(const std::shared_ptr<const PackageID> & id,
        const Environment * const env, const std::shared_ptr<const Choice> & choice,
        const Tribool forced_value) :
    _enabled(forced_value.is_indeterminate() ? env->want_choice_enabled(id, choice, canonical_unprefixed_name()).is_true() :
            forced_value.is_true() ? true : false),
    _forced(! forced_value.is_indeterminate())
{
}

const UnprefixedChoiceName
ELikePreserveWorkChoiceValue::unprefixed_name() const
{
    return canonical_unprefixed_name();
}

const ChoiceNameWithPrefix
ELikePreserveWorkChoiceValue::name_with_prefix() const
{
    return canonical_name_with_prefix();
}

bool
ELikePreserveWorkChoiceValue::enabled() const
{
    return _enabled;
}

bool
ELikePreserveWorkChoiceValue::enabled_by_default() const
{
    return false;
}

bool
ELikePreserveWorkChoiceValue::locked() const
{
    return false;
}

const std::string
ELikePreserveWorkChoiceValue::description() const
{
    return "Do not remove build directories, and do not modify the image when merging";
}

bool
ELikePreserveWorkChoiceValue::explicitly_listed() const
{
    return true;
}

const std::string
ELikePreserveWorkChoiceValue::parameter() const
{
    return "";
}

const std::shared_ptr<const PermittedChoiceValueParameterValues>
ELikePreserveWorkChoiceValue::permitted_parameter_values() const
{
    return make_null_shared_ptr();
}

namespace
{
    ELikeSymbolsChoiceValueParameter get_symbols(const std::shared_ptr<const PackageID> & id,
            const std::string & env_value)
    {
        if (env_value.empty())
            return escvp_split;

        try
        {
            return destringify<ELikeSymbolsChoiceValueParameter>(env_value);
        }
        catch (const DestringifyError &)
        {
            Context context("When getting value of the symbols option for '" + stringify(*id) + "':");
            Log::get_instance()->message("elike_symbols_choice_value.invalid", ll_warning, lc_context)
                << "Value '" << env_value << "' is not a legal value, using \"split\" instead";
            return escvp_split;
        }
    }
}

ELikeSymbolsChoiceValue::ELikeSymbolsChoiceValue(const std::shared_ptr<const PackageID> & id,
        const Environment * const env, const std::shared_ptr<const Choice> & choice, const ELikeSymbolsChoiceValueParameter _force) :
    _enabled(! env->want_choice_enabled(id, choice, canonical_unprefixed_name()).is_false()),
    _param(_force != last_escvp ? _force : get_symbols(id, env->value_for_choice_parameter(id, choice, canonical_unprefixed_name())))
{
}

const UnprefixedChoiceName
ELikeSymbolsChoiceValue::unprefixed_name() const
{
    return canonical_unprefixed_name();
}

const ChoiceNameWithPrefix
ELikeSymbolsChoiceValue::name_with_prefix() const
{
    return canonical_name_with_prefix();
}

bool
ELikeSymbolsChoiceValue::enabled() const
{
    return _enabled;
}

bool
ELikeSymbolsChoiceValue::enabled_by_default() const
{
    return true;
}

bool
ELikeSymbolsChoiceValue::locked() const
{
    return false;
}

const std::string
ELikeSymbolsChoiceValue::description() const
{
    return "How to handle debug symbols in installed files";
}

bool
ELikeSymbolsChoiceValue::explicitly_listed() const
{
    return true;
}

const std::string
ELikeSymbolsChoiceValue::parameter() const
{
    return stringify(_param);
}

const std::shared_ptr<const PermittedChoiceValueParameterValues>
ELikeSymbolsChoiceValue::permitted_parameter_values() const
{
    return CommonValues::get_instance()->permitted_symbols_values;
}

const UnprefixedChoiceName
ELikeSymbolsChoiceValue::canonical_unprefixed_name()
{
    return UnprefixedChoiceName("symbols");
}

const ChoiceNameWithPrefix
ELikeSymbolsChoiceValue::canonical_name_with_prefix()
{
    return ChoiceNameWithPrefix(stringify(canonical_build_options_prefix()) + ":" + stringify(canonical_unprefixed_name()));
}

bool
ELikeSymbolsChoiceValue::should_split(const std::string & v)
{
    switch (destringify<ELikeSymbolsChoiceValueParameter>(v))
    {
        case escvp_split:
        case escvp_compress:
            return true;

        case escvp_preserve:
        case escvp_strip:
            return false;

        case last_escvp:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Unhandled ELikeSymbolsChoiceValueParameter");
}

bool
ELikeSymbolsChoiceValue::should_strip(const std::string & v)
{
    switch (destringify<ELikeSymbolsChoiceValueParameter>(v))
    {
        case escvp_split:
        case escvp_compress:
        case escvp_strip:
            return true;

        case escvp_preserve:
            return false;

        case last_escvp:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Unhandled ELikeSymbolsChoiceValueParameter");
}

bool
ELikeSymbolsChoiceValue::should_compress(const std::string & v)
{
    switch (destringify<ELikeSymbolsChoiceValueParameter>(v))
    {
        case escvp_compress:
            return true;

        case escvp_split:
        case escvp_preserve:
        case escvp_strip:
            return false;

        case last_escvp:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Unhandled ELikeSymbolsChoiceValueParameter");
}
