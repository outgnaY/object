AUTOMAKE_OPTIONS = subdir-objects

noinst_PROGRAMS = object test_bson test_req test_hash test_sds test_string test_array

AM_CPPFLAGS = -I$(srcdir)/include

object_SOURCES = bson/obj_bson_iter.c bson/obj_bson_validate.c bson/obj_bson_visit.c bson/obj_bson.c \
				 mem/obj_mem_simple.c mem/obj_mem.c \
				 network/obj_buffer.c network/obj_conn.c network/obj_proto.c \
				 util/obj_hash.c util/obj_list.c util/obj_math.c util/obj_sds.c util/obj_siphash.c util/obj_string.c \
				 obj_daemon.c obj_thread.c obj.c
test_bson_SOURCES = test/test_bson.c bson/obj_bson_iter.c bson/obj_bson_validate.c bson/obj_bson_visit.c bson/obj_bson.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_list.c util/obj_math.c util/obj_string.c
test_req_SOURCES = test/test_req.c mem/obj_mem_simple.c mem/obj_mem.c bson/obj_bson.c util/obj_math.c
test_hash_SOURCES = test/test_hash.c util/obj_hash.c util/obj_siphash.c mem/obj_mem_simple.c mem/obj_mem.c
test_sds_SOURCES = test/test_sds.c util/obj_sds.c mem/obj_mem_simple.c mem/obj_mem.c
test_string_SOURCES = test/test_string.c util/obj_string.c
test_array_SOURCES = test/test_array.c util/obj_array.c util/obj_list.c mem/obj_mem_simple.c mem/obj_mem.c

all:
	mv -f object ../bin/object
	mv -f test_bson ../bin/test_bson
	mv -f test_req ../bin/test_req
	mv -f test_hash ../bin/test_hash
	mv -f test_sds ../bin/test_sds
	mv -f test_string ../bin/test_string
	mv -f test_array ../bin/test_array
	mv -f obj_daemon.o obj_thread.o obj.o bson/*.o mem/*.o network/*.o test/*.o util/*.o ../obj
clean:
	rm -f ../bin/object
	rm -f ../bin/test_bson
	rm -f ../bin/test_req
	rm -f ../bin/test_hash
	rm -f ../bin/test_sds
	rm -f ../bin/test_string
	rm -f ../bin/test_array
	rm -f ../obj/*
clean-local:
	-rm config.log 
