if [ -d ../build ] ; then
  BIN_DIR=../build
else
  if [ true = "$debug" ] ; then
    BIN_DIR=../bin/dbg
  else 
    BIN_DIR=../bin/opt
  fi
fi

GOLDEN_DIR=data/golden-output
INPUT_DIR=data/input
