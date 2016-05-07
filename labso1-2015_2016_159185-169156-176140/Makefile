CC = gcc
PROJECTNAME = splitsearch

help :
	@echo "* Comandi make disponibili:\n"
	@echo "   help    - questa pagine."
	@echo "   compile - compila l'eseguibile in build/."
	@echo "   clean   - rimuove eventuali file di backup ed eseguibili."
	@echo "   build   - come eseguire clean e compile."
	@echo "   test1   - test su file lungo con massimo 10 risultati."
	@echo "   test2   - test descrittivo su file corto."
	@echo "   test3   - test descrittivo su file corto con massimo 2 risultati."
	@echo "   test    - esegue build e tre casi di test."

build/% : src/%.c
	$(CC) $^ -o $@

compile : build/$(PROJECTNAME)

.PHONY: clean

clean :
	@rm -f *~ assets/*~ src/*~ build/*
	@echo "Cleaning done."

build : clean compile

test1 : build/$(PROJECTNAME)
	@echo "\n===Test 1==="
	$^ -t rete -i assets/input_mix_long.txt -m 10

test2 : build/$(PROJECTNAME)
	@echo "\n===Test 2==="
	$^ -t 4 -i assets/input_int_short.txt -v

test3 : build/$(PROJECTNAME)
	@echo "\n===Test 3==="
	$^ -t 4 -i assets/input_int_short.txt -m 2 -v

test : build test1 test2 test3
