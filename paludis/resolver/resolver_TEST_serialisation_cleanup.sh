#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

if [ -d resolver_TEST_serialisation_dir ] ; then
    rm -fr resolver_TEST_serialisation_dir
else
    true
fi

