#
# Copyright (C) 2013 Ahoy
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ath-i2s
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ath-i2s
  SUBMENU:=Sound Support
  TITLE:=Atheros AR9331 I2S Audio Driver
  FILES:=$(PKG_BUILD_DIR)/ath-i2s.ko
  AUTOLOAD:=$(call AutoLoad,30,ath-i2s)
  KCONFIG:=
endef

define KernelPackage/ath-i2s/description
  Kernel module for Atheros AR9331 I2S audio
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(BUILDFLAGS)" \
		modules
endef

define KernelPackage/ath-i2s/install
		$(INSTALL_DIR) $(1)/etc/init.d
		$(INSTALL_BIN) ./files/ath-i2s.init $(1)/etc/init.d/ath-i2s
endef

$(eval $(call KernelPackage,ath-i2s))
