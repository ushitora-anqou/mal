step6_file: step6_file.cpp reader.cpp type.cpp
	g++ -o $@ -Wall -std=c++17 -g -O0 $^

step5_tco: step5_tco.cpp reader.cpp type.cpp
	g++ -o $@ -Wall -std=c++17 -g -O0 $^

step4_if_fn_do: step4_if_fn_do.cpp reader.cpp type.cpp
	g++ -o $@ -Wall -std=c++17 -g -O0 $^

step3_env: step3_env.cpp reader.cpp type.cpp
	g++ -o $@ -std=c++17 -g -O0 $^

step2_eval: step2_eval.cpp reader.cpp type.cpp
	g++ -o $@ -std=c++17 -g -O0 $^

step1_read_print: step1_read_print.cpp reader.cpp
	g++ -o $@ -std=c++17 -g -O0 $^

step0_repl: step0_repl.cpp
	g++ -o $@ -std=c++17 -g -O0 $^
