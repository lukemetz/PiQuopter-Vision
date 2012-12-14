default: main.o Controller.o Controller_consts.o
	g++ Controller.o main.o Controller_consts.o -o main -L/usr/local/lib/ -laruco -lopencv_core -lopencv_highgui -lboost_system -lboost_thread -pthread

main.o: main.cpp
	g++ main.cpp -c -laruco -I/usr/local/include -L/usr/local/lib/ -laruco -lopencv_core -lopencv_highgui -lboost_system -lboost_thread -pthread

Controller.o: Controller.cpp Controller.hpp
	g++ Controller.cpp -c -laruco -I/usr/local/include -L/usr/local/lib/ -laruco -lopencv_core -lopencv_highgui -lboost_system -lboost_thread -pthread

Controller_consts.o: Controller_consts.cpp Controller.hpp
	g++ Controller_consts.cpp -c -laruco -I/usr/local/include -L/usr/local/lib/ -laruco -lopencv_core -lopencv_highgui -lboost_system -lboost_thread -pthread
