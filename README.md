databayes
=========

Probabilistic Database that uses bayesian inference to build up relations.

Install C++ redis client redis3m.  Follow the instructions at https://github.com/luca3m/redis3m.


Installation
------------

(Not yet implemented)

	git clone https://github.com/rfaulkner/databayes
	cd databayes
	cmake .
	make
	sudo make install


Setup
-----

Ensure that you LD_LIBRARY_PATH is set:

    export LD_LIBRARY_PATH=/usr/local/lib

Use the following compiler flags:

    -std=c++0x $(pkg-config --cflags --libs redis3m)