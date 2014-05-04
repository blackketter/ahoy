LUA_INC= /usr/include/lua5.1

WARN= -Wall -Werror -Wmissing-prototypes -Wmissing-declarations -std=c99 -pedantic
INCS= -I$(LUA_INC)
CFLAGS= -O2 $(WARN) $(INCS) $(DEFS) -fPIC

# OS dependent
LIB_OPTION= -shared #for Linux
#LIB_OPTION= -bundle -undefined dynamic_lookup #for MacOS X

LIBNAME= avahi.so

OBJS= avahi.o
SRCS= avahi.c
AR= ar rcu
RANLIB= ranlib

all: lib

lib: $(LIBNAME)

$(LIBNAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(LIB_OPTION) $(OBJS)
#	$(CC) -shared -O3 $(OBJS) -o $(LIBNAME)

#~ $(COMPAT_DIR)/compat-5.1.o: $(COMPAT_DIR)/compat-5.1.c
#~ $(CC) -c $(CFLAGS) -o $@ $(COMPAT_DIR)/compat-5.1.c

#~ install:
#~ mkdir -p $(LUA_LIBDIR)/avahi
#~ cp src/$(LIBNAME) $(LUA_LIBDIR)/avahi

clean:
	rm -f $(LIBNAME) *.o