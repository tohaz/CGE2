make
stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex13_app 2>&1 | tee valgrind_output_ex13.lst

