# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2023 JELOS (https://github.com/JustEnoughLinuxOS)

PKG_NAME="arm"
PKG_LICENSE="GPLv2"
PKG_SITE="https://rocknix.org"
PKG_DEPENDS_TARGET="toolchain squashfs-tools:host dosfstools:host fakeroot:host kmod:host mtools:host populatefs:host libc gcc libusb  SDL2 SDL2_gfx SDL2_image SDL2_mixer SDL2_net SDL2_ttf"
PKG_SECTION="virtual"
PKG_LONGDESC="Root package used to build and create 32-bit userland"

### Display Drivers
if [ ! "${OPENGL}" = "no" ]; then
  PKG_DEPENDS_TARGET+=" ${OPENGL} glu libglvnd glew"
fi

if [ "${OPENGLES_SUPPORT}" = yes ]; then
  PKG_DEPENDS_TARGET+=" ${OPENGLES}"
fi

if [ "${VULKAN_SUPPORT}" = "yes" ]
then
  PKG_DEPENDS_TARGET+=" vulkan-loader vulkan-headers"
fi

if [ "${DISPLAYSERVER}" = "wl" ]; then
  PKG_DEPENDS_TARGET+=" wayland ${WINDOWMANAGER} libXtst libXfixes libXi gdk-pixbuf libvdpau"
fi

### Audio
if [ "${PIPEWIRE_SUPPORT}" = "yes" ]; then
  PKG_DEPENDS_TARGET+=" pipewire"
fi

### Emulators and Cores
EMUS_32BIT=$(bash -c ". ${ROOT}/packages/virtual/emulators/package.mk; echo \$EMUS_32BIT")
PKG_DEPENDS_TARGET+=" retroarch ${EMUS_32BIT}"

PKG_DEPENDS_TARGET+=" ${ADDITIONAL_PACKAGES}"
