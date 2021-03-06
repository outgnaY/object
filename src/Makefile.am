AUTOMAKE_OPTIONS = subdir-objects

noinst_PROGRAMS = object test_bson test_req test_hash test_sds test_string test_array test_expr test_bcon test_rbtree test_object_pool test_prealloc_map test_lock test_stringdata

AM_CPPFLAGS = -I$(srcdir)/include

object_SOURCES = bson/obj_bson_iter.c bson/obj_bson_validate.c bson/obj_bson_visit.c bson/obj_bson.c \
				 mem/obj_mem_simple.c mem/obj_mem.c \
				 util/obj_hash.c util/obj_list.c util/obj_math.c util/obj_sds.c util/obj_siphash.c util/obj_string.c \
				 concurrency/obj_concurrency.c concurrency/obj_lock.c concurrency/obj_locker.c concurrency/obj_prealloc_map.c \
				 network/obj_buffer.c network/obj_conn.c network/obj_proto.c \
				 obj_daemon.c obj_thread.c obj.c
test_bson_SOURCES = test/test_bson.c bson/obj_bson_iter.c bson/obj_bson_validate.c bson/obj_bson_visit.c bson/obj_bson.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_list.c util/obj_math.c util/obj_string.c
test_req_SOURCES = test/test_req.c mem/obj_mem_simple.c mem/obj_mem.c bson/obj_bson.c util/obj_math.c
test_hash_SOURCES = test/test_hash.c util/obj_hash.c util/obj_siphash.c mem/obj_mem_simple.c mem/obj_mem.c
test_sds_SOURCES = test/test_sds.c util/obj_sds.c mem/obj_mem_simple.c mem/obj_mem.c
test_string_SOURCES = test/test_string.c util/obj_string.c
test_array_SOURCES = test/test_array.c util/obj_array.c util/obj_list.c mem/obj_mem_simple.c mem/obj_mem.c
test_expr_SOURCES = test/test_expr.c  bson/obj_bson_iter.c bson/obj_bson_visit.c bson/obj_bson.c expr/obj_expr_parse.c expr/obj_expr.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_array.c util/obj_math.c util/obj_status_with.c util/obj_string.c
test_bcon_SOURCES = test/test_bcon.c bson/obj_bson_iter.c bson/obj_bson_visit.c bson/obj_bson.c bson/obj_bson_bcon.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_math.c util/obj_string.c
test_rbtree_SOURCES = test/test_rbtree.c  util/obj_rbtree.c mem/obj_mem_simple.c mem/obj_mem.c
test_object_pool_SOURCES = test/test_object_pool.c util/obj_object_pool.c util/obj_array.c util/obj_list.c mem/obj_mem_simple.c mem/obj_mem.c 
test_prealloc_map_SOURCES = test/test_prealloc_map.c concurrency/obj_prealloc_map.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_siphash.c
test_lock_SOURCES = test/test_lock.c concurrency/obj_concurrency.c concurrency/obj_lock.c concurrency/obj_locker.c concurrency/obj_prealloc_map.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_siphash.c
test_stringdata_SOURCES = test/test_stringdata.c util/obj_stringdata.c mem/obj_mem_simple.c mem/obj_mem.c
test_nss_SOURCES = test/test_nss.c obj_nss.c mem/obj_mem_simple.c mem/obj_mem.c

all:
	mv -f object ../bin/object
	mv -f test_bson ../bin/test_bson
	mv -f test_req ../bin/test_req
	mv -f test_hash ../bin/test_hash
	mv -f test_sds ../bin/test_sds
	mv -f test_string ../bin/test_string
	mv -f test_array ../bin/test_array
	mv -f test_expr ../bin/test_expr
	mv -f test_bcon ../bin/test_bcon
	mv -f test_rbtree ../bin/test_rbtree
	mv -f test_object_pool ../bin/test_object_pool
	mv -f test_prealloc_map ../bin/test_prealloc_map
	mv -f test_lock ../bin/test_lock
	mv -f test_stringdata ../bin/test_stringdata
	mv -f obj_daemon.o obj_thread.o obj.o bson/*.o concurrency/*.o expr/*.o mem/*.o network/*.o test/*.o util/*.o ../obj
clean:
	rm -f ../bin/object
	rm -f ../bin/test_bson
	rm -f ../bin/test_req
	rm -f ../bin/test_hash
	rm -f ../bin/test_sds
	rm -f ../bin/test_string
	rm -f ../bin/test_array
	rm -f ../bin/test_expr
	rm -f ../bin/test_bcon
	rm -f ../bin/test_rbtree
	rm -f ../bin/test_object_pool
	rm -f ../bin/test_prealloc_map
	rm -f ../bin/test_lock
	rm -f ../bin/test_stringdata
	rm -f ../obj/*
clean-local:
	-rm config.log 
