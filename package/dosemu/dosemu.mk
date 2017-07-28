DOSEMU_VERSION = master
DOSEMU_SITE = git://git.code.sf.net/p/dosemu/code
DOSEMU_SITE_METHOD = git

DOSEMU_CONF_OPTS = --without-alsa --without-sndfile  --without-gpm

DOSEMU_DEPENDENCIES = zlib 
# Patching aclocal.m4
DOSEMU_AUTORECONF = YES

$(eval $(autotools-package))
#$(eval $(generic-package))
