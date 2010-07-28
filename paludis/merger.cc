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

#include <paludis/merger.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <istream>
#include <ostream>

using namespace paludis;

MergerError::MergerError(const std::string & m) throw () :
    Exception(m)
{
}

namespace paludis
{
    template <>
    struct Imp<Merger>
    {
        MergerParams params;
        bool result;
        bool skip_dir;

        Imp(const MergerParams & p) :
            params(p),
            result(true),
            skip_dir(false)
        {
        }
    };
}

#include <paludis/merger-se.cc>

Merger::Merger(const MergerParams & p) :
    Pimp<Merger>(p)
{
}

Merger::~Merger() = default;

Hook
Merger::extend_hook(const Hook & h)
{
    return h;
}

bool
Merger::check()
{
    Context context("When checking merge from '" + stringify(_imp->params.image()) + "' to '"
            + stringify(_imp->params.root()) + "':");

    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_check_pre")
                         ("INSTALL_SOURCE", stringify(_imp->params.image()))
                         ("INSTALL_DESTINATION", stringify(_imp->params.root())))).max_exit_status())
        make_check_fail();

    do_dir_recursive(true, _imp->params.image(), _imp->params.root() / _imp->params.install_under());

    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_check_post")
                         ("INSTALL_SOURCE", stringify(_imp->params.image()))
                         ("INSTALL_DESTINATION", stringify(_imp->params.root())))).max_exit_status())
        make_check_fail();

    return _imp->result;
}

void
Merger::make_check_fail()
{
    _imp->result = false;
}

void
Merger::do_dir_recursive(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    Context context("When " + stringify(is_check ? "checking" : "performing") + " merge from '" +
            stringify(src) + "' to '" + stringify(dst) + "':");

    if (! src.is_directory())
        throw MergerError("Source directory '" + stringify(src) + "' is not a directory");
    if ((! is_check) && (! dst.is_directory()))
        throw MergerError("Destination directory '" + stringify(dst) + "' is not a directory");

    on_enter_dir(is_check, src);

    DirIterator d(src, { dio_include_dotfiles, dio_inode_sort }), d_end;

    if (is_check && d == d_end && dst != _imp->params.root().realpath())
    {
        if (_imp->params.options()[mo_allow_empty_dirs])
            Log::get_instance()->message("merger.empty_directory", ll_warning, lc_context) << "Installing empty directory '"
                << stringify(dst) << "'";
        else
            on_error(is_check, "Attempted to install empty directory '" + stringify(dst) + "'");
    }

    for ( ; d != d_end ; ++d)
    {
        EntryType m(entry_type(*d));
        switch (m)
        {
            case et_sym:
                on_sym(is_check, *d, dst);
                continue;

            case et_file:
                on_file(is_check, *d, dst);
                continue;

            case et_dir:
                on_dir(is_check, *d, dst);
                if (_imp->result)
                {
                    if (! _imp->skip_dir)
                        do_dir_recursive(is_check, *d,
                                is_check ? (dst / d->basename()) : (dst / d->basename()).realpath());
                    else
                        _imp->skip_dir = false;
                }
                continue;

            case et_misc:
                on_misc(is_check, *d, dst);
                continue;

            case et_nothing:
            case last_et:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Unexpected entry_type '" + stringify(m) + "'");
    }

    on_leave_dir(is_check, src);
}

void
Merger::on_enter_dir(bool, const FSEntry)
{
}

void
Merger::on_leave_dir(bool, const FSEntry)
{
}

EntryType
Merger::entry_type(const FSEntry & f)
{
    Context context("When checking type of '" + stringify(f) + "':");

    if (! f.exists())
        return et_nothing;

    if (f.is_symbolic_link())
        return et_sym;

    if (f.is_regular_file())
        return et_file;

    if (f.is_directory())
        return et_dir;

    return et_misc;
}

void
Merger::on_file(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    Context context("When handling file '" + stringify(src) + "' to '" + stringify(dst) + "':");

    EntryType m(entry_type(dst / src.basename()));

    if (is_check &&
        0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_check_file_pre")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst / src.basename())))).max_exit_status())
        make_check_fail();

    if (! is_check)
    {
        HookResult hr(_imp->params.environment()->perform_hook(extend_hook(
                        Hook("merger_install_file_override")
                        ("INSTALL_SOURCE", stringify(src))
                        ("INSTALL_DESTINATION", stringify(dst / src.basename()))
                        .grab_output(Hook::AllowedOutputValues()("skip")))));

        if (hr.max_exit_status() != 0)
            Log::get_instance()->message("merger.file.skip_hooks.failure", ll_warning, lc_context) << "Merge of '"
                << stringify(src) << "' to '" << stringify(dst) << "' skip hooks returned non-zero";
        else if (hr.output() == "skip")
        {
            std::string tidy(stringify((dst / src.basename()).strip_leading(_imp->params.root().realpath())));
            display_override("--- [skp] " + tidy);
            return;
        }
    }

    on_file_main(is_check, m, src, dst);

    if (is_check &&
        0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_check_file_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst / src.basename())))).max_exit_status())
        make_check_fail();
}

