# To build
# make oldconfig
# make package/libopusfile/compile V=99
# To just run the packaging again
# make package/libopusfile/compile V=99 CONFIG_AUTOREBUILD=

include $(TOPDIR)/rules.mk

PKG_NAME:=libopusfile
PKG_VERSION:=0.5
PKG_RELEASE:=$(PKG_SOURCE_VERSION)

PKG_FILE_NAME:=opusfile

PKG_SOURCE:=$(PKG_FILE_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://downloads.xiph.org/releases/opus/
PKG_MD5SUM:=6c96ae240ada27fafff423f96a8814cb

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_FILE_NAME)-$(PKG_VERSION)

PKG_BUILD_DEPENDS:=libogg libopus libopenssl

include $(INCLUDE_DIR)/package.mk

define Package/libopusfile
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Opusfile library
  DEPENDS:= +libogg +libopus +libopenssl
endef

# This uses the upstream install to put build products into $(PKG_INSTALL_DIR)
PKG_INSTALL:=1

CONFIGURE_ARGS += --enable-fixed-point

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/opus
	$(CP) $(PKG_INSTALL_DIR)/usr/include/opus/opusfile.h $(1)/usr/include/opus/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopusfile.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopusurl.{a,so*} $(1)/usr/lib/
endef

define Package/libopusfile/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopusfile.so.* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopusurl.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libopusfile))
