.PHONY: all
.PHONY: clean

UNAME     = $(shell uname)
LINKFLAGS = -g -ggdb -lm
CFLAGS    = -Wall -pedantic -I. -I.. -g -ggdb -DDEBUG

ifeq ($(UNAME), QNX)
	PLATTFORM = QNX
endif

ifeq ($(UNAME), Linux)
	PLATTFORM = LINUX
endif

ifneq (,$(findstring BSD,$(UNAME)))
	PLATTFORM = BSD
endif

ifneq (,$(findstring MINGW,$(UNAME)))
	PLATTFORM = WINDOWS
endif

ifneq (,$(findstring CYGWIN,$(UNAME)))
	PLATTFORM = WINDOWS
endif

ifeq ($(PLATTFORM), BSD)
	CFLAGS    += -pthread
	LINKFLAGS += -pthread -lrt
endif

ifeq ($(PLATTFORM), LINUX)
	CFLAGS    += -pthread
	LINKFLAGS += -pthread -lrt
endif

ifeq ($(PLATTFORM), WINDOWS)
	CFLAGS    += 
	LINKFLAGS += -L. -I./include -lpthreadGC2
endif

EXE_PROGS = answer
SRC_PROGS = $(EXE_PROGS:%=%.c)
OBJ_PROGS = $(EXE_PROGS:%=%.o)

SRC_NMQUEUE = $(wildcard src/*.c)
OBJ_NMQUEUE = $(SRC_NMQUEUE:%.c=%.o)

SRC_TOOLS = $(wildcard ../tools/*.c)
OBJ_TOOLS = $(SRC_TOOLS:%.c=%.o)

SRC_TESTS = $(wildcard tests/*.c)
OBJ_TESTS = $(SRC_TESTS:%.c=%.o)
EXE_TESTS = $(SRC_TESTS:%.c=%)

all: $(EXE_TESTS) $(EXE_PROGS)

clean:
	$(RM) $(OBJ_NMQUEUE)
	$(RM) $(OBJ_TESTS)
	$(RM) $(EXE_TESTS)
	$(RM) $(OBJ_TOOLS)
	$(RM) $(EXE_PROGS)
	$(RM) $(OBJ_PROGS)

$(EXE_TESTS) $(EXE_PROGS):%:%.c $(OBJ_NMQUEUE) $(OBJ_TOOLS)
	$(CC) -c $@.c -o $@.o $(CFLAGS)
	$(CC) -o $@ $(OBJ_NMQUEUE) $(OBJ_TOOLS) $@.o $(LINKFLAGS)

$(OBJ_NMQUEUE):%.o:%.c %.h
	$(CC) -c $< -o $@ $(CFLAGS)
