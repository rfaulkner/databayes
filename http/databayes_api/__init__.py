__author__ = 'rfaulk'

from databayes_api import config
from flask import Flask
from flask.ext.login import LoginManager
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

login_manager = LoginManager()
login_manager.init_app(app)






