#! /bin/bash
#
# Installs requirements on Ubuntu systems
#
# Run with sudo.
#
# Last updated: Ubuntu 18, 10/2019
# Last updated: Gitlab CI (gcc Docker image); 10/2019
#

# Exit if any subprogram exits with an error
set -e

apt-get update --yes
apt-get install --yes cmake       \
  libeigen3-dev                   \
  libpng-dev libpng++-dev         \
  libboost-dev

if [[ "" == $GITLAB_CI ]] ; then
  apt-get install --yes             \
  exuberant-ctags                   \
  clang-7                           \
  libc++-7-dev libc++abi-7-dev
fi
