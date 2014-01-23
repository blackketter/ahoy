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

define Package/system-config/description
  Linux system configuration files specific to Ahoy
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		LIBDIR="$(TARGET_LDFLAGS)" \
		CC="$(TARGET_CC) -Wall -Werror $(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
		LD="$(TARGET_CROSS)ld -shared" \
		all
endef

define Package/ahoy/install
	$(INSTALL_DIR) $(1)/etc/dropbear
	$(INSTALL_DATA) ./files/authorized-keys $(1)/etc/dropbear/authorized-keys
endef

$(eval $(call BuildPackage,system-config))
