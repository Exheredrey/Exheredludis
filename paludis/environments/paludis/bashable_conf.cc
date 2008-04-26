/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include "bashable_conf.hh"
#include <paludis/util/config_file.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>

using namespace paludis;
using namespace paludis::paludis_environment;

std::tr1::shared_ptr<LineConfigFile>
paludis::paludis_environment::make_bashable_conf(const FSEntry & f)
{
    Context context("When making a config file out of '" + stringify(f) + "':");

    std::tr1::shared_ptr<LineConfigFile> result;

    if (is_file_with_extension(f, ".bash", IsFileWithOptions()))
    {
        std::stringstream s;
        Command cmd(Command("bash '" + stringify(f) + "'")
                .with_setenv("PALUDIS_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                .with_stderr_prefix(f.basename() + "> ")
                .with_captured_stdout_stream(&s));
        int exit_status(run_command(cmd));
        result.reset(new LineConfigFile(s, LineConfigFileOptions()));

        if (exit_status != 0)
        {
            Log::get_instance()->message("paludis_environment.bash_conf.failure", ll_warning, lc_context)
                << "Script '" << f <<"' returned non-zero exit status '" << exit_status << "'";
            result.reset();
        }
    }
    else
        result.reset(new LineConfigFile(f, LineConfigFileOptions()));

    return result;
}

