#! /bin/sh

. scripts/testenv.sh

set +e

rm -f CPP_LIB_DIE_OUTPUT
$BIN_DIR/error-test > $GOLDEN_DIR/error.txt 2>&1
if [ 1 != "$?" ] ; then
  echo "Error test failed (should exit with 1)"
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
