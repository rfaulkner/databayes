__author__ = 'rfaulk'

from databayes_api import config
from flask import Flask
from flask.ext.login import LoginManager


# Instantiate flask app
app = Flask(__name__)
app.config['SECRET_KEY'] = config.__secret_key__
app.config['VERSION'] = config.__version__

login_manager = LoginManager()
login_manager.init_app(app)


