# -*- sh -*-

#This is a UNIX-only makefile.  For other systems, get a makefile from
#src/mkfiles/



jed: makefiles
	cd src; $(MAKE) jed
	@echo If you have X, try 'make xjed'
all: makefiles
	cd src; $(MAKE) all
xjed: makefiles
	cd src; $(MAKE) xjed
rgrep: makefiles
	cd src; $(MAKE) rgrep
Makefile: configure autoconf/Makefile.in
	@echo "Makefile is older than the configure script".
	@echo "Please re-run the configure script."
	@exit 1
src/Makefile: configure src/Makefile.in src/config.hin src/jed-feat.h
	@echo "src/Makefile is older than its dependencies".
	@echo "Please re-run the configure script."
	@exit 1
makefiles: Makefile src/Makefile
clean:
	/bin/rm -f *~
	cd src; $(MAKE) clean
#
distclean:
	/bin/rm -f *~ Makefile config.status config.log config.cache files.pck
	cd src; $(MAKE) distclean
#
install: makefiles
	cd src; $(MAKE) install
#
getmail: makefiles
	cd src; $(MAKE) getmail
	@echo getmail created.  Copy it to JED_ROOT/bin.

# The symlinks target is for my own private use.  It simply creates the object
# directory as a symbolic link to a local disk instead of an NFS mounted one.
symlinks:
	cd src; $(MAKE) symlinks
configure: autoconf/aclocal.m4 autoconf/configure.ac
	cd autoconf && autoconf && mv ./configure ..
update: autoconf/config.sub autoconf/config.guess
autoconf/config.guess: /usr/share/misc/config.guess
	/bin/cp -f /usr/share/misc/config.guess autoconf/config.guess
autoconf/config.sub: /usr/share/misc/config.sub
	/bin/cp -f /usr/share/misc/config.sub autoconf/config.sub

.PHONY: jed all xjed rgrep clean distclean install getmail symlinks makefiles
