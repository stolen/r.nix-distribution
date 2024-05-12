# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2023 JELOS (https://github.com/JustEnoughLinuxOS)

PKG_NAME="generic-dsi"
PKG_VERSION="0.1.0"
PKG_LICENSE="BSD"
#PKG_URL="${PKG_SITE}/archive/refs/tags/v${PKG_VERSION}.tar.gz"
PKG_LONGDESC="generic DSI panel driver"
PKG_DEPENDS_TARGET="toolchain linux"
PKG_DEPENDS_INIT="toolchain"
PKG_NEED_UNPACK="${LINUX_DEPENDS}"
PKG_TOOLCHAIN="manual"
PKG_IS_KERNEL_PKG="yes"

make_target() {
  echo 'obj-m += panel-generic-dsi.o' > ${PKG_BUILD}/Makefile
  kernel_make -C $(kernel_path) M=${PKG_BUILD}
}

makeinstall_target() {
  mkdir -p ${INSTALL}/$(get_full_module_dir)/${PKG_NAME}
  cp *.ko ${INSTALL}/$(get_full_module_dir)/${PKG_NAME}/
}

make_init() {
  #make_target
}

makeinstall_init() {
  mkdir -p ${INSTALL}/lib
  cp -av ${PKG_DIR}/initramfs/* ${INSTALL}/
  cp -av ${PKG_DIR}/firmware ${INSTALL}/lib/
  #cp *.ko ${INSTALL}/lib/modules/
}
