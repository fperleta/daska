
# common variables
SHELL=/bin/sh
CXX=g++
CPP=g++ -E
PKGS=xcb cairo-xcb pangocairo
HEAD="`git rev-parse --short HEAD` `date -Iseconds`"
CCFLAGS=-W -Wall -std=gnu++14 -pthread -g -Og -DDEBUG \
	$(PKGS:%=`pkg-config --cflags %`) \
	-DDASKA_HEAD=\"$(HEAD)\"
CPPFLAGS=-iquote src/
LIBS=-L/usr/local/lib -lm -lev $(PKGS:%=`pkg-config --libs %`)

# dependency extraction for C files
.%.d: %.cc
	@ $(SHELL) -ec '$(CPP) $(CCFLAGS) $(CPPFLAGS) -MM $< | sed "s|$.*.o|& $@|g" > $@'

obj/%.o: src/%.cc
	$(CXX) -o $@ -c $< $(CCFLAGS) $(CPPFLAGS)

