"""
    Defines the routing endpoints of the RESTful API for databayes.

    Each method corresponds to an API action and returns the status of the action and the output.  This
    layer handles communication to the databayes daemon.
"""

from databayes_api import app, log, redisio, get_next_command, config, \
    gen_queue_id
import json

from flask import render_template, redirect, url_for, \
    request, escape, flash, g, session, Response


def define_entity(entity):
    """ Handles remote requests to databayes for entity definition
    :return:    JSON response indicating status of action & output
    """
    redisio.DataIORedis().connect()
    fields = request.args.get('fields').split(',')
    types = request.args.get('types').split(',')

    if len(fields) != len(types):
        return Response(json.dumps(['number of fields and types do not match']),
                        mimetype='application/json')

    args = []
    for i in xrange(len(fields)):
        args[i] = fields[i] + '=' + types[i]
    cmd = 'def {0}({1})'.format(entity, ",".join(args))
    # Send cmd to databayes daemon
    redisio.DataIORedis().write(config.DBY_CMD_QUEUE_PREFIX + gen_queue_id(), cmd)

    return Response(json.dumps(['Command Inserted']),  mimetype='application/json')

def add_relation():
    """ Handles remote requests to databayes for adding relations
    :return:    JSON response indicating status of action & output
    """
    return Response(json.dumps(['Command Inserted']),  mimetype='application/json')

def generate():
    """ Handles remote requests to databayes for generating samples
    :return:    JSON response indicating status of action & output
    """
    return Response(json.dumps(['Command Inserted']),  mimetype='application/json')

def list_entity():
    """ Handles remote requests to databayes for listing entities
    :return:    JSON response indicating status of action & output
    """
    return Response(json.dumps(['Command Inserted']),  mimetype='application/json')

def list_relation():
    """ Handles remote requests to databayes for listing relations
    :return:    JSON response indicating status of action & output
    """
    return Response(json.dumps(['Command Inserted']),  mimetype='application/json')

# Stores view references in structure
view_list = {
    define_entity.__name__: define_entity,
    add_relation.__name__: add_relation,
    generate.__name__: generate,
    list_entity.__name__: list_entity,
    list_relation.__name__: list_relation,
}

route_deco = {
    define_entity.__name__: app.route('/def', methods=['GET', 'POST']),
    add_relation.__name__: app.route('/add', methods=['GET', 'POST']),
    generate.__name__: app.route('/gen', methods=['GET', 'POST']),
    list_entity.__name__: app.route('/lstrel', methods=['GET', 'POST']),
    list_relation.__name__: app.route('/lstent', methods=['GET', 'POST']),
}

# Apply decorators to views
def init_views():
    for key in route_deco:
        log.info('Registering view - {0}'.format(key))
        route = route_deco[key]
        view_method = view_list[key]
        view_list[key] = route(view_method)

    # log.info(__name__ + ' :: Registered views - {0}'.format(str(view_list)))