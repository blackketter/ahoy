#
# Copyright (C) 2013 Ahoy
#
# This is free software, licensed under the Apache 2 license.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=athplay
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk


define Package/athplay
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=ath-i2s audio player
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-o $(PKG_BUILD_DIR)/athplay $(PKG_BUILD_DIR)/athplay.c
endef


define Package/athplay/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/athplay $(1)/usr/bin/athplay
endef

$(eval $(call BuildPackage,athplay))
