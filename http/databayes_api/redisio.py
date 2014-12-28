"""
Module for handling redis IO
"""

import redis
import hashlib

from flickipedia.config import log, settings

__author__ = 'Ryan Faulkner'
__date__ = "2014-04-01"


def hmac(key):
    """ Use an hmac to generate a hash key """
    return hashlib.md5(key + settings.__secret_key__).hexdigest()


def _decode_list(data):
    """ Decodes list elements """
    rv = []
    for item in data:
        if isinstance(item, unicode):
            item = item.encode('utf-8')
        elif isinstance(item, list):
            item = _decode_list(item)
        elif isinstance(item, dict):
            item = _decode_dict(item)
        rv.append(item)
    return rv


def _decode_dict(data):
    """
    Decodes dict elements.

    'object_hook' for json.loads (e.g. obj =
                                  json.loads(s, object_hook=_decode_dict))
    """
    rv = {}
    for key, value in data.iteritems():
        if isinstance(key, unicode):
            key = key.encode('utf-8')
        if isinstance(value, unicode):
            value = value.encode('utf-8')
        elif isinstance(value, list):
            value = _decode_list(value)
        elif isinstance(value, dict):
            value = _decode_dict(value)
        rv[key] = value
    return rv


class DataIORedis(object):
    """ Class implementing data IO for Redis. """

    DEFAULT_HOST = 'localhost'
    DEFAULT_PORT = 6379
    DEFAULT_DB = 0

    __instance = None

    def __new__(cls, *args, **kwargs):
        """ This class is Singleton, return only one instance """
        if not cls.__instance:
            cls.__instance = super(DataIORedis, cls).__new__(cls, *args,
                                                           **kwargs)
        return cls.__instance

    def __init__(self, **kwargs):
        super(DataIORedis, self).__init__(**kwargs)
        self.setconfig(**kwargs)

    def setconfig(self, **kwargs):
        """ Sets the instance config """
        self.host = kwargs['host'] if kwargs.has_key('host') else \
            self.DEFAULT_HOST
        self.port = kwargs['port'] if kwargs.has_key('port') else \
            self.DEFAULT_PORT
        self.db = kwargs['db'] if kwargs.has_key('db') else self.DEFAULT_DB

    def connect(self):
        self.conn = redis.Redis(host=self.host, port=self.port, db=self.db)

    def write(self, key, value):
        if self.conn:
            try:
                return self.conn.set(key, value)
            except KeyError as e:
                log.error('Missing param -> {0}'.format(e.message))
                return False
        else:
            log.error('No redis connection.')
            return False

    def read(self, key):
        if self.conn:
            try:
                return self.conn.get(key)
            except KeyError as e:
                log.error('Missing param -> {0}'.format(e.message))
                return False
        else:
            log.error('No redis connection.')
            return False

    def delete(self, **kwargs):
        if self.conn:
            try:
                return self.conn.delete(kwargs['key'])
            except KeyError as e:
                log.error('Missing param -> {0}'.format(e.message))
                return False
        else:
            log.error('No redis connection.')
            return False
