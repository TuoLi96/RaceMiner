DOFIX := false

SUB_DIR :=  Race

.PHONY: subdirs $(SUB_DIR)

subdirs: $(SUB_DIR)

$(SUB_DIR):
	    @+make -C $@ DOFIX=$(DOFIX)

clean:$(SUB_DIR_CLEAN) 
		$(foreach N, $(SUB_DIR),make clean -C $(N);)
