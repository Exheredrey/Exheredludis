#!/bin/bash
# vim: set sw=4 sts=4 et :

shopt -s expand_aliases
shopt -s extglob
set +o posix

export TEST_STATUS=
export PALUDIS_IN_TEST_FRAMEWORK="yes"
unset PALUDIS_UTILITY_PATH_SUFFIXES PALUDIS_EBUILD_MODULE_SUFFIXES

test_return_code()
{
    local r="$?"
    if [[ "0" == "${r}" ]] ; then
        echo -n "."
    else
        echo -n "!{retcode: ${r}}"
        export local_test_status="fail"
        export TEST_STATUS="fail"
    fi
}

test_equality()
{
    if [[ "${1}" == "${2}" ]] ; then
        echo -n "."
    else
        echo -n "!{'${1}' not equal to '${2}'}"
        export local_test_status="fail"
        export TEST_STATUS="fail"
    fi
}

echo "Test program ${1}:"
source "$(dirname ${1} )/ebuild.bash" || exit 200
source "${1}" || exit 200

for testname in $(set | grep '_TEST *() *$' ) ; do
    [[ ${testname/()} != ${testname} ]] && continue
    echo -n "* ${testname%_TEST}: "
    export local_test_status=""
    ${testname}
    [[ -z "$local_test_status" ]] && echo " OK" || echo " FAIL"
done

[[ -z "$TEST_STATUS" ]]

