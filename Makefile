GSL_PATH ?= /usr
GSL_PATH_LIB ?= $(GSL_PATH)/lib/
CFLAGS =-g -O0 -std=c99 -Wall -Wextra -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -fPIC -g3 -I$(GSL_PATH)/include/ #-ftest-coverage -fprofile-arcs
LDFLAGS = -lm -lgsl -lgslcblas -ldl -L$(GSL_PATH_LIB) -Wl,--rpath=${GSL_PATH_LIB}
COVERAGE_CFLAGS = $(CFLAGS) -fprofile-arcs -ftest-coverage 


all: build 

build: smartayke.so server player_wiwi.so #player_random.so 

server:	src/server.c src/graph.c src/move.c src/dijkstra.c
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

smartayke.so: src/smartayke.c src/player_utils.c src/move.c src/player_mouvement.c src/dijkstra.c src/graph.c
	gcc $(CFLAGS) -shared $^ -o $@ $(LDFLAGS)

# player_random.so: src/player_random.c src/player_utils.c src/move.c src/player_mouvement.c src/graph.c
# 	gcc $(CFLAGS) -shared $^ -fpic -o $@  $(LDFLAGS)

player_wiwi.so: src/player_wiwi.c src/player_utils.c src/move.c src/player_mouvement.c src/dijkstra.c src/graph.c 
	gcc $(CFLAGS) -shared $^ -o $@ $(LDFLAGS)





TEST_LIBS = src/dijkstra.c src/graph.c src/player_utils.c src/player_mouvement.c src/move.c
TEST_SRC = test/main.c test/test_player_mouvement.c test/test_dijkstra.c test/test_graph.c test/test_move.c
TEST_EXEC = alltests

build_tests: $(TEST_SRC) $(TEST_LIBS)
	gcc $(COVERAGE_CFLAGS) -Isrc/ -Itest/ $^ -o $(TEST_EXEC) $(LDFLAGS)

install: build
	@mkdir -p install/
	@cp server install/
	@cp smartayke.so player_wiwi.so install/
	@if [ -f alltests ]; then cp alltests install/; fi
	@for file in test_*; do \
		if [ -f "$$file" ]; then \
			cp "$$file" install/; \
		fi \
	done
	@echo " Installation complète dans le répertoire install/"

test: build_tests
	@./$(TEST_EXEC)

coverage: test
	gcov -n -o . alltests-*.gcno

full-coverage:
	#Clean existing files
	rm -f *.gcno *.gcda
	rm -f server smartayke.so player_wiwi.so
	
	#Compile with coverage flags
	gcc $(COVERAGE_CFLAGS) -shared src/smartayke.c src/player_utils.c src/move.c src/player_mouvement.c src/dijkstra.c src/graph.c -o smartayke.so $(LDFLAGS)
	gcc $(COVERAGE_CFLAGS) src/server.c src/graph.c src/move.c src/dijkstra.c -o server $(LDFLAGS)
	gcc $(COVERAGE_CFLAGS) -shared src/player_wiwi.c src/player_utils.c src/move.c src/player_mouvement.c src/dijkstra.c src/graph.c -o player_wiwi.so $(LDFLAGS)

	cp smartayke.so player_wiwi.so server install/
	./server ./smartayke.so ./player_wiwi.so

	gcc $(COVERAGE_CFLAGS) -Isrc/ -Itest/ $(TEST_LIBS) $(TEST_SRC) -o $(TEST_EXEC) $(LDFLAGS)
	./$(TEST_EXEC)
	
	gcov -n -o . *.gcda

clean:
	@rm -f *~ src/*~ test/*~
	@rm -f install/*
	@rm -rf *.gcno *.gcda *.gcov
	@rm -f *.so server
	@rm -f $(TEST_EXEC)
	
	

.PHONY: build build_tests install test clean

