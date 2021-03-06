TARGETS = extract-spfeatures 
SOURCES = extract-spfeatures.cc heads.cc read-tree.cc sym.cc
OBJECTS = $(patsubst %.l,%.o,$(patsubst %.c,%.o,$(SOURCES:%.cc=%.o)))

#CPPFLAGS=-g -pg -O0

all: $(TARGETS)

extract-spfeatures: extract-spfeatures.o heads.o read-tree.o sym.o spfeatures.h
	$(CXX) $(LDFLAGS) $^ -o $@

read-tree.cc: read-tree.l
	flex -oread-tree.cc read-tree.l

.PHONY: 
clean: 
	rm -fr *.o *.d *~ core read-tree.cc $(TARGETS)

.PHONY: real-clean
real-clean: clean 
	rm -fr $(TARGETS)

# this command tells GNU make to look for dependencies in *.d files
-include $(patsubst %.l,%.d,$(patsubst %.c,%.d,$(SOURCES:%.cc=%.d)))
