#!/bin/bash
USER="miguelsilvalac15"

# Dependencies
# Build essentials
sudo apt-get install build-essential git cmake pkg-config libnl-3-dev libnl-route-3-dev

# RDMA libraries
sudo apt-get install libibverbs1  libibverbs-dev  librdmacm-dev  librdmacm1  ibverbs-utils  rdmacm-utils ibutils libibcommon1 ibdapl2 ibsim-utils libcxgb3-1 libibmad5 libibumad3 libmlx4-1 libmthca1 libnes1

# get and build softiwarp kernel
git clone https://github.com/zrlio/softiwarp
cd softiwarp
git checkout dev-siw.mem_ext
cd kernel
make

# load kernel modules
sudo insmod ./siw.ko
sudo modprobe rdma_ucm

cd $HOME
# get compile the userspace library
git clone https://github.com/zrlio/softiwarp-user-for-linux-rdma
cd softiwarp-user-for-linux-rdma
cmake .
make

# Add lib to the LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/$USER/softiwarp-user-for-linux-rdma/lib/

# Make the driver file entry, this is a bit ugly hack
sudo mkdir /usr/local/etc/libibverbs.d/
sudo cp /home/$USER/softiwarp-user-for-linux-rdma/etc/libibverbs.d/* /usr/local/etc/libibverbs.d/
