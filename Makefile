main.o: main.cpp Controller.cpp Controller.hpp
	g++ main.cpp -c -laruco -I/usr/local/include -L/usr/local/lib/ -laruco -lopencv_core -lopencv_highgui -lboost_system -lboost_thread -pthread

Controller.o: Controller.cpp Controller.hpp
	g++ Controller.cpp -c -laruco -I/usr/local/include -L/usr/local/lib/ -laruco -lopencv_core -lopencv_highgui -lboost_system -lboost_thread -pthread

default: main.o Controller.o
	g++ Controller.cpp main.cpp -o main -L/usr/local/lib/ -laruco -lopencv_core -lopencv_highgui -lboost_system -lboost_thread -pthread