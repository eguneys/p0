#!/usr/bin/env bash

pushd "$(dirname "$0")"

case $1 in
    plain|debug|release)
        BUILDTYPE=$1
        ;;
    *)
        BUILDTYPE=release
        ;;
esac


BUILDDIR=build/${BUILDTYPE}

meson ${BUILDDIR} --buildtype ${BUILDTYPE} "$@"

cd ${BUILDDIR}

NINJA=$(awk '/ninja/ {ninja=$4} END {print ninja}' meson-logs/meson-log.txt)

if [ -n "${INSTALL_PREFIX}" ]
then
    ${NINJA} install
else
    ${NINJA}
fi

popd
