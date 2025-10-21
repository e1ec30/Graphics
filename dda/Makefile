GSL_DIR = libs/gsl-install
CFLAGS = -I$(GSL_DIR)/include -Wall -g
LDFLAGS = -L$(GSL_DIR)/lib -lgsl -lgslcblas -lm
EXEC = dda_program

$(EXEC): src/main.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(EXEC)
	LD_LIBRARY_PATH=$(GSL_DIR)/lib:$(LD_LIBRARY_PATH) ./$(EXEC)

clean:
	rm -f $(EXEC)
