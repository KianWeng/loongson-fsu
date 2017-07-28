################################################################################
#
# makedevs
#
################################################################################

# source included in buildroot

PPPD_FILES_SOURCE = 
PPPD_FILES_SITE_METHOD = 
PPPD_FILES_SITE = 

PPPD_FILES_DEPENDENCIES = \
	pppd

define PPPD_FILES_INSTALL_TARGET_CMDS
	cp -av $(TOPDIR)/package/pppd-files/files/.  $(TARGET_DIR)/
endef


$(eval $(generic-package))
$(eval $(host-generic-package))
