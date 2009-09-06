/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
 * Copyright (c) 2009 David Leverton
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

#include "pty.hh"
#include <paludis/util/exception.hh>
#include <paludis/util/log.hh>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include "config.h"

#ifdef HAVE_PTSNAME_R
#  include <vector>
#endif

using namespace paludis;

PtyError::PtyError(const std::string & our_message) throw () :
    Exception(our_message)
{
}

Pty::Pty()
{
    Context context("When creating pty FDs:");

    _fds[0] = posix_openpt(O_RDWR | O_NOCTTY);
    if (-1 == _fds[0])
        throw PtyError("posix_openpt(3) failed (is /dev/pts mounted?): " + std::string(std::strerror(errno)));
    if (-1 == grantpt(_fds[0]))
    {
        close(_fds[0]);
        throw PtyError("grantpt(3) failed: " + std::string(std::strerror(errno)));
    }
    if (-1 == unlockpt(_fds[0]))
    {
        close(_fds[0]);
        throw PtyError("unlockpt(3) failed: " + std::string(std::strerror(errno)));
    }

#ifdef HAVE_PTSNAME_R
    std::vector<char> name;
    name.resize(16);
    while (-1 == ptsname_r(_fds[0], &name[0], name.size()))
    {
        if (ERANGE == errno)
            name.resize(name.size() * 2);
        else
        {
            close(_fds[0]);
            throw PtyError("ptsname_r(3) failed: " + std::string(std::strerror(errno)));
        }
    }
    _fds[1] = open(&name[0], O_WRONLY | O_NOCTTY);
#else
    const char * name(ptsname(_fds[0]));
    if (0 == name)
    {
        close(_fds[0]);
        throw PtyError("ptsname(3) failed: " + std::string(std::strerror(errno)));
    }
    _fds[1] = open(name, O_WRONLY | O_NOCTTY);
#endif

    if (-1 == _fds[1])
    {
        close(_fds[0]);
        throw PtyError("open(2) failed: " + std::string(std::strerror(errno)));
    }

    Log::get_instance()->message("util.pty.fds", ll_debug, lc_context) << "Pty FDs are '" << read_fd() << "', '"
        << write_fd() << "'";
}

Pty::~Pty()
{
}

