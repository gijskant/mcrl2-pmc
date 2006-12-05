CONFIG = build/config.mk

-include $(CONFIG)

# Creates an application bundle on Mac OS X
.PHONY: all bjam install clean distclean distribution

all: $(CONFIG) $(BJAM)
	$(BOOST_BUILD)
	@$(MAKE) -C src/doc

install: $(CONFIG) $(BJAM)
	$(BOOST_BUILD) --install
	@$(MAKE) -C src/doc install

clean:
	@$(MAKE) -C src/doc clean
	$(RM) -rf build/bin/*
	$(RM) -r autom4te.cache config.log *.o *~ core core.*

test:
	$(BJAM) ./status

distclean:
	@${MAKE} -C src/doc distclean
	$(RM) -r autom4te.cache *.o *.app *~ core core.*
	$(RM) -r config.log config.status build/config.mk build/config.jam src/setup.h
	$(RM) -rf build/bin

configure: build/autoconf/configure.ac
	autoconf -o $@ -W all $<

include build/make/bjam.mk

$(CONFIG):
	$(error Please run configure first)
