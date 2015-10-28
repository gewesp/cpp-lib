#! /bin/sh

. scripts/testenv.sh

./scripts/run-fast-tests.sh
./scripts/run-annoying-tests.sh

$BIN_DIR/spatial-index-test                 > $GOLDEN_DIR/spatial-index.txt
