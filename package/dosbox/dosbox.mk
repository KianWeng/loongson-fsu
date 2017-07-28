DOSBOX_VERSION = 3914
DOSBOX_SITE = package/dosbox/src/dosbox-3914/
DOSBOX_SITE_METHOD = local

#DOSBOX_SITE = svn://svn.code.sf.net/p/dosbox/code-0/dosbox/trunk
#DOSBOX_SITE_METHOD = svn

#DOSBOX_CONF_OPTS = --without-alsa --without-sndfile  --without-gpm
TARGET_CFLAGS += -g
TARGET_CXXFLAGS += -g
DOSBOX_CONF_ENV += SDL_CONFIG=$(STAGING_DIR)/usr/bin/sdl-config

DOSBOX_DEPENDENCIES = zlib libpng  sdl_net sdl
#alsa-lib
# Patching aclocal.m4
DOSBOX_AUTORECONF = YES


define DOSBOX_INSTALL_TARGET_CMDS
	#$(MAKE) -C $(@D) DESTDIR="$(TARGET_DIR)" install
	$(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_INSTALL_TARGET_OPTS) -C $($(PKG)_SRCDIR) install
	$(INSTALL) -D  package/dosbox/dosbox.conf $(TARGET_DIR)/root/.dosbox/dosbox.conf
	$(INSTALL) -D  package/dosbox/dosbox.conf $(TARGET_DIR)/root/.dosbox/dosbox-SVN.conf
	$(INSTALL) -D  package/dosbox/rundosbox $(TARGET_DIR)/usr/bin/rundosbox
endef
$(eval $(autotools-package))
#$(eval $(generic-package))
