/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
 * Copyright (c) 2006 Danny van Dyk
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

#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_sets.hh>
#include <paludis/repositories/e/glsa.hh>
#include <paludis/repositories/e/dep_parser.hh>

#include <paludis/environment.hh>
#include <paludis/util/config_file.hh>
#include <paludis/set_file.hh>
#include <paludis/dep_tag.hh>
#include <paludis/package_id.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_requirements.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/dep_spec.hh>
#include <paludis/elike_slot_requirement.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/action-fwd.hh>
#include <tr1/functional>
#include <algorithm>
#include <list>
#include <map>
#include <set>

#include "config.h"

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for ERepositorySets.
     *
     * \ingroup grperepository
     */
    template<>
    struct Implementation<ERepositorySets>
    {
        const Environment * const environment;
        const ERepository * const e_repository;
        const ERepositoryParams params;

        Implementation(const Environment * const e, const ERepository * const p,
                const ERepositoryParams & k) :
            environment(e),
            e_repository(p),
            params(k)
        {
        }
    };
}

ERepositorySets::ERepositorySets(const Environment * const e, const ERepository * const p,
        const ERepositoryParams & k) :
    PrivateImplementationPattern<ERepositorySets>(new Implementation<ERepositorySets>(e, p, k))
{
}

ERepositorySets::~ERepositorySets()
{
}

std::tr1::shared_ptr<SetSpecTree::ConstItem>
ERepositorySets::package_set(const SetName & ss) const
{
    using namespace std::tr1::placeholders;

    if ("system" == ss.data())
        throw InternalError(PALUDIS_HERE, "system set should've been handled by ERepository");
    else if ("security" == ss.data())
        return security_set(false);
    else if ("insecurity" == ss.data())
        return security_set(true);

    std::pair<SetName, SetFileSetOperatorMode> s(find_base_set_name_and_suffix_mode(ss));

    if ((_imp->params.setsdir / (stringify(s.first) + ".conf")).exists())
    {
        std::tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(ss, stringify(_imp->e_repository->name())));

        FSEntry ff(_imp->params.setsdir / (stringify(s.first) + ".conf"));
        Context context("When loading package set '" + stringify(s.first) + "' from '" + stringify(ff) + "':");

        SetFile f(SetFileParams::create()
                .file_name(ff)
                .environment(_imp->environment)
                .type(sft_paludis_conf)
                .parser(std::tr1::bind(&parse_user_package_dep_spec, _1,
                        _imp->environment, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set, filter::All()))
                .set_operator_mode(s.second)
                .tag(tag));

        return f.contents();
    }
    else
        return std::tr1::shared_ptr<SetSpecTree::ConstItem>();
}

std::tr1::shared_ptr<const SetNameSet>
ERepositorySets::sets_list() const
{
    Context context("While generating the list of sets:");

    std::tr1::shared_ptr<SetNameSet> result(new SetNameSet);
    result->insert(SetName("insecurity"));
    result->insert(SetName("security"));
    result->insert(SetName("system"));

    try
    {
        using namespace std::tr1::placeholders;

        std::list<FSEntry> repo_sets;
        std::remove_copy_if(
                DirIterator(_imp->params.setsdir),
                DirIterator(),
                std::back_inserter(repo_sets),
                std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(is_file_with_extension, _1, ".conf", IsFileWithOptions())));

        std::list<FSEntry>::const_iterator f(repo_sets.begin()),
            f_end(repo_sets.end());

        for ( ; f != f_end ; ++f)
            try
            {
                result->insert(SetName(strip_trailing_string(f->basename(), ".conf")));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message("e.sets.failure", ll_warning, lc_context) << "Skipping set '"
                    << *f << "' due to exception '" << e.message() << "' (" << e.what() << ")";
            }
    }
    catch (const paludis::DirOpenError &)
    {
    }

    return result;
}

namespace
{
    bool
    match_range(const PackageID & e, const erepository::GLSARange & r)
    {
        if (r.slot() != "*")
        {
            try
            {
                if (e.slot() != SlotName(r.slot()))
                    return false;
            }
            catch (const SlotNameError &)
            {
                throw GLSAError("Got bad slot '" + r.slot() + "'");
            }
        }

        VersionOperatorValue our_op(static_cast<VersionOperatorValue>(-1));
        std::string ver(r.version());
        if (r.op() == "le")
            our_op = vo_less_equal;
        if (r.op() == "lt")
            our_op = vo_less;
        if (r.op() == "eq")
        {
            if (! ver.empty() && '*' == ver.at(ver.length() - 1))
            {
                ver.erase(ver.length() - 1);
                our_op = vo_equal_star;
            }
            else
                our_op = vo_equal;
        }
        if (r.op() == "gt")
            our_op = vo_greater;
        if (r.op() == "ge")
            our_op = vo_greater_equal;

        if (-1 != our_op)
            return (VersionOperator(our_op).as_version_spec_comparator()(e.version(), VersionSpec(ver)));

        if (0 == r.op().compare(0, 1, "r"))
        {
            return e.version().remove_revision() == VersionSpec(ver).remove_revision() &&
                match_range(e, make_named_values<erepository::GLSARange>(
                            value_for<n::op>(r.op().substr(1)),
                            value_for<n::slot>(r.slot()),
                            value_for<n::version>(r.version())));
        }

        throw GLSAError("Got bad op '" + r.op() + "'");
    }

