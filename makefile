CC = gcc
CFLAGS = -I$(IDIR) -Wall -g -c -o
OPTIONS = -o
IDIR = ./src/include
SDIR = ./src
ODIR = ./obj
BDIR = ./bin
TDIR = ./src/test


all: test_hash
test_hash:  $(ODIR)/test_hash.o $(ODIR)/obj_hash.o $(ODIR)/obj_siphash.o $(ODIR)/obj_mem.o $(ODIR)/obj_mem_simple.o
	$(CC) $^ $(OPTIONS) $(BDIR)/test_hash -lpthread
test_mem: $(ODIR)/test_mem.o $(ODIR)/obj_mem.o $(ODIR)/obj_mem_simple.o $(ODIR)/obj_string.o
	$(CC) $^ $(OPTIONS) $(BDIR)/test_mem
test_bson: $(ODIR)/test_bson.o $(ODIR)/obj_mem.o $(ODIR)/obj_mem_simple.o $(ODIR)/obj_string.o $(ODIR)/obj_math.o $(ODIR)/obj_list.o $(ODIR)/obj_bson.o $(ODIR)/obj_bson_validate.o 
	$(CC) $^ $(OPTIONS) $(BDIR)/test_bson

# src/test
$(ODIR)/test_hash.o: $(TDIR)/test_hash.c
	$(CC) $(CFLAGS) $@ $<
$(ODIR)/test_mem.o: $(TDIR)/test_mem.c
	$(CC) $(CFLAGS) $@ $<
$(ODIR)/test_bson.o: $(TDIR)/test_bson.c
	$(CC) $(CFLAGS) $@ $<

$(ODIR)/obj_mem.o: $(SDIR)/mem/obj_mem.c
	$(CC) $(CFLAGS) $@ $<
$(ODIR)/obj_mem_simple.o: $(SDIR)/mem/obj_mem_simple.c
	$(CC) $(CFLAGS) $@ $<
# src/bson/
$(ODIR)/obj_bson.o: $(SDIR)/bson/obj_bson.c
	$(CC) $(CFLAGS) $@ $<
$(ODIR)/obj_bson_validate.o: $(SDIR)/bson/obj_bson_validate.c
	$(CC) $(CFLAGS) $@ $<

# src/util/
$(ODIR)/obj_hash.o: $(SDIR)/util/obj_hash.c
	$(CC) $(CFLAGS) $@ $<
$(ODIR)/obj_siphash.o: $(SDIR)/util/obj_siphash.c
	$(CC) $(CFLAGS) $@ $<
$(ODIR)/obj_string.o: $(SDIR)/util/obj_string.c
	$(CC) $(CFLAGS) $@ $<
$(ODIR)/obj_math.o: $(SDIR)/util/obj_math.c
	$(CC) $(CFLAGS) $@ $<
$(ODIR)/obj_list.o: $(SDIR)/util/obj_list.c
	$(CC) $(CFLAGS) $@ $<

#src/
$(ODIR)/obj.o: $(SDIR)/obj.c
	$(CC) $(CFLAGS) $@ $<

clean:
	rm -f $(ODIR)/*.o $(BDIR)/*

