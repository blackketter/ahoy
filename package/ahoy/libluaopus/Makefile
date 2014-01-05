#
# Copyright (C) 2013 Ahoy
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luaopus
PKG_RELEASE:=0.91

include $(INCLUDE_DIR)/package.mk

define Package/luaopus
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=LuaOpus
  DEPENDS:=+lua +opus
endef

define Package/luaopus/description
  LuaOpus provides a Lua interface to the Opus audio compression library
endef

define Build/Configure
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		LIBDIR="$(TARGET_LDFLAGS)" \
		CC="$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) -std=gnu99" \
		LD="$(TARGET_CROSS)ld -shared" \
		all
endef

define Package/luaopus/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/opus.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,luaopus))
