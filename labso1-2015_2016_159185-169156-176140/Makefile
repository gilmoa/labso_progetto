CC = gcc
CFLAGS = -Wall
PROJECTNAME = splitsearch

help :
	@echo "*Avaiable make commands:"
	@echo "help    - this page."
	@echo "compile - compile source code to build/."
	@echo "clean   - clean backup files."
	@echo "build   - same as clean and compile."
	@echo "test    - build and run test case."


compile :
	$(CC) $(CFLAGS) src/$(PROJECTNAME).c -o build/$(PROJECTNAME)

.PHONY: clean

clean :
	@rm -f *~ assets/*~ src/*~ build/*
	@echo "Cleaning done."

build : clean compile

test : compile
	build/$(PROJECTNAME) -t 4 -i assets/input04.txt -m 10

t :
	$(CC) $(CFLAGS) src/test.c -o build/test && build/test
