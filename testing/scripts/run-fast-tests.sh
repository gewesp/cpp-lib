#! /bin/sh

. scripts/testenv.sh

$BIN_DIR/gnss-test unittest                 > $GOLDEN_DIR/gnss.txt
$BIN_DIR/map-test                           > $GOLDEN_DIR/map.txt
$BIN_DIR/math-test                          > $GOLDEN_DIR/math.txt
$BIN_DIR/matrix-wrapper-test                > $GOLDEN_DIR/matrix-wrapper.txt
$BIN_DIR/interpolation-test                 > $GOLDEN_DIR/interpolation.txt
$BIN_DIR/optimization-test                  > $GOLDEN_DIR/optimization.txt
$BIN_DIR/rosenbrock-search --repeat 10 4                                 \
  > $GOLDEN_DIR/rosenbrock-search-4.txt
$BIN_DIR/rosenbrock-search --repeat 10 5                                 \
  > $GOLDEN_DIR/rosenbrock-search-5.txt
$BIN_DIR/rosenbrock-search --repeat 1 --arguments 5                      \
  > $GOLDEN_DIR/rosenbrock-search-5-instrumented.txt
$BIN_DIR/quaternion-test                    > $GOLDEN_DIR/quaternion.txt
$BIN_DIR/registry-test                      > $GOLDEN_DIR/registry.txt

$BIN_DIR/gnss-test kml $INPUT_DIR/greifensee.kml                         \
  kml.Document.Placemark.Polygon.outerBoundaryIs.LinearRing.coordinates  \
                                            > $GOLDEN_DIR/gnss-kml.txt
$BIN_DIR/gnss-test airport_db $INPUT_DIR/airfields.csv                   \
  $INPUT_DIR/airfield-queries.txt                                        \
                                            > $GOLDEN_DIR/airport-db.txt
$BIN_DIR/gnss-test geoid ../data/ww15mgh.grd                             \
                                            > $GOLDEN_DIR/geoid.txt

$BIN_DIR/gnss-test airport_db_openaip                                    \
  $INPUT_DIR/openaip_airports_switzerland_ch.aip                         \
  $INPUT_DIR/airfield-queries.txt                                        \
                                            > $GOLDEN_DIR/airport-db-openaip.txt

$BIN_DIR/ogn-test --file $INPUT_DIR/ogn-1.ogn --ddb_query_interval -1    \
                  --utc 1436467342                                       \
                                            > $GOLDEN_DIR/ogn-1.txt
$BIN_DIR/ogn-test --file $INPUT_DIR/ogn-1.ogn --ddb_query_interval -1    \
                  --utc 1436467342 --thermals 1                          \
                                            > $GOLDEN_DIR/ogn-1-thermals.txt
$BIN_DIR/ogn-test --file $INPUT_DIR/ogn-1.ogn --ddb_query_interval -1    \
                  --utc 1436467342 --thermals 2                          \
                                            > $GOLDEN_DIR/ogn-1-thermals-2.txt
$BIN_DIR/ogn-test --file $INPUT_DIR/ogn-2.ogn --ddb_query_interval -1    \
                                            > $GOLDEN_DIR/ogn-2.txt
$BIN_DIR/ogn-test --unittests               > $GOLDEN_DIR/ogn-unittests.txt
$BIN_DIR/table-test < $INPUT_DIR/table1.txt > $GOLDEN_DIR/table1.txt
$BIN_DIR/util-test  < $INPUT_DIR/util.txt   > $GOLDEN_DIR/util.txt

$BIN_DIR/tcp-test reverse test:stdio < $INPUT_DIR/tcp.txt > $GOLDEN_DIR/tcp.txt

$BIN_DIR/top-n-test

$BIN_DIR/file-test --tee $GOLDEN_DIR/tee1.txt $GOLDEN_DIR/tee2.txt $GOLDEN_DIR/tee3.txt << __EOF__
All
three 
files
should have
the same
contents!
__EOF__

$BIN_DIR/file-test --fileops                > $GOLDEN_DIR/fileops.txt

$BIN_DIR/syslogger-test                     > $GOLDEN_DIR/syslogger.txt
