 #/**************************************
 #**** 2017-4-23      ******************
 #**** Abdulhakim Qahtan ****************
 #**** aqahtan@qf.org.qa ****************
 #***************************************/

CC = g++
DEBUG = -O3 #-D_GLIBCXX_USE_CXX11_ABI=0
CFLAGS = -std=c++11 $(DEBUG) -c
LFLAGS = -std=c++11 $(DEBUG)
OBJS = main.o common.o csv_reader.o Table.o Framework.o 

all: CE

CE : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o CE

main.o : main.cpp common.o csv_reader.o Table.o Framework.o
	$(CC) $(CFLAGS) $< -o $@ 

common.o : common.cpp common.h
	$(CC) $(CFLAGS) $< -o $@
	
csv_reader.o : csv_reader.cpp csv_reader.h
	$(CC) $(CFLAGS) $< -o $@ 

Table.o : Table.cpp Table.h common.h
	$(CC) $(CFLAGS) $< -o $@ 

Framework.o : Framework.cpp Framework.h
	$(CC) $(CFLAGS) $< -o $@ 


clean:
	rm *.o
