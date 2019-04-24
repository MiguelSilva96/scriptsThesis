!#/bin/bash

# Setup the environment
sudo apt-get update

# Use a kernel version compatible with softiwarp
sudo apt-get install dist-upgrade

# Dependencies
# Build essentials
sudo apt-get install build-essential git cmake pkg-config libnl-3-dev libnl-route-3-dev

# RDMA libraries
sudo apt-get install libibverbs1  libibverbs-dev  librdmacm-dev  librdmacm1  ibverbs-utils  rdmacm-utils ibutils libibcommon1 ibdapl2 ibsim-utils libcxgb3-1 libibmad5 libibumad3 libmlx4-1 libmthca1 libnes1
# After this, reboot!
