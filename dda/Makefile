GSL_DIR = libs/gsl-install
CFLAGS = -I$(GSL_DIR)/include -I./libs -Wall -ggdb
LDFLAGS = -L$(GSL_DIR)/lib -lgsl -lgslcblas -lm -lpng
EXEC = dda_program

$(EXEC): src/main.c libs/uselibpng.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build: $(EXEC)

run: $(EXEC)
	LD_LIBRARY_PATH=$(GSL_DIR)/lib:$(LD_LIBRARY_PATH) ./$(EXEC) $(filename)

debug: $(EXEC)
	LD_LIBRARY_PATH=$(GSL_DIR)/lib:$(LD_LIBRARY_PATH) gdb ./$(EXEC)

clean:
	rm -f $(EXEC)
