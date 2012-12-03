main: main.cpp Controller.cpp Controller.hpp server.cpp server.hpp
	g++ main.cpp Controller.cpp server.cpp -o main -laruco -I/usr/local/include -L/usr/local/lib/ -laruco -lopencv_core -lopencv_highgui -lboost_system -lboost_thread -pthread

client: client.cpp
	g++ client.cpp -lboost_system -lboost_thread -o client -pthread