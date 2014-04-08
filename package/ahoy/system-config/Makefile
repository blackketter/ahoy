#
# Copyright (C) 2013 Ahoy
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
# 

include $(TOPDIR)/rules.mk

PKG_NAME:=system-config
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/system-config
  SUBMENU:=
  CATEGORY:=Ahoy
  TITLE:=System Config
  DEPENDS:=+dropbear
endef

define Build/Prepare
endef

define Build/Compile
endef


define Package/system-config/description
  Linux system configuration files specific to Ahoy
endef

define Package/system-config/install
	$(INSTALL_DIR) $(1)/etc/dropbear
	$(INSTALL_DATA) ./files/authorized_keys $(1)/etc/dropbear/authorized_keys
	$(INSTALL_DIR) $(1)/etc/modules.d
	$(INSTALL_DATA) ./files/94-spi-gpio-custom $(1)/etc/modules.d/94-spi-gpio-custom
endef

$(eval $(call BuildPackage,system-config))
