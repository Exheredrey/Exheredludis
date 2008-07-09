#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh
#
# Based in part upon ebuild.sh from Portage, which is Copyright 1995-2005
# Gentoo Foundation and distributed under the terms of the GNU General
# Public License v2.
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License, version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA

addread()
{
    export SANDBOX_READ="${SANDBOX_READ+${SANDBOX_READ}:}${1}"
}

addwrite()
{
    export SANDBOX_WRITE="${SANDBOX_WRITE+${SANDBOX_WRITE}:}${1}"
}

adddeny()
{
    export SANDBOX_DENY="${SANDBOX_DENY+${SANDBOX_DENY}:}${1}"
}

addpredict()
{
    export SANDBOX_PREDICT="${SANDBOX_PREDICT+${SANDBOX_PREDICT}:}${1}"
}

