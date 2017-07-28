################################################################################
#
# pmon
#
################################################################################
PMON_VERSION = $(call qstrip,$(BR2_BOOT_PMON_VERSION))
PMON_SITE_TMP = $(call qstrip,$(BR2_BOOT_PMON_CUSTOM_LOCAL_PATH)) 
PMON_SITE = $(call qstrip,$(PMON_SITE_TMP))
PMON_SITE_METHOD = local
PMON_BOARD_NAME = $(call qstrip,$(BR2_BOOT_PMON_BOARDNAME))
PMON_DIR_PATH = output/build/pmon-$(PMON_VERSION)/zloader.$(PMON_BOARD_NAME)
PMON_GCC_PATH = $(call qstrip,$(BR2_TOOLCHAIN_EXTERNAL_PATH))bin/mipsel-linux-

#PMON_LICENSE = GPLv2+
#PMON_LICENSE_FILES = README

PMON_INSTALL_IMAGES = YES

define PMON_BUILD_CMDS
	cd $(PMON_DIR_PATH) && make cfg all tgt=rom CROSS_COMPILE=$(PMON_GCC_PATH) DEBUG=-g && ./mkenv
endef

define PMON_INSTALL_IMAGES_CMDS
	cp $(PMON_DIR_PATH)/gzrom.bin $(BINARIES_DIR) 
endef

$(eval $(generic-package))

ifeq ($(BR2_TARGET_PMON),y)
# we NEED a board name unless we're at make source	
ifeq ($(filter source,$(MAKECMDGOALS)),)

ifeq ($(PMON_SITE),)
$(error NO pmon custom source site set. Check your BR2_BOOT_PMON_CUSTOM_LOCAL_PATH setting)
endif

ifeq ($(PMON_BOARD_NAME),)
$(error NO pmon board name set. Check your BR2_BOOT_PMON_BOARDNAME setting)
endif

endif

endif
