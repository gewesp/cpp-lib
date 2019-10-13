if [ -d ../build ] ; then
  BIN_DIR=../build
else
  echo "Build directory ../build not present!"
  exit 1
fi

GOLDEN_DIR=data/golden-output
INPUT_DIR=data/input

set -x
