#
# Copyright (C) 2013 Ahoy
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luaavahi
PKG_RELEASE:=0.01

include $(INCLUDE_DIR)/package.mk

define Package/luaavahi
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=LuaAvahi
  DEPENDS:=+lua +libavahi
endef

define Package/luaavahi/description
  LuaAvahi provides a Lua interface to Avahi
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

define Package/luaavahi/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/avahi.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,luaavahi))
