valgrind --tool=memcheck \
		 --dsymutil=yes \
		 --track-origins=yes \
		 --show-leak-kinds=all \
		 --trace-children=yes \
		 --suppressions=$HOME/.valgrind-python.supp \
 		 --leak-check=full $1

		 # --gen-suppressions=yes \

