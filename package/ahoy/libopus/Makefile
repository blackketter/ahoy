# To build
# make oldconfig
# make package/libopus/compile V=99
# To just run the packaging again
# make package/libopus/compile V=99 CONFIG_AUTOREBUILD=

include $(TOPDIR)/rules.mk

PKG_NAME:=libopus
PKG_VERSION:=1.1
PKG_RELEASE:=$(PKG_SOURCE_VERSION)

PKG_FILE_NAME:=opus

PKG_SOURCE:=$(PKG_FILE_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://downloads.xiph.org/releases/opus/
PKG_MD5SUM:=c5a8cf7c0b066759542bc4ca46817ac6

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_FILE_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/libopus
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Opus codec library
endef

# This uses the upstream install to put build products into $(PKG_INSTALL_DIR)
PKG_INSTALL:=1

CONFIGURE_ARGS += --enable-fixed-point

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/opus
	$(CP) $(PKG_INSTALL_DIR)/usr/include/opus/opus*.h $(1)/usr/include/opus/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopus.{a,so*} $(1)/usr/lib/
endef

define Package/libopus/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopus.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libopus))
