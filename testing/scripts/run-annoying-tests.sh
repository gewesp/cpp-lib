#! /bin/bash

. scripts/testenv.sh

set +e

rm -f CPP_LIB_DIE_OUTPUT
error_out=$(mktemp)
$BIN_DIR/error-test > "$error_out" 2>&1
if [ 1 != "$?" ] ; then
  echo "Error test failed (should exit with 1)"
  exit 1
fi

lines1=$(egrep '\(EMERGENCY\) Assertion failed: 5 == 2 \+ 2 .*/error-test.cpp.*' CPP_LIB_DIE_OUTPUT | wc -l)

lines2=$(egrep '\(EMERGENCY\) Assertion failed: 5 == 2 \+ 2 .*/error-test.cpp.*' "$error_out" | wc -l)
if [[ 1 != $lines1 ]] ; then
  echo "Error test failed: Incorrect message in CPP_LIB_DIE_OUTPUT"
  exit 1
fi

if [[ 2 != $lines2 ]] ; then
  echo "Error test failed: Incorrect message in $error_out"
  exit 1
fi

set -e

$BIN_DIR/audio-test $INPUT_DIR/triad.melody
mv $INPUT_DIR/triad.snd $GOLDEN_DIR

if [ Darwin = `uname` ] ; then
  TESTDIR=/tmp/cpp-lib-test-output
  mkdir $TESTDIR
  afconvert -d LEI16 -f caff --verbose $GOLDEN_DIR/triad.snd -o $TESTDIR/triad.caf
  open $TESTDIR
fi