    bool
    is_vulnerable(const GLSAPackage & glsa_pkg, const PackageID & c)
    {
        /* a package is affected if it matches any vulnerable line, except if it matches
         * any unaffected line. */
        bool vulnerable(false);
        for (GLSAPackage::RangesConstIterator r(glsa_pkg.begin_vulnerable()), r_end(glsa_pkg.end_vulnerable()) ;
                r != r_end && ! vulnerable ; ++r)
            if (match_range(c, *r))
                vulnerable = true;

        if (! vulnerable)
            return false;

        for (GLSAPackage::RangesConstIterator r(glsa_pkg.begin_unaffected()), r_end(glsa_pkg.end_unaffected()) ;
                r != r_end && vulnerable ; ++r)
            if (match_range(c, *r))
                vulnerable = false;

        return vulnerable;
    }
}

std::tr1::shared_ptr<SetSpecTree::ConstItem>
ERepositorySets::security_set(bool insecurity) const
{
    Context context("When building security or insecurity package set:");
    std::tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > security_packages(
            new ConstTreeSequence<SetSpecTree, AllDepSpec>(std::tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));

    if (!_imp->params.securitydir.is_directory_or_symlink_to_directory())
        return security_packages;

    std::map<std::string, std::tr1::shared_ptr<GLSADepTag> > glsa_tags;

    for (DirIterator f(_imp->params.securitydir), f_end ; f != f_end; ++f)
    {
        if (! is_file_with_prefix_extension(*f, "glsa-", ".xml", IsFileWithOptions()))
            continue;

        Context local_context("When parsing security advisory '" + stringify(*f) + "':");

        try
        {
            std::tr1::shared_ptr<const GLSA> glsa(GLSA::create_from_xml_file(stringify(*f)));
            Context local_local_context("When handling GLSA '" + glsa->id() + "' from '" +
                    stringify(*f) + "':");

            for (GLSA::PackagesConstIterator glsa_pkg(glsa->begin_packages()),
                    glsa_pkg_end(glsa->end_packages()) ; glsa_pkg != glsa_pkg_end ; ++glsa_pkg)
            {
                std::tr1::shared_ptr<const PackageIDSequence> candidates;
                if (insecurity)
                    candidates = (*_imp->environment)[selection::AllVersionsSorted(generator::Package(glsa_pkg->name()))];
                else
                    candidates = (*_imp->environment)[selection::AllVersionsSorted(
                            generator::Package(glsa_pkg->name()) |
                            filter::SupportsAction<InstalledAction>())];

                for (PackageIDSequence::ConstIterator c(candidates->begin()), c_end(candidates->end()) ;
                        c != c_end ; ++c)
                {
                    if (! is_vulnerable(*glsa_pkg, **c))
                        continue;

                    if (glsa_tags.end() == glsa_tags.find(glsa->id()))
                        glsa_tags.insert(std::make_pair(glsa->id(), std::tr1::shared_ptr<GLSADepTag>(
                                        new GLSADepTag(glsa->id(), glsa->title(), *f))));

                    if (insecurity)
                    {
                        std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(
                                    make_package_dep_spec()
                                    .package((*c)->name())
                                    .version_requirement(VersionRequirement(vo_equal, (*c)->version()))
                                    .in_repository((*c)->repository()->name())));
                        spec->set_tag(glsa_tags.find(glsa->id())->second);
                        security_packages->add(std::tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(
                                    new TreeLeaf<SetSpecTree, PackageDepSpec>(spec)));
                    }
                    else
                    {
                        Context local_local_local_context("When finding upgrade for '" + stringify(glsa_pkg->name()) + ":"
                                + stringify((*c)->slot()) + "'");

                        /* we need to find the best not vulnerable installable package that isn't masked
                         * that's in the same slot as our vulnerable installed package. */
                        bool ok(false);
                        std::tr1::shared_ptr<const PackageIDSequence> available(
                                (*_imp->environment)[selection::AllVersionsSorted(
                                    generator::Matches(make_package_dep_spec()
                                        .package(glsa_pkg->name())
                                        .slot_requirement(make_shared_ptr(new ELikeSlotExactRequirement((*c)->slot(), false))),
                                        MatchPackageOptions()) |
                                    filter::SupportsAction<InstallAction>() |
                                    filter::NotMasked())]);

                        for (PackageIDSequence::ReverseConstIterator r(available->rbegin()), r_end(available->rend()) ; r != r_end ; ++r)
                        {
                            if (is_vulnerable(*glsa_pkg, **r))
                            {
                                Log::get_instance()->message("e.glsa.skipping_vulnerable", ll_debug, lc_context)
                                    << "Skipping '" << **r << "' due to is_vulnerable match";
                                continue;
                            }

                            std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(make_package_dep_spec()
                                        .package((*r)->name())
                                        .version_requirement(VersionRequirement(vo_equal, (*r)->version()))
                                        .in_repository((*r)->repository()->name())));
                            spec->set_tag(glsa_tags.find(glsa->id())->second);
                            security_packages->add(std::tr1::shared_ptr<SetSpecTree::ConstItem>(
                                        new TreeLeaf<SetSpecTree, PackageDepSpec>(spec)));
                            ok = true;
                            break;
                        }

                        if (! ok)
                            throw GLSAError("Could not determine upgrade path to resolve '"
                                    + glsa->id() + ": " + glsa->title() + "' for package '"
                                    + stringify(**c) + "'");
                    }
                }
            }
        }
        catch (const GLSAError & e)
        {
            Log::get_instance()->message("e.glsa.failure", ll_warning, lc_context)
                << "Cannot use GLSA '" << *f << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        }
        catch (const NameError & e)
        {
            Log::get_instance()->message("e.glsa.failure", ll_warning, lc_context)
                << "Cannot use GLSA '" << *f << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        }
    }

    return security_packages;
}

