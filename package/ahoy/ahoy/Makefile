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
	$(INSTALL_DIR) $(1)/ahoy
	$(INSTALL_BIN) ./files/ahoy.lua $(1)/ahoy/ahoy.lua

	$(INSTALL_DATA) ./files/codec.lua $(1)/ahoy/codec.lua
	$(INSTALL_DATA) ./files/leds.lua $(1)/ahoy/leds.lua
	$(INSTALL_DATA) ./files/buttons.lua $(1)/ahoy/buttons.lua
	$(INSTALL_DATA) ./files/utils.lua $(1)/ahoy/utils.lua
	$(INSTALL_DATA) ./files/audio.lua $(1)/ahoy/audio.lua

	$(INSTALL_DATA) ./files/strict.lua $(1)/ahoy/strict.lua
	$(INSTALL_DATA) ./files/syslogger.lua $(1)/ahoy/syslogger.lua

	$(INSTALL_DATA) ./files/i2c-test.lua $(1)/ahoy/i2c-test.lua

	$(INSTALL_DIR) $(1)/ahoy/sounds
	$(INSTALL_DATA) ./files/sounds/hail.wav $(1)/ahoy/sounds/hail.wav
	$(INSTALL_DATA) ./files/sounds/1k.wav $(1)/ahoy/sounds/1k.wav
	$(INSTALL_DATA) ./files/sounds/vol.wav $(1)/ahoy/sounds/vol.wav

	$(INSTALL_DATA) ./files/sounds/whistle.opus $(1)/ahoy/sounds/whistle.opus
	$(INSTALL_DATA) ./files/sounds/setup-on.opus $(1)/ahoy/sounds/setup-on.opus
	$(INSTALL_DATA) ./files/sounds/setup-off.opus $(1)/ahoy/sounds/setup-off.opus

	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/i2s.so $(1)/usr/lib/lua

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ahoy.init $(1)/etc/init.d/ahoy

endef

$(eval $(call BuildPackage,ahoy))
