#!/usr/bin/env bash

#   Core Packages
#   -------------

apt-get update
apt-get install -y apache2
apt-get install -y libapache2-mod-wsgi

rm -rf /var/www
ln -fs /vagrant /var/www
apt-get install -y gfortran libopenblas-dev liblapack-dev
apt-get install -y g++ gdb


#   General Packages
#   ---------------

apt-get install -y git
apt-get install -y vim
apt-get install -y redis-server
apt-get install -y curl

#   C++ pacakges
#   ------------
apt-get install -y libmsgpack-dev libboost-thread-dev libboost-date-time-dev libboost-test-dev libboost-filesystem-dev libboost-system-dev libhiredis-dev cmake build-essential pkg-config


#   Python Packages
#   ---------------

apt-get install -y python-dev
apt-get install -y python-pip
apt-get install -y python2.7-mysqldb
apt-get install -y build-essential python-all-dev libboost-python-dev libssl-dev

apt-get -y install software-properties-common
apt-get -y install python-software-properties

pip install redis
apt-get -y install python-beautifulsoup