void
Merger::on_dir(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    Context context("When handling dir '" + stringify(src) + "' to '" + stringify(dst) + "':");

    EntryType m(entry_type(dst / src.basename()));

    if (is_check &&
        0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_check_dir_pre")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst / src.basename())))).max_exit_status())
        make_check_fail();

    if (! is_check)
    {
        HookResult hr(_imp->params.environment()->perform_hook(extend_hook(
                        Hook("merger_install_dir_override")
                        ("INSTALL_SOURCE", stringify(src))
                        ("INSTALL_DESTINATION", stringify(dst / src.basename()))
                        .grab_output(Hook::AllowedOutputValues()("skip")))));

        if (hr.max_exit_status() != 0)
            Log::get_instance()->message("merger.dir.skip_hooks.failure", ll_warning, lc_context) << "Merge of '"
                << stringify(src) << "' to '" << stringify(dst) << "' skip hooks returned non-zero";
        else if (hr.output() == "skip")
        {
            std::string tidy(stringify((dst / src.basename()).strip_leading(_imp->params.root().realpath())));
            display_override("--- [skp] " + tidy);
            _imp->skip_dir = true;
            return;
        }
    }

    on_dir_main(is_check, m, src, dst);

    if (is_check &&
        0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_check_dir_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst / src.basename())))).max_exit_status())
        make_check_fail();
}

void
Merger::on_sym(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    Context context("When handling sym '" + stringify(src) + "' to '" + stringify(dst) + "':");

    EntryType m(entry_type(dst / src.basename()));

    if (is_check &&
        0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_check_sym_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst / src.basename())))).max_exit_status())
        make_check_fail();

    if (! is_check)
    {
        HookResult hr(_imp->params.environment()->perform_hook(extend_hook(
                        Hook("merger_install_sym_override")
                        ("INSTALL_SOURCE", stringify(src))
                        ("INSTALL_DESTINATION", stringify(dst / src.basename()))
                        .grab_output(Hook::AllowedOutputValues()("skip")))));

        if (hr.max_exit_status() != 0)
            Log::get_instance()->message("merger.sym.skip_hooks.failure", ll_warning, lc_context) << "Merge of '"
                << stringify(src) << "' to '" << stringify(dst) << "' skip hooks returned non-zero";
        else if (hr.output() == "skip")
        {
            std::string tidy(stringify((dst / src.basename()).strip_leading(_imp->params.root().realpath())));
            display_override("--- [skp] " + tidy);
            return;
        }
    }
    else
    {
        if (symlink_needs_rewriting(src) && ! _imp->params.options()[mo_rewrite_symlinks])
            on_error(is_check, "Symlink to image detected at: " + stringify(src) + " (" + src.readlink() + ")");
    }

    on_sym_main(is_check, m, src, dst);

    if (is_check &&
        0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_check_sym_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst / src.basename())))).max_exit_status())
        make_check_fail();
}

void
Merger::on_misc(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    Context context("When handling misc '" + stringify(src) + "' to '" + stringify(dst) + "':");

    on_error(is_check, "Cannot write '" + stringify(src) + "' to '" + stringify(dst) +
            "' because it is not a recognised file type");
}

bool
Merger::symlink_needs_rewriting(const FSEntry & sym)
{
    std::string target(sym.readlink());
    std::string real_image(stringify(_imp->params.image().realpath()));

    return (0 == target.compare(0, real_image.length(), real_image));
}

void
Merger::set_skipped_dir(const bool value)
{
    _imp->skip_dir = value;
}

