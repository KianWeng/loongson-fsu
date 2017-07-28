################################################################################
#
# makedevs
#
################################################################################

# source included in buildroot
USBIP_SOURCE = 
USBIP_SITE_METHOD = local
USBIP_SITE = $(TOPDIR)/package/usbip/userspace

USBIP_DEPENDENCIES = \
	libsysfs libglib2

define USBIP_CONFIGURE_CMDS
	(cd $(@D); pwd; ./autogen.sh;rm -f config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CONFIGURE_ARGS) \
		./configure \
		--host mipsel-linux \
		--prefix=/usr \
	)
endef

define USBIP_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) CFLAGS="$(TARGET_CFLAGS) -g" LDFLAGS=-all-static
endef

define USBIP_INSTALL_TARGET_CMDS
	$(MAKE) DESTDIR=$(TARGET_DIR) PREFIX=/usr -C $(@D) install
endef


$(eval $(generic-package))
$(eval $(host-generic-package))
