LIBAUDIT_VERSION = debian
LIBAUDIT_SITE = git://anonscm.debian.org/collab-maint/audit.git
LIBAUDIT_SITE_METHOD = git

LIBAUDIT_AUTORECONF = YES
$(eval $(autotools-package))
