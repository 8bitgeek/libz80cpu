# Build Target
TARGET=libz80cpu.a

# GCC toolchain programs.
CC = ${LIBZ80_GCC}gcc
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
C_SRC  += src/cpu.c

# Object files to build.
OBJS  = $(AS_SRC:.S=.o)
OBJS += $(C_SRC:.c=.o)

# Default rule to build the whole project.
.PHONY: all
all: $(TARGET)

# Rule to build assembly files.
%.o: %.S
	$(CC) -x assembler-with-cpp $(ASFLAGS) $(INCLUDE) $< -o $@

# Rule to compile C files.
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

# Rule to create an ELF file from the compiled object files.
$(TARGET): $(OBJS)
	$(AR) $(AFLAGS) $@  $^ 


# Rule to clear out generated build files.
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

