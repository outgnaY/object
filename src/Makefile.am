AUTOMAKE_OPTIONS = subdir-objects

noinst_PROGRAMS = test_hash 

AM_CPPFLAGS = -I$(srcdir)/include

test_hash_SOURCES = test/test_hash.c util/obj_hash.c util/obj_siphash.c mem/obj_mem_simple.c mem/obj_mem.c

all:
	mv -f test_hash ../bin/test_hash
	mv -f bson/*.o mem/*.o network/*.o storage/*.o test/*.o util/*.o ../obj
clean:
	rm -f ../bin/test_hash
	rm -f ../obj/*