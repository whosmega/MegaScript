CC = gcc 
CFLAGS = -O2

ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
EXE = mega.exe
RM = del
MV = move
CLIBS = -lm
DLLEXT = dll
DYNAMIC_FLG = 
else
EXE = mega
RM = rm
MV = mv
CLIBS = -lm -ldl -lssl -lcrypto
DLLEXT = so
DYNAMIC_FLG = -export-dynamic
endif

BIN =  chunk.o debug.o globals.o memory.o \
	   scanner.o value.o compiler.o gcollect.o \
	   main.o object.o table.o vm.o \
	    
LIB_BIN = _socket.o _ssocket.o _coroutine.o

DLLS = _socket.$(DLLEXT) _ssocket.$(DLLEXT) _coroutine.$(DLLEXT)

$(EXE) $(DLLS): $(BIN) $(LIB_BIN)
	$(CC) $(CLIBS) $(CFLAGS) $(DYNAMIC_FLG) $(BIN) -o $(EXE)
	$(CC) $(CFLAGS) -shared _socket.o -o _socket.$(DLLEXT) 
	$(CC) $(CFLAGS) -shared -lssl -lcrypto _ssocket.o -o _ssocket.$(DLLEXT)
	$(CC) $(CFLAGS) -shared _coroutine.o -o _coroutine.$(DLLEXT)
	$(MV) *.o bin
	$(MV) *.$(DLLEXT) lib

chunk.o : includes/chunk.h includes/memory.h includes/value.h \
		  src/chunk.c
	$(CC) $(CFLAGS) -c src/chunk.c

debug.o : includes/debug.h includes/value.h includes/object.h \
		  src/debug.c 
	$(CC) $(CFLAGS) -c src/debug.c 

globals.o : includes/globals.h includes/object.h includes/value.h \
			src/globals.c 
	$(CC) $(CFLAGS) -c src/globals.c 

memory.o : includes/memory.h includes/gcollect.h includes/debug.h \
		   src/memory.c 
	$(CC) $(CFLAGS) -c src/memory.c 

scanner.o : includes/scanner.h includes/common.h \
			src/scanner.c 
	$(CC) $(CFLAGS) -c src/scanner.c 

value.o : includes/value.h includes/memory.h includes/object.h \
		  src/value.c
	$(CC) $(CFLAGS) -c src/value.c 

compiler.o : includes/compiler.h includes/chunk.h includes/scanner.h \
			 includes/vm.h includes/debug.h includes/value.h includes/object.h \
			 includes/memory.h \
			 src/compiler.c 
	$(CC) $(CFLAGS) -c src/compiler.c 

gcollect.o : includes/gcollect.h includes/debug.h includes/memory.h \
			 src/gcollect.c 
	$(CC) $(CFLAGS) -c src/gcollect.c 

main.o : includes/common.h includes/chunk.h includes/debug.h includes/vm.h \
		 includes/compiler.h includes/table.h includes/object.h \
		 src/main.c 
	$(CC) $(CFLAGS) -c src/main.c 

object.o : includes/object.h includes/memory.h includes/common.h includes/value.h \
		   includes/vm.h includes/table.h includes/debug.h  \
		   src/object.c 
	$(CC) $(CFLAGS) -c src/object.c 

table.o : includes/table.h includes/memory.h includes/object.h includes/value.h \
		  src/table.c 
	$(CC) $(CFLAGS) -c src/table.c 

vm.o : includes/vm.h includes/chunk.h includes/common.h includes/debug.h \
	   includes/object.h includes/value.h includes/table.h includes/globals.h \
	   includes/memory.h includes/compiler.h\
	   src/vm.c 
	$(CC) $(CFLAGS) -c src/vm.c 

# libraries 

_socket.o : includes/vm.h includes/memory.h includes/msapi.h \
	core/_socket.c 
	$(CC) $(CFLAGS) -fpic -c core/_socket.c 

_ssocket.o : includes/msapi.h includes/memory.h includes/lib_ssocket.h \
	core/_ssocket.c 
	$(CC) $(CFLAGS) -fpic -c core/_ssocket.c 

_coroutine.o : includes/msapi.h includes/object.h \
	core/_coroutine.c
	$(CC) $(CFLAGS) -fpic -c core/_coroutine.c

clean:
	$(RM) bin/*.o
	$(RM) lib/*.$(DLLEXT)
