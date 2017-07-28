################################################################################
#
# qwt
#
################################################################################

MYQT_VERSION = 1.0
MYQT_SITE = package/myqt/src/
MYQT_SITE_METHOD = local
MYQT_DEPENDENCIES = qt


define MYQT_CONFIGURE_CMDS
	(cd $(@D); $(TARGET_MAKE_ENV) $(QT_QMAKE))
endef

define MYQT_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


$(eval $(generic-package))
