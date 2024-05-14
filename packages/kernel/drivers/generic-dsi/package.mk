# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2024 ROCKNIX (https://github.com/ROCKNIX)

PKG_NAME="generic-dsi"
PKG_VERSION="0.1.0"
PKG_LICENSE="GPL"
PKG_LONGDESC="generic DSI panel driver"
PKG_NEED_UNPACK="${LINUX_DEPENDS}"
PKG_TOOLCHAIN="manual"
PKG_IS_KERNEL_PKG="yes"

make_target() {
  if [ "${I_AM_DEVELOPER}" != "yes" ]; then
    echo "######   you should not build ${PKG_NAME} as package!  ######" >&2;
    exit 3;
  fi
  echo 'obj-m += panel-generic-dsi.o' > ${PKG_BUILD}/Makefile
  kernel_make -C $(kernel_path) M=${PKG_BUILD}
}

makeinstall_target() {
  :
  #mkdir -p ${INSTALL}/$(get_full_module_dir)/${PKG_NAME}
  #cp *.ko ${INSTALL}/$(get_full_module_dir)/${PKG_NAME}/
}

make_init() {
  :
}

makeinstall_init() {
  mkdir -p ${INSTALL}/lib
  cp -av ${PKG_DIR}/firmware ${INSTALL}/lib/
}
