#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

default_src_configure()
{
    if [[ -x ${ECONF_SOURCE:-.}/configure ]] ; then
        econf \
            ${DEFAULT_SRC_CONFIGURE_PARAMS} \
            $(for s in ${DEFAULT_SRC_CONFIGURE_OPTION_ENABLES} ; do \
                option_enable "${s}" ; \
            done ) \
            $(for s in ${DEFAULT_SRC_CONFIGURE_OPTION_WITHS} ; do \
                option_with "${s}" ; \
            done )
    fi
}

src_configure()
{
    default "$@"
}

exheres_internal_configure()
{
    if [[ -d "${S}" ]] ; then
        cd "${S}" || die "cd to \${S} (\"${S}\") failed"
    elif [[ -d "${WORKDIR}" ]] ; then
        cd "${WORKDIR}" || die "cd to \${WORKDIR} (\"${WORKDIR}\") failed"
    fi

    if hasq "configure" ${RESTRICT} ; then
        ebuild_section "Skipping src_configure (RESTRICT)"
    elif hasq "configure" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_configure (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting src_configure"
        src_configure
        ebuild_section "Done src_configure"
    fi
}

