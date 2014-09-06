LUA_INC= /usr/include/lua5.1

WARN= -Wall -Werror -Wmissing-prototypes -Wmissing-declarations -std=c99 -pedantic
INCS= -I$(LUA_INC)
CFLAGS= -O2 $(WARN) $(INCS) $(DEFS) -fpic -ggdb3  -Wl,--no-as-needed 

# OS dependent
LIB_OPTION= -shared -lavahi-client  -lavahi-common  

LIBNAME= avahi.so

OBJS= avahi.o 
SRCS= avahi.c
AR= ar rcu
RANLIB= ranlib

all: lib

lib: $(LIBNAME)

$(LIBNAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(LIB_OPTION) $(OBJS)

clean:
	rm -f $(LIBNAME) *.o
