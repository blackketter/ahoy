LUA_INC= /usr/include/lua5.1

#~ WARN= -Wall -Wmissing-prototypes -Wmissing-declarations -ansi -pedantic
WARN= 
INCS= -I$(LUA_INC)
CFLAGS= -O2 $(WARN) $(INCS) $(DEFS) -fPIC

# OS dependent
LIB_OPTION= -shared #for Linux
#LIB_OPTION= -bundle -undefined dynamic_lookup #for MacOS X

LIBNAME= i2c.so

OBJS= i2c.o
SRCS= i2c.c
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
#~ mkdir -p $(LUA_LIBDIR)/i2c
#~ cp src/$(LIBNAME) $(LUA_LIBDIR)/i2c

clean:
	rm -f $(LIBNAME) *.o