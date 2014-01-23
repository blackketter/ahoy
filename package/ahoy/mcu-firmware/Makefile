#
# Copyright (C) 2013 Ahoy
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mcu-firmware
PKG_RELEASE:=0.1

include $(INCLUDE_DIR)/package.mk

define Package/mcu-firmware
  SECTION:=ahoy
  CATEGORY:=Ahoy
  TITLE:=mcu-firmware
endef

define Package/mcu-firmware/description
  Firmware for Ahoy IO microcontroller
endef

define Build/Configure
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) all
endef

define Package/luai2c/install
	$(INSTALL_DIR) $(1)/ahoy/mcu
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/main.hex $(1)/ahoy/mcu/main.hex
endef

$(eval $(call BuildPackage,mcu-firmware))
