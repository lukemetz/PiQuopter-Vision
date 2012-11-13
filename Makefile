main: main.cpp
	g++ main.cpp -o main -laruco -I/usr/local/include -L/usr/local/lib/ -laruco -lopencv_core -lopencv_highgui