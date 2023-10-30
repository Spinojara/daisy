CC        = cc
CSTANDARD = -std=c99
CWARNINGS = -Wall -Wextra -Wshadow -pedantic
ARCH      = native
COPTIMIZE = -O2 -march=$(ARCH) -flto

ifeq ($(DEBUG), 1)
	CDEBUG = -g3 -ggdb
else ifeq ($(DEBUG), 2)
	CDEBUG = -g3 -ggdb -fsanitize=address,undefined
else ifeq ($(DEBUG), 3)
	CDEBUG = -g3 -ggdb -fsanitize=address,undefined
	COPTIMIZE =
else
	CDEBUG = -DNDEBUG
endif

CFLAGS = $(CSTANDARD) $(CWARNINGS) $(COPTIMIZE) $(CDEBUG)
LDFLAGS = $(CFLAGS) -lm

SRC = daisy.c deck.c search.c truecount.c
OBJ = $(addprefix obj/,$(addsuffix .o,$(basename $(SRC))))
DEP = $(addprefix dep/,$(addsuffix .d,$(basename $(notdir $(wildcard src/*.c)))))

all: daisy

daisy: $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

obj/%.o: src/%.c dep/%.d Makefile
	@mkdir -p obj
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

dep/%.d: src/%.c
	@mkdir -p dep
	@$(CC) -MM -MT "$(<:src/%.c=obj/%.o)" $(CFLAGS) -Iinclude $< -o $@

clean:
	rm -rf obj dep

-include $(DEP)

.PHONY: all clean
