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

# Locales required to install en_US.utf-8 locale
apt-get update --yes
apt-get install --yes cmake       \
  locales                         \
  libeigen3-dev                   \
  libpng-dev libpng++-dev         \
  libboost-dev

# https://community.appdynamics.com/t5/Dynamic-Languages-Node-JS-Python/locale-facet-S-create-c-locale-name-not-valid/td-p/33693
# Note: Argument of locale-gen undocumented in man page...
locale-gen en_US.UTF-8
# update-locale LANG=en_US.UTF-8

if [[ "" == $GITLAB_CI ]] ; then
  apt-get install --yes             \
  exuberant-ctags                   \
  clang-7                           \
  libc++-7-dev libc++abi-7-dev
fi
