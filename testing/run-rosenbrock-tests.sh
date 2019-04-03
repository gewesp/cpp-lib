#! /bin/sh

set -e

. scripts/testenv.sh

# Only Rosenbrock search.  Not part of the standard test suite
# not deterministic,
set -e
$BIN_DIR/rosenbrock-search --repeat 10 4                                 \
  > $GOLDEN_DIR/rosenbrock-search-4.txt
$BIN_DIR/rosenbrock-search --repeat 10 5                                 \
  > $GOLDEN_DIR/rosenbrock-search-5.txt
$BIN_DIR/rosenbrock-search --repeat 1 --arguments 5                      \
  > $GOLDEN_DIR/rosenbrock-search-5-instrumented.txt
