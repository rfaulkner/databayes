#!/usr/bin/python
# -*- coding: utf-8 -*-

from distutils.core import setup

with open('../README.md') as f:
    long_description = f.read()

__version__ = '0.0.1'

setup(
    name='databayes-http',
    version=__version__,
    long_description=long_description,
    description='HTTP RESTful endpoint for databayes.',
    url='http://www.github.com/rfaulkner/databayes',
    author="Ryan Faulkner",
    author_email="bobs.ur.uncle@gmail.com",
    packages=[
        'databayes-api',
        ],
    install_requires=[
        'Flask == 0.9',
        'Flask-Login == 0.2.6',
        'Flask-OpenID == 1.2.1',
        'python-dateutil >= 2.1',
        'redis >= 2.9.1',
        'sqlalchemy >= 0.9.1',
    ],
    classifiers=[
        'Development Status :: 2 - Pre-Alpha',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python',
        'Topic :: Software Development :: Libraries :: Python Modules'
    ],
    data_files=[('readme', ['README.md'])]
)
