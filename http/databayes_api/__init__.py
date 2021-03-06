__author__ = 'rfaulk'

from databayes_api import config, redisio
from flask import Flask
import logging

FORMAT="%(asctime)s %(levelname)-8s %(message)s"
logging.basicConfig(format=FORMAT)
log = logging.getLogger(__name__)
log.setLevel(logging.ERROR)

def log_set_level(level):
    """ Set logging level """
    log.setLevel(level)


# Instantiate flask app
app = Flask(__name__)
app.config['SECRET_KEY'] = config.__secret_key__
app.config['VERSION'] = config.__version__


def gen_queue_id():
    """ Handles the logic to determine the ID for a new queue item
    :return: the integer queue id
    """
    redisio.DataIORedis().connect()
    counter_curr = redisio.DataIORedis().read(config.REDIS_QUEUE_COUNTER_KEY)

    # In the case that the counter is not set initialize to 0
    if not counter_curr:
        counter_curr = 0

    if int(counter_curr) < config.REDIS_QUEUE_COUNTER_MAX:
        qid = counter_curr
        counter_curr = int(counter_curr) + 1
    else:
        qid = config.REDIS_QUEUE_COUNTER_MAX
        counter_curr = 0

    # Update the queue position / insert the command
    redisio.DataIORedis().write(config.REDIS_QUEUE_COUNTER_KEY, counter_curr)

    return qid


def exists_queue_item(qid):
    """ Detects existence of unique queue elements
    :param qid: int id of the queue item to look up
    :return:    boolean indicating whether it was found
    """
    if redisio.DataIORedis().read(config.REDIS_QUEUE_COUNTER_KEY + str(qid)):
        return True
    else:
        return False







