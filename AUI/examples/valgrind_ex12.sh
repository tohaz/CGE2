make
stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex12_progressbar 2>&1 | tee valgrind_output_ex12.lst

