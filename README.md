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

In the current setup you will need create an object file for the md5 lib manually (will eventually fix this):

    g++ -std=c++0x -o md5.o -c src/md5.cpp

For execution (link hiredis, md5, libboost_regex):

    databayes$ g++ -std=c++0x src/client.cpp $(pkg-config --cflags --libs redis3m jsoncpp) -g -o dbcli /usr/lib/libhiredis.a /usr/lib/libboost_regex.a ./md5.o
    databayes$ ./dbcli


How does it work?
-----------------

"Databayes" is a probabilistic database.  "Entities" define classes of things that are sets of attributes that may take on different values.  The
"Relation" is the atomic unit which expresses a link between instances of two entities dependent on attribute values.  From the set of relations
probability distributions may be determined that allow for sampling and inference to be applied over sets of entity attributes.


Parser
------

The general parser syntax has the following definition:

 *      (1) ADD REL E1(x1=vx1[, x2=vx2, ..]) E2(y1=vy1[, y2=vy2, ..])
 *      (2) GEN E1[.A_E1] GIVEN E2 [ATTR Ai=Vi[, ...]]
 *      (3) INF E1.A_E1 GIVEN E2 [ATTR Ai=Vi[, ...]]
 *      (4) DEF E1[(x1_type-x1, x2_type-x2, ...)]
 *      (5) LST REL [E1 [E2]]
 *      (6) LST ENT [E1]*
 *      (7) RM REL E1(x_1 [, x_2, ..]) E2(y_1 [, y_2, ..])
 *      (8) RM ENT [E1]*
 *      (9) SET E.A AS V

More details on how to use these to build entities, relations and how to use generative commands to sample.


Examples
--------

### Defining an entity:

When defining an entity then entity-name and attributes with their types.  Entity-names must be unique (ie. not already exist)
and all attrbute types must be valid:

    databayes > def x(y_integer, z_float)
    databayes > def a(b_string, c_integer)
    databayes > def nofields


### Creating a Relation:

When creating a relation you specify entities and optionally any number of entity attributes constrained by value.  Entities
and specified attributes must exist however value constraints are optional:

    databayes > add rel x(a=1, b=2.1) y(c=22, d=0.3)


### Listing Entities:

When listing entities (and in general) a wildcard ("*") for one or more characters may be used:

    databayes > lst ent *   // List all entities
    databayes > lst ent a*  // List all entities beginning with 'a'


### Listing Relations:

When listing relations conditions on attributes may be specified where desired.  Attributes and entities must exist.

    databayes > lst rel * *                     // List all entities
    databayes > lst rel a* b*                   // List all entities
    databayes > lst rel a*(x=20) b*             // List all entities
    databayes > lst rel a*(x=20) b*(y=hello)    // List all entities

### Removing Entities:

Allows client to remove entities from the database:

    databayes > rm ent myent    // removes entity myent, this cascades to all relations containing myent

Care must be taken when using this command since removal will automatically cascade to all relations dependent upon this entity.

### Removing Relations:

Allows client to remove relations from the database - WARNING, this will remove all relations of this type:

    databayes > rm rel x(a=1, b=2.1) y(c=22, d=0.3)

This will remove all relations whose attributes match the assignments.