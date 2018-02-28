step2_eval: step2_eval.cpp reader.cpp type.cpp
	g++ -o $@ -std=c++17 -g -O0 $^

step1_read_print: step1_read_print.cpp reader.cpp
	g++ -o $@ -std=c++17 -g -O0 $^

step0_repl: step0_repl.cpp
	g++ -o $@ -std=c++17 -g -O0 $^
