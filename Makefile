MM: main.o marketMaker.o dataType.o
	g++ -Wall main.o marketMaker.o dataType.o -o MM
main.o: main.cpp marketMaker.h 
	g++ -Wall -c main.cpp marketMaker.h dataType.h
marketMaker.o: marketMaker.cpp 
	g++ -Wall -c marketMaker.cpp dataType.h 
dataType.o: dataType.cpp dataType.h
	g++ -Wall -c dataType.cpp
clean:
	rm *.o 
