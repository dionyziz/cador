cador : cador.o config.o error.o network.o string.o vilundo.o user.o room.o extensions
	g++ -O0 -g cador.o config.o error.o network.o string.o vilundo.o user.o excalibur.o dummy.o \
		room.o -L/usr/lib -lpcrecpp -lmysqlclient -lxerces-c -lz -o $@
test : config.o error.o network.o string.o vilundo.o user.o room.o test.o extensions
	g++ -O0 -g config.o error.o network.o string.o vilundo.o user.o excalibur.o dummy.o room.o \
		test.o -L/usr/lib -lpcrecpp -lmysqlclient -lxerces-c -lz -o $@
extensions : extensions/dummy.cpp extensions/excalibur.cpp extensions/dummy.h extensions/excalibur.h extension.h
	g++ -O0 -g -c extensions/dummy.cpp
	g++ -O0 -g -c extensions/excalibur.cpp -I/usr/include/mysql
cador.o : cador.cpp
	g++ -O0 -g -c cador.cpp -I/usr/include/mysql
config.o : config.cpp
	g++ -O0 -g -c $?
error.o : error.cpp
	g++ -O0 -g -c $?
network.o : network.cpp
	g++ -O0 -g -c $?
string.o : string.cpp
	g++ -O0 -g -c $?
vilundo.o : vilundo.cpp vilundo.h
	g++ -O0 -g -c vilundo.cpp -I/usr/include/mysql
test.o : test.cpp
	g++ -O0 -g -c $?
user.o : user.cpp
	g++ -O0 -g -c $?
room.o : room.cpp
	g++ -O0 -g -c $? -I/usr/include/mysql
cadortest : cadortest.cpp
	g++ -O0 -g $? -o $@
clean :
	-rm cador test *.o *~

