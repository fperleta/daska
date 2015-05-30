
.PHONY: all daska clean

all:	daska

install:	daska
	rm $$HOME/bin/daska
	cp bin/daska $$HOME/bin/daska

include config.mk

DASKA_OBJS=main ui view
DASKA_MOD_OBJS=clock pipe xmonad

OBJS=$(addsuffix .o, \
     $(addprefix obj/, $(DASKA_OBJS) \
	$(addprefix mod/, $(DASKA_MOD_OBJS))))

SRCS=$(OBJS:obj/%.o=src/%.cc)
DEPS=$(join $(dir $(SRCS)), $(addprefix ., $(notdir $(SRCS:.cc=.d))))

include $(DEPS)

daska:	bin/daska $(DEPS)
bin/daska:	$(OBJS)
	$(CXX) -o bin/daska $(OBJS) $(CFLAGS) $(LIBS)

clean:
	for i in `find obj/ -name "*.o"`; do rm $$i; done

