make
stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t1_aui 2>&1 | tee valgrind_output_t1.lst

