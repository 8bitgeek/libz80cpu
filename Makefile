# Build Target
TARGET=libz80cpu.a

# GCC toolchain programs.
CPP = ${LIBZ80_GCC}g++
AR = ${LIBZ80_GCC}ar

# C compilation directives
CFLAGS += -c
CFLAGS += -Wall
CFLAGS += ${LIBZ80_CFLAGS}

# Archiver Flags.
AFLAGS += rcs 

# Header file directories.
INCLUDE += -I inc

# Z/80 CPU Source Files
CPP_SRC  += src/sim80io.cpp
CPP_SRC  += src/sim80mem.cpp
CPP_SRC  += src/sim80vm_i8080.cpp
CPP_SRC  += src/sim80vm_z80a.cpp
CPP_SRC  += src/sim80vm.cpp


# Object files to build.
OBJS += $(CPP_SRC:.cpp=.o)

# Default rule to build the whole project.
.PHONY: all
all: $(TARGET)

# Rule to compile C files.
%.o: %.cpp
	$(CPP) $(CFLAGS) $(INCLUDE) $< -o $@

# Rule to create an ELF file from the compiled object files.
$(TARGET): $(OBJS)
	$(AR) $(AFLAGS) $@  $^ 


# Rule to clear out generated build files.
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

