
SUBDIRS = lib plugins src themes

install-exec-hook: 
	if test -d $(srcdir)/themes; then \
	 $(mkinstalldirs) $(THEME_DIR); \
	  for theme in $(srcdir)/themes/*; do \
	   if test -d $$theme; then \
	    cp -R $$theme $(THEME_DIR); \
	   fi \
	 done \
	fi

AUTOMAKE_OPTIONS = foreign 
