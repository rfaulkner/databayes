
__version__ = '0.0.1'

LICENSE = "BSD (Three Clause)"
AUTHORS = { 'Ryan Faulkner': 'bobs.ur.uncle@gmail.com'}
ADMINS = { 'Ryan Faulkner': 'bobs.ur.uncle@gmail.com' }
FLASK_LOG = '/var/log/flask.log'
SITE_URL = 'http://databayes-api.com'

DBY_CMD_QUEUE_PREFIX = 'dby_command_queue_' # Must match value in daemon.cpp
DBY_RSP_QUEUE_PREFIX = 'dby_response_queue_'
REDIS_QUEUE_COUNTER_KEY = 'dby_command_queue_counter'
REDIS_QUEUE_COUNTER_MAX = 10

__secret_key__ = "something"

__instance_host__ = '127.0.0.1'
__instance_host_vagrant__ = '0.0.0.0'
__instance_port__ = 5000

