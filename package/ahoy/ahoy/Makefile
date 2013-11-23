#
# Copyright (C) 2013 Ahoy
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ahoy
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/ahoy
  SUBMENU:=
  CATEGORY:=Ahoy
  TITLE:=Ahoy
  DEPENDS:=+lua
endef

define Package/ahoy/description
  Main Ahoy application
endef

define Build/Compile
endef

define Package/ahoy/install
	$(INSTALL_DIR) $(1)/ahoy
	$(INSTALL_DATA) ./files/hwcodec.lua $(1)/ahoy/hwcodec.lua
	$(INSTALL_BIN) ./files/ahoy.lua $(1)/ahoy/ahoy.lua
	$(INSTALL_DATA) ./files/i2c-test.lua $(1)/ahoy/i2c-test.lua

	$(INSTALL_DIR) $(1)/ahoy/sounds
	$(INSTALL_DATA) ./files/sounds/hail.wav $(1)/ahoy/sounds/hail.wav
endef

$(eval $(call BuildPackage,ahoy))
