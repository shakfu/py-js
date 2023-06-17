g++ --std=c++17 \
	-O2 \
	-Wall \
	-Wfatal-errors \
	-Wno-sign-compare \
	-Wno-unused-variable \
	-fno-rtti \
	-stdlib=libc++ \
	-I../.. \
	-o main \
	main.cpp

