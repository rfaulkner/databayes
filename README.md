databayes
=========

Probabilistic Database that uses bayesian inference to build up relations.

Install C++ redis client hiredis & jsoncpp.  Follow the instructions at https://github.com/redis/hiredis (release 0.11.0).

Repeat for https://github.com/open-source-parsers/jsoncpp.


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

Use the following compiler flags including the jsoncpp library:

    -std=c++0x $(pkg-config --cflags --libs jsoncpp)

In the current setup you will need create an object file for the md5 lib manually (will eventually fix this):

    g++ -std=c++0x -o md5.o -c src/md5.cpp

For execution (link hiredis, md5, libboost_regex):

    databayes$ g++ -std=c++0x src/client.cpp $(pkg-config --cflags --libs jsoncpp) -g -o dbcli /usr/lib/libhiredis.a /usr/lib/libboost_regex.a ./md5.o
    databayes$ ./dbcli


How does it work?
-----------------

"Databayes" is a probabilistic database.  "Entities" define classes of things that are sets of attributes that may take on different values.  The
"Relation" is the atomic unit which expresses a link between instances of two entities dependent on attribute values.  From the set of relations
probability distributions may be determined that allow for sampling and inference to be applied over sets of entity attributes.


Parser
------

The general parser syntax has the following definition:

 Implements an SLR parser. Valid Statements:

    (1) ADD REL E1(x1=vx1[, x2=vx2, ..]) E2(y1=vy1[, y2=vy2, ..])
    (2) GEN E1[.A_E1] GIVEN E2 [ATTR Ai=Vi[, ...]]
    (3) INF E1.A_E1 GIVEN E2 [ATTR Ai=Vi[, ...]]
    (4) DEF E1[(x1_type-x1, x2_type-x2, ...)]
    (5) LST REL [E1 [E2]]
    (6) LST ENT [E1]*
    (7) RM REL E1(x_1 [, x_2, ..]) E2(y_1 [, y_2, ..])
    (8) RM ENT [E1]*
    (9) SET E.A FOR E1(x1=vx1[, x2=vx2, ..]) E2(y1=vy1[, y2=vy2, ..]) AS V

1. provides a facility for insertion into the system
2. generate a sample conditional on a set of constraints
3. infer an expected value for an attribute
4. define a new entity
5. list relations optionally dependent relational entities
6. list entities.  Either specify them or simply list all.
7. remove a relation
8. remove an entity
9. set an attribute value

More details on how to use these to build entities, relations and how to use generative commands to sample.


Filtering
---------

The index provides a means to filter a set of relations via the following method:

    void IndexHandler::filterRelations(std::vector<Relation>& relations, AttributeBucket& filterAttrs, std::string comparator)

Given a set of relations and a bucket of attributes each relation in the set is compared against each attribute in the
bucket.  Only if the relation contains the given attribute is a comparison made.  If the attribute does exist in the relation,
then the value is compared to that of the bucket attribute given the comparator passed.

Comparators are defined by the following string values:

    #define ATTR_TUPLE_COMPARE_EQ "="
    #define ATTR_TUPLE_COMPARE_LT "<"
    #define ATTR_TUPLE_COMPARE_GT ">"
    #define ATTR_TUPLE_COMPARE_LTE "<="
    #define ATTR_TUPLE_COMPARE_GTE ">="
    #define ATTR_TUPLE_COMPARE_NE "!="

Only if the relation returns true on compare for all bucket attributes is the relation retained in the filtered list.  The
reference to the relations vector is mutated accordingly.


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


Functionality Checklist
-----------------------

 - [x] CLI: Token Parsing
 - [x] CLI: Parsing Loop to process input commands
 - [ ] CLI: Cursor navigation with arrow keys
 - [ ] CLI: Regenerate Last Command
 - [ ] CLI: Handle arbitrary amounts of whitespace
 - [x] Parser Modelling: Parser class to maintain internal state of command execution
 - [x] Parser Modelling: Command codes and State Modeling
 - [x] Parser Modelling: Error codes and Error handling Login
 - [x] Parser Modelling: Termination loop to handle processing of parsed command by calling the index
 - [x] Parser Command: Defining Entities
 - [x] Parser Command: Adding Relations
 - [x] Parser Command: Removing Entities
 - [x] Parser Command: Removing Relations
 - [x] Parser Command: Setting Attribute Values
 - [x] Parser Command: Listing Existing Entities
 - [x] Parser Command: Listing Existing Relations
 - [x] Parser Command: Generating Relation Samples
 - [ ] Parser Command: Inferring Expected Value of Relation Samples
 - [x] Object Modeling: Entities
 - [x] Object Modeling: Relations
 - [x] Object Modeling: Attributes
 - [x] Object Modeling: Attribute based collections
 - [x] Object Modeling: JSON Representation of Entities
 - [x] Object Modeling: JSON Representation of Relations
 - [x] Object Modeling: JSON Representation of Relations with type data
 - [x] Object Modeling: Mapping among JSON and object model representations for entities
 - [x] Object Modeling: Mapping among JSON and object model representations for relations
 - [x] Object Modeling: Define attribute types
 - [x] Object Modeling: Attribute type validation
 - [x] Object Modeling: Counts for relations
 - [x] Storage Modeling: Entity Storage
 - [x] Storage Modeling: Relation Storage
 - [x] Storage Modeling: Type Storage
 - [x] Storage Modeling: Index to handle mapping, filtering, extracting relation & entity data from memory to the runtime
 - [x] Storage Modeling: Redis interface for in memory storage
 - [ ] Storage Modeling: Disk Storage model
 - [ ] Storage Modeling: Swapping logic for disk storage
 - [ ] Storage Modeling: Cascading removal of Relations when Entities are removed (NEEDS TESTING)
 - [x] Probabilistic Modeling: Generate Marginal Distributions
 - [x] Probabilistic Modeling: Generate Joint Distributions
 - [x] Probabilistic Modeling: Generate Conditional Distributions
 - [x] Probabilistic Modeling: Sampling Marginal Distributions
 - [x] Probabilistic Modeling: Sampling Joint Distributions
 - [x] Probabilistic Modeling: Sampling Joint Distributions given causality
 - [x] Probabilistic Modeling: Expectation of attribute values across a relation set
 - [x] Probabilistic Modeling: Counting Relations
 - [x] Probabilistic Modeling: Counting Entity occurrences in Relation sets
 - [x] Hosting: Flask server logic utilizing wsgi_mod with Apache
 - [ ] Hosting: Mapping scheme from URL to Parser Commands (ADDITIONAL COMMANDS NEED TO BE HANDLED)
 - [x] Hosting: HTTP Parsing logic (see views.py)
 - [x] Hosting: Broker functionality for parser commands coming in form HTTP (currently uses redis)
 - [ ] Hosting: Broker serving logic (NEEDS TESTING)


Development
-----------

All contributions are certainly welcome!  In fact I'd love to have some partners on this work at this point especially those
that have interest in schemaless relational systems and bayesian statistics.

To get going simply follow the instructions in "Setup" above (or even fork your own repo) and use the push.sh script to
push your changes to your local vagrant instance for testing.

