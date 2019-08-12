#! /bin/bash
#
# Installs requirements on Ubuntu systems
# Last updated: Ubuntu 18, 8/2019
#

# Exit if any subprogram exits with an error
set -e

sudo apt install                  \
  cmake                           \
  exuberant-ctags                 \
  libeigen3-dev                   \
  libpng-dev libpng++-dev         \
  libboost1.65-dev                \
  clang-7                         \
  libc++-7-dev libc++abi-7-dev
