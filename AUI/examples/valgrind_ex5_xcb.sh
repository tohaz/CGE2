make
env -u WAYLAND_DISPLAY stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex5_list 2>&1 | tee valgrind_output_ex5.lst

