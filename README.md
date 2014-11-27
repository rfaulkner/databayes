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

    databayes$ g++ -std=c++0x src/client.cpp $(pkg-config --cflags --libs redis3m jsoncpp) -g -o dbcli /usr/lib/libhiredis.a /usr/lib/libboost_regex.a
    databayes$ ./dbcli


Parser
------

The general parser syntax has the following definition:

 *      (1) ADD REL E1(x_1 [, x_2, ..]) E2(y_1 [, y_2, ..])
 *      (2) GEN REL E1[(x_1=v_1, x_2=v_2, ...)] CONSTRAIN E2[, E3, ...]
 *      (3) DEF E1[(x_1, x_2, ...)]
 *      (4) LST REL [E1 [E2]]
 *      (5) LST ENT [E1]*

More details on how to use these to build entities, relations and how to use generative commands to sample.


Examples
--------

Defining an entity:
~~~~~~~~~~~~~~~~~~~

When defining an entity then entity-name and attributes with their types.  Entity-names must be unique (ie. not already exist)
and all attrbute types must be valid:

    databayes > def x(y_integer, z_float)
    databayes > def a(b_string, c_integer)
    databayes > def nofields


Creating a Relation:
~~~~~~~~~~~~~~~~~~~~

When creating a relation you specify entities and optionally any number of entity attributes constrained by value.  Entities
and specified attributes must exist however value constraints are optional:

    databayes > add rel x(a=1, b_float) y(c_integer, d_float)
    databayes > add rel x(a_integer, b_float) y(c_integer, d_float)


Listing Entities:
~~~~~~~~~~~~~~~~~

When listing entities (and in general) a wildcard ("*") for one or more characters may be used:

    lst ent *   // List all entities
    lst ent a*  // List all entities beginning with 'a'


Listing Relations:
~~~~~~~~~~~~~~~~~~

When listing relations conditions on attributes may be specified where desired.  Attributes and entities must exist.

    lst rel * *                     // List all entities
    lst rel a* b*                   // List all entities
    lst rel a*(x=20) b*             // List all entities
    lst rel a*(x=20) b*(y=hello)    // List all entities