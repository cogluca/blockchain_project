CFLAGS=-g -O0 -std=c89 -I. -pedantic
MATH_LIB=-lm
all: createdir build_dependencies build_node build_user build_app 

SRC_DIR=src
LIB_DIR=include
BUILD_DIR=build
BIN_DIR=bin
createdir: 
	mkdir -p bin 
	mkdir -p $(BUILD_DIR)

clean:
	$(RM) -rf build/*.o
	$(RM) -rf bin/*
	$(RM) -rf *.o

build_dependencies: $(SRC_DIR)/ipc.c $(LIB_DIR)/ipc.h $(SRC_DIR)/transaction_pool.c $(LIB_DIR)/transaction_pool.h $(LIB_DIR)/parameters.h \
					 $(SRC_DIR)/config.c $(LIB_DIR)/config.h $(SRC_DIR)/utils.c $(LIB_DIR)/utils.h $(SRC_DIR)/simulation_stats.c $(LIB_DIR)/simulation_stats.h $(MATH_LIB) 
	$(CC) $(CFLAGS) $(SRC_DIR)/ipc.c -c -o $(BUILD_DIR)/ipc.o 
	$(CC) $(CFLAGS) $(SRC_DIR)/transaction_pool.c -c -o $(BUILD_DIR)/transaction_pool.o $(MATH_LIB)
	$(CC) $(CFLAGS) $(SRC_DIR)/config.c  -c -o $(BUILD_DIR)/config.o
	$(CC) $(CFLAGS) $(SRC_DIR)/utils.c  -c -o $(BUILD_DIR)/utils.o
	$(CC) $(CFLAGS) $(SRC_DIR)/simulation_stats.c  -c -o $(BUILD_DIR)/simulation_stats.o

build_node : $(SRC_DIR)/node.c $(LIB_DIR)/node.h 
	$(CC) $(CFLAGS) -D_GNU_SOURCE $(SRC_DIR)/node.c $(BUILD_DIR)/simulation_stats.o  $(BUILD_DIR)/utils.o $(BUILD_DIR)/ipc.o $(BUILD_DIR)/config.o $(BUILD_DIR)/transaction_pool.o -o $(BIN_DIR)/node $(MATH_LIB)
	
build_user : $(SRC_DIR)/user.c $(LIB_DIR)/user.h
	$(CC) $(CFLAGS) -D_GNU_SOURCE $(SRC_DIR)/user.c $(BUILD_DIR)/simulation_stats.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/ipc.o $(BUILD_DIR)/config.o -o $(BIN_DIR)/user $(MATH_LIB)

build_app : $(SRC_DIR)/main.c $(LIB_DIR)/models.h
	$(CC) $(CFLAGS) $(SRC_DIR)/main.c $(BUILD_DIR)/ipc.o $(BUILD_DIR)/config.o -o $(BIN_DIR)/main 
