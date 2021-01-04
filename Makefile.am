SUBDIRS = src

clean-local:
	-rm config.log
	for dir in $(SUBDIRS); do \
		(cd $$dir && $(MAKE) $(AM_MAKEFLAGS) $@) || exit 1; \
	done
