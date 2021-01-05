AUTOMAKE_OPTIONS = subdir-objects

noinst_PROGRAMS = test_hash test_sds

AM_CPPFLAGS = -I$(srcdir)/include

test_hash_SOURCES = test/test_hash.c util/obj_hash.c util/obj_siphash.c mem/obj_mem_simple.c mem/obj_mem.c
test_sds_SOURCES = test/test_sds.c util/obj_sds.c mem/obj_mem_simple.c mem/obj_mem.c

all:
	mv -f test_hash ../bin/test_hash
	mv -f test_sds ../bin/test_sds
	mv -f mem/*.o test/*.o util/*.o ../obj
clean:
	rm -f ../bin/test_hash
	rm -f ../bin/test_sds
	rm -f ../obj/*
clean-local:
	-rm config.log 
