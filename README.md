databayes
=========

Probabilistic Database that uses bayesian inference to build up relations.

Install C++ redis client redis3m & jsoncpp.  Follow the instructions at https://github.com/luca3m/redis3m.

Repeat for https://github.com/open-source-parsers/jsoncpp.

Install C redis client https://github.com/redis/hiredis release 0.11.0.
 

Installation
------------

(Not yet implemented)

	git clone https://github.com/rfaulkner/databayes
	cd databayes
	cmake \.
	make
	sudo make install


Setup
-----

Ensure that you LD_LIBRARY_PATH is set:

    export LD_LIBRARY_PATH=/usr/local/lib

Use the following compiler flags including the redis3m and jsoncpp libraries:

    -std=c++0x $(pkg-config --cflags --libs redis3m jsoncpp)

For execution (link hiredis):

    databayes$ g++ -std=c++0x src/client.cpp $(pkg-config --cflags --libs redis3m jsoncpp) -g -o dbcli /usr/lib/libhiredis.a
    databayes$ ./dbcli


Parser
------

The general parser syntax has the following definition:

 *      (1) ADD REL E1(x_1 [, x_2, ..]) E2(y_1 [, y_2, ..])
 *      (2) GET REL E1[(x_1=v_1, x_2=v_2, ...)] [E2(y_1=u_1, y_2=u_2, ...)]
 *      (3) GEN REL E1[(x_1=v_1, x_2=v_2, ...)] CONSTRAIN E2[, E3, ...]
 *      (4) DEF E1[(x_1, x_2, ...)]
 *      (5) LST REL [E1 [E2]]
 *      (6) LST ENT [E1]*

More details on how to use these to build entities, relations and how to use generative commands to sample.