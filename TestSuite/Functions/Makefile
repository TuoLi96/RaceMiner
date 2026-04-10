DIR_INCLUDE=../include
#CFLAGS += -DFIX
ifeq ($(DOFIX), true)
	CFLAGS += -DFIX
endif

src = $(wildcard *.c)
obj = $(src:%.c=%.o)

target = $(patsubst %.c, lib_%.a, $(src))

all: $(target)

$(target):lib_%.a:%.o
	@$(AR) -r $@ $^

%.o:%.c
	@$(CC) -c $< $(CFLAGS) -I $(DIR_INCLUDE) -o $@

.PHONY:all clean
clean:
#	@echo "clean "+$(obj)+$(target)
	@rm -rf  $(obj) $(target) 
