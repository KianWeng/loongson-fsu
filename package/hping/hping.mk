# Taken from the buildroot examples

HPING_VERSION = 20051105
HPING_SOURCE:=hping3-$(HPING_VERSION).tar.gz
HPING_SITE:=http://www.hping.org/
HPING_DEPENDENCIES = libpcap




define HPING_BUILD_CMDS
	$(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS)
endef

define HPING_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(HPING_DIR)/hping3 $(TARGET_DIR)/usr/bin/hping3
endef


$(eval $(generic-package))
