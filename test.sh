#!/bin/sh

DIRNAME=`dirname $0`
REALPATH=`which realpath`
if [ ! -z "${REALPATH}" ]; then
  DIRNAME=`realpath ${DIRNAME}`
fi

if [ "$HPHP_HOME" != "" ]; then
    HHVM="${HPHP_HOME}/target/usr/local/bin/hhvm"
else
    HHVM=hhvm
fi

$HHVM -vDynamicExtensions.0=${DIRNAME}/shared_fifo.so ${DIRNAME}/test.php

