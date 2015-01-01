### This Makefile was written for GNU Make. ###
TERMUTIL_INCS       = /I$(TERMUTIL_DIR)/include/
TERMUTIL_DIR        = TermUtil
TERMUTIL_REPOSITORY = https://github.com/koturn/$(TERMUTIL_DIR).git
TERMUTIL_LIBS_DIR   = $(TERMUTIL_DIR)/lib
TERMUTIL_LDLIBS     = -L./$(TERMUTIL_LIBS_DIR) -ltermutil
TERMUTIL_INCS       = -I./$(TERMUTIL_DIR)/include/

ifeq ($(DEBUG),true)
    COPTFLAGS  := -O0 -g3 -ftrapv -fstack-protector-all -D_FORTIFY_SOURCE=2
    LDLIBS     += -lssp
else
ifeq ($(OPT),true)
    COPTFLAGS  := -flto -Ofast -march=native -DNDEBUG
    LDOPTFLAGS := -flto -Ofast -s
else
ifeq ($(LTO),true)
    COPTFLAGS  := -flto -DNDEBUG
    LDOPTFLAGS := -flto
else
    COPTFLAGS  := -O3 -DNDEBUG
    LDOPTFLAGS := -O3 -s
endif
endif
endif

C_WARNING_FLAGS := -Wall -Wextra -Wformat=2 -Wstrict-aliasing=2 \
                   -Wcast-align -Wcast-qual -Wconversion \
                   -Wfloat-equal -Wpointer-arith -Wswitch-enum \
                   -Wwrite-strings -pedantic

CC       := gcc
MAKE     := make
GIT      := git
INCS     := $(TERMUTIL_INCS)
CFLAGS   := -pipe $(C_WARNING_FLAGS) $(COPTFLAGS) $(INCS)
LDFLAGS  := -pipe $(LDOPTFLAGS)
LDLIBS   := $(TERMUTIL_LDLIBS)
TARGET   := minesweeper
OBJ      := $(addsuffix .o, $(basename $(TARGET)))
SRC      := $(OBJ:%.o=%.c)

ifeq ($(OS),Windows_NT)
    TARGET := $(addsuffix .exe, $(TARGET))
endif

%.exe:
	$(CC) $(LDFLAGS) $(filter %.c %.o, $^) $(LDLIBS) -o $@
%.out:
	$(CC) $(LDFLAGS) $(filter %.c %.o, $^) $(LDLIBS) -o $@


.PHONY: all
all: $(TERMUTIL_LIBS_DIR)/libtermutil.a $(TARGET)

$(TARGET): $(OBJ)

$(OBJ): $(SRC)

$(TERMUTIL_LIBS_DIR)/libtermutil.a:
	@if [ ! -d $(dir $@) ]; then \
		$(GIT) clone $(TERMUTIL_REPOSITORY); \
	fi
	$(MAKE) -C $(TERMUTIL_DIR)


.PHONY: test
test:
	./$(TARGET) -h


.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJ)
.PHONY: cleanobj
cleanobj:
	$(RM) $(OBJ)
