CC = gcc
PROJECTNAME = splitsearch

help :
	@echo "* Comandi make disponibili:"
	@echo "help    - questa pagine."
	@echo "compile - compila l'eseguibile in build/."
	@echo "clean   - rimuove eventuali file di backup ed eseguibili."
	@echo "build   - come eseguire clean e compile."
	@echo "test    - esegue build e esegue un caso di test."


compile :
	$(CC) src/$(PROJECTNAME).c -o build/$(PROJECTNAME)

.PHONY: clean

clean :
	@rm -f *~ assets/*~ src/*~ build/*
	@echo "Cleaning done."

build : clean compile

test : compile
	build/$(PROJECTNAME) -t 4 -i assets/input04.txt -m 10
