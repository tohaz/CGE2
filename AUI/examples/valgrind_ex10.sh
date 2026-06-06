make
stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex10_popup_menu 2>&1 | tee valgrind_output_ex10.lst

