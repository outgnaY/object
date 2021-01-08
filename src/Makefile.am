AUTOMAKE_OPTIONS = subdir-objects

noinst_PROGRAMS = test_bson test_hash test_sds

AM_CPPFLAGS = -I$(srcdir)/include

test_bson_SOURCES = test/test_bson.c bson/obj_bson_iter.c bson/obj_bson_validate.c bson/obj_bson_visit.c bson/obj_bson.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_list.c util/obj_math.c
test_hash_SOURCES = test/test_hash.c util/obj_hash.c util/obj_siphash.c mem/obj_mem_simple.c mem/obj_mem.c
test_sds_SOURCES = test/test_sds.c util/obj_sds.c mem/obj_mem_simple.c mem/obj_mem.c

all:
	mv -f test_bson ../bin/test_bson
	mv -f test_hash ../bin/test_hash
	mv -f test_sds ../bin/test_sds
	mv -f bson/*.o mem/*.o test/*.o util/*.o ../obj
clean:
	rm -f ../bin/test_bson
	rm -f ../bin/test_hash
	rm -f ../bin/test_sds
	rm -f ../obj/*
clean-local:
	-rm config.log 
