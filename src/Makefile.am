AUTOMAKE_OPTIONS = subdir-objects

noinst_PROGRAMS = object test_bson test_req test_string test_array test_expr test_bcon test_rbtree test_object_pool test_prealloc_map test_lock test_set test_interval

AM_CPPFLAGS = -I$(srcdir)/include

object_SOURCES = obj_daemon.c obj_thread.c obj.c \
				 bson/obj_bson_bcon.c bson/obj_bson_iter.c bson/obj_bson_validate.c bson/obj_bson_visit.c bson/obj_bson.c \
				 command/obj_command.c \
				 concurrency/obj_concurrency.c concurrency/obj_lock.c concurrency/obj_locker.c \
				 engine/catalog/obj_catalog.c engine/index/obj_index.c engine/record/obj_record_store.c engine/type/obj_type.c engine/obj_engine.c \
				 exec/obj_exec_tree.c exec/obj_exec_working_set.c \
				 expr/obj_expr_optimize.c expr/obj_expr_parse.c expr/obj_expr.c \
				 mem/obj_mem_simple.c mem/obj_mem.c \
				 network/obj_buffer.c network/obj_conn.c network/obj_proto.c \
				 query/obj_index_bounds.c query/obj_interval.c query/obj_query_index.c query/obj_query_plan_executor.c query/obj_query_plan_iter.c query/obj_query_plan_tree.c query/obj_query_planner.c query/obj_query.c \
				 util/obj_array.c util/obj_list.c util/obj_math.c util/obj_object_pool.c util/obj_prealloc_map.c util/obj_rbtree.c util/obj_set.c util/obj_siphash.c util/obj_status_with.c util/obj_string.c util/obj_stringdata.c
			
test_bson_SOURCES = test/test_bson.c bson/obj_bson_bcon.c bson/obj_bson_iter.c bson/obj_bson_validate.c bson/obj_bson_visit.c bson/obj_bson.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_list.c util/obj_math.c util/obj_string.c
test_req_SOURCES = test/test_req.c mem/obj_mem_simple.c mem/obj_mem.c bson/obj_bson_iter.c bson/obj_bson_visit.c bson/obj_bson.c bson/obj_bson_bcon.c util/obj_math.c util/obj_string.c
test_string_SOURCES = test/test_string.c util/obj_string.c
test_array_SOURCES = test/test_array.c util/obj_array.c util/obj_list.c mem/obj_mem_simple.c mem/obj_mem.c
test_expr_SOURCES = test/test_expr.c bson/obj_bson_iter.c bson/obj_bson_visit.c bson/obj_bson.c expr/obj_expr_parse.c expr/obj_expr.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_array.c util/obj_math.c util/obj_status_with.c util/obj_string.c
test_bcon_SOURCES = test/test_bcon.c bson/obj_bson_iter.c bson/obj_bson_visit.c bson/obj_bson.c bson/obj_bson_bcon.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_math.c util/obj_string.c
test_rbtree_SOURCES = test/test_rbtree.c  util/obj_rbtree.c mem/obj_mem_simple.c mem/obj_mem.c
test_object_pool_SOURCES = test/test_object_pool.c util/obj_object_pool.c util/obj_array.c util/obj_list.c mem/obj_mem_simple.c mem/obj_mem.c 
test_prealloc_map_SOURCES = test/test_prealloc_map.c util/obj_prealloc_map.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_siphash.c
test_lock_SOURCES = test/test_lock.c concurrency/obj_concurrency.c concurrency/obj_lock.c concurrency/obj_locker.c util/obj_prealloc_map.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_siphash.c
#test_stringdata_SOURCES = test/test_stringdata.c util/obj_stringdata.c mem/obj_mem_simple.c mem/obj_mem.c
#test_index_key_SOURCES = test/test_index_key.c storage/index/obj_index_key.c storage/v1/index/obj_v1_index_key.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_string.c util/obj_math.c bson/obj_bson.c bson/obj_bson_iter.c bson/obj_bson_visit.c bson/obj_bson_bcon.c
test_set_SOURCES = test/test_set.c util/obj_set.c util/obj_prealloc_map.c mem/obj_mem_simple.c mem/obj_mem.c util/obj_siphash.c
#test_query_SOURCES = test/test_query.c query/obj_query.c query/obj_query_index.c query/obj_query_plan_iter.c query/obj_query_planner.c query/obj_query_plan_tree.c query/obj_interval.c query/obj_index_bounds.c query/obj_query_plan_executor.c exec/obj_exec_tree.c exec/obj_exec_working_set.c mem/obj_mem_simple.c mem/obj_mem.c bson/obj_bson_iter.c bson/obj_bson.c bson/obj_bson_visit.c bson/obj_bson_bcon.c util/obj_array.c util/obj_siphash.c util/obj_prealloc_map.c util/obj_set.c util/obj_math.c util/obj_string.c util/obj_status_with.c expr/obj_expr_parse.c expr/obj_expr.c expr/obj_expr_optimize.c engine/catalog/obj_catalog.c engine/index/obj_index.c engine/record/obj_record_store.c engine/type/obj_type.c 
test_interval_SOURCES = test/test_interval.c query/obj_interval.c mem/obj_mem_simple.c mem/obj_mem.c bson/obj_bson_iter.c bson/obj_bson.c bson/obj_bson_visit.c bson/obj_bson_bcon.c util/obj_array.c util/obj_math.c util/obj_string.c


all:
	mv -f object ../bin/object
	mv -f test_bson ../bin/test_bson
	mv -f test_req ../bin/test_req
	mv -f test_string ../bin/test_string
	mv -f test_array ../bin/test_array
	mv -f test_expr ../bin/test_expr
	mv -f test_bcon ../bin/test_bcon
	mv -f test_rbtree ../bin/test_rbtree
	mv -f test_object_pool ../bin/test_object_pool
	mv -f test_prealloc_map ../bin/test_prealloc_map
	mv -f test_lock ../bin/test_lock
	mv -f test_set ../bin/test_set
	mv -f test_interval ../bin/test_interval
	mv -f obj_daemon.o obj_thread.o obj.o bson/*.o concurrency/*.o engine/catalog/*.o engine/index/*.o engine/record/*.o engine/type/*.o exec/*.o expr/*.o mem/*.o network/*.o query/*.o test/*.o util/*.o ../obj
clean:
	rm -f ../bin/object
	rm -f ../bin/test_bson
	rm -f ../bin/test_req
	rm -f ../bin/test_string
	rm -f ../bin/test_array
	rm -f ../bin/test_expr
	rm -f ../bin/test_bcon
	rm -f ../bin/test_rbtree
	rm -f ../bin/test_object_pool
	rm -f ../bin/test_prealloc_map
	rm -f ../bin/test_lock
	rm -f ../bin/test_index_key
	rm -f ../bin/test_set
	rm -f ../bin/test_interval
	rm -f ../obj/*
clean-local:
	-rm config.log 
