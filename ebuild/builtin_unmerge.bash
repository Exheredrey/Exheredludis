#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation; either version
# 2 of the License, or (at your option) any later version.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA

builtin_unmerge()
{
    local dbdir="${ROOT}"/var/db/pkg/"${CATEGORY}/${PF}" entry
    [[ -d "${dbdir}" ]] || die "couldn't find pkg db directory (\"${dbdir}\")"

    for v in CATEGORY CBUILD CFLAGS CHOST CXXFLAGS DEPEND DESCRIPTION EAPI \
        FEATURES HOMEPAGE INHERITED IUSE KEYWORDS LICENSE PDEPEND PF \
        PROVIDE RDEPEND SLOT SRC_URI USE ; do
        eval "${v}=\$(< ${dbdir}/${v} ) || die \"Load key ${v} failed\""
    done

    if [[ -f ${dbdir}/CONFIG_PROTECT ]]; then
        CONFIG_PROTECT="$(< ${dbdir}/CONFIG_PROTECT) ${CONFIG_PROTECT}"
        if [[ -f ${dbdir}/CONFIG_PROTECT_MASK ]]; then
            CONFIG_PROTECT_MASK="$(< ${dbdir}/CONFIG_PROTECT_MASK)"
        fi
    else
        eval $(bzcat "${dbdir}/environment.bz2" | while read line; do
            if [[ ${line%%=*} == CONFIG_PROTECT ]]; then
                echo "CONFIG_PROTECT='${line#*=} ${CONFIG_PROTECT}'"
            elif [[ ${line%%=*} == CONFIG_PROTECT_MASK ]]; then
                echo "CONFIG_PROTECT_MASK='${line#*=}'"
            fi
        done)
    fi

    if [[ -n ${PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT} ]]; then
        CONFIG_PROTECT=${PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT}
    fi
    if [[ -n ${PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK} ]]; then
        CONFIG_PROTECT_MASK=${PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK}
    fi

    export CONFIG_PROTECT="${CONFIG_PROTECT}"
    export CONFIG_PROTECT_MASK="${CONFIG_PROTECT_MASK}"

    ${PALUDIS_EBUILD_MODULES_DIR}/utils/unmerge "${ROOT}/" "${dbdir}/CONTENTS" \
        || die "unmerge failed"

    rm -fr "${dbdir}"
}

ebuild_f_unmerge()
{
    local old_sandbox_write="${SANDBOX_WRITE}"
    SANDBOX_WRITE="${SANDBOX_WRITE+${SANDBOX_WRITE}:}${ROOT%/}/"
    local old_sandbox_on="${SANDBOX_ON}"
    [[ "$(canonicalise ${ROOT} )" != "/" ]] || SANDBOX_ON=0

    if hasq "unmerge" ${RESTRICT} ; then
        ebuild_section "Skipping builtin_unmerge (RESTRICT)"
    elif hasq "unmerge" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping builtin_unmerge (SKIP_FUNCTIONS)"
    else
        ebuild_section "Starting builtin_unmerge"
        builtin_unmerge
        ebuild_section "Done builtin_unmerge"
    fi

    SANDBOX_WRITE="${old_sandbox_write}"
    SANDBOX_ON="${old_sandbox_on}"
}


