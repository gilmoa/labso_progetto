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

clean :
	@rm -f *~
	@rm -f assets/*~
	@rm -f src/*~
	@rm -f build/*~
	@echo "Cleaning done."

build : clean compile

test : compile
	build/$(PROJECTNAME) 4 assets/input03.txt
