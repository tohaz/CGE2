env -u WAYLAND_DISPLAY stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t3_label 2>&1 | tee ./valgrind_output_t3_xcb.lst
