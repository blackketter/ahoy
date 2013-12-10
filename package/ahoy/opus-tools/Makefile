# To build
# make oldconfig
# make package/opus-tools/compile V=99
# To just run the packaging again
# make package/opus-tools/compile V=99 CONFIG_AUTOREBUILD=

include $(TOPDIR)/rules.mk

PKG_NAME:=opus-tools
PKG_VERSION:=0.1.8
PKG_RELEASE:=$(PKG_SOURCE_VERSION)

PKG_SOURCE:=opus-tools-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://downloads.xiph.org/releases/opus/
PKG_MD5SUM:=b424790eda9357a4df394e2d7ca19eac

include $(INCLUDE_DIR)/package.mk

define Package/opus-tools
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Opus tools
  DEPENDS:=+opus +libogg +libflac
endef

# This uses the upstream install to put build products into $(PKG_INSTALL_DIR)
PKG_INSTALL:=1

# Now we pick what we really want to package from the build products
define Package/opus-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/opusenc $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/opusdec $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/opusinfo $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,opus-tools))
