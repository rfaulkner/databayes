"""
    Defines the routing endpoints of the RESTful API for databayes.

    Each method corresponds to an API action and returns the status of the action and the output.  This
    layer handles communication to the databayes daemon.

    IMPORTANT NOTE! - Only one of these server instances should be running to avoid race conditions
"""

from databayes_api import app, log, redisio, get_next_command, config, \
    gen_queue_id, exists_queue_item
import json

from flask import render_template, redirect, url_for, \
    request, escape, flash, g, session, Response


# UTILITY METHODS

def handle_queue_validation():
    """
    Method for handling queue validation in the view logic
    :return:
    """
    qid = str(gen_queue_id())
    iterations = 0
    while exists_queue_item(qid):
        if iterations == config.REDIS_QUEUE_COUNTER_MAX:
            return -1   # Indicates failure
        qid = str(gen_queue_id())
        iterations += 1
    return str(qid)


def unpack_query_params(request):
    """
    Helper method to fetch query paramaters for command requests
    :param request:
    :return:
    """
    ret = {'ok': True, 'fields': [], 'types': [], 'fields1': [],
           'fields2': [], 'values1': [], 'values2': [], 'message': ''}
    ret['fields'] = request.args.get('fields').split(',') \
        if request.args.get('fields') else []
    ret['types'] = request.args.get('types').split(',') \
        if request.args.get('fields') else []
    ret['fields1'] = request.args.get('fields1').split(',') \
        if request.args.get('fields1') else []
    ret['fields2'] = request.args.get('fields2').split(',') \
        if request.args.get('fields2') else []
    ret['values1'] = request.args.get('values1').split(',') \
        if request.args.get('values1') else []
    ret['values2'] = request.args.get('values2').split(',') \
        if request.args.get('values2') else []
    if len(ret['fields']) != len(ret['types']) or \
                    len(ret['fields1']) != len(ret['values1']) or \
                    len(ret['fields2']) != len(ret['values2']):
        ret['ok'] = False
        ret['message'] = 'Count of fields and types or values do not match'
    return ret


def wait_for_response(qid, poll_frequency = 10, max_tries = 5):
    """
    Handles polling a response from the redis queue determined by id.  Returns
        an empty response if it never arrives.
    :param qid:             int     redis queue id
    :param poll_frequency:  int     millisecond frequency of a poll
    :param max_tries:       int     poll no more times than this
    :return:                string  response written to redis from the daemon
    """
    rsp = ""
    for i in xrange(max_tries):
        rsp = redisio.DataIORedis().read(config.DBY_RSP_QUEUE_PREFIX + qid)
        if rsp: # got response, stop polling
            break
    return rsp

# VIEW METHODS

def define_entity(entity):
    """
    Handles remote requests to databayes for entity definition
    Translation:    def e(<f1>_<t1>, <f2>_<t2>, ...) ->
                    /def/e?fields=f1,f2,...&types=t1,t2,...
    :return:    JSON response indicating status of action & output
    """
    redisio.DataIORedis().connect()
    query_param_obj = unpack_query_params(request)
    if (not query_param_obj['ok']):
        return Response(json.dumps([query_param_obj['message']]),
                        mimetype='application/json')

    # Retrieve a valid queue item
    qid = handle_queue_validation()
    if qid == -1:
        return Response(json.dumps(['Queue is full, try again later.']),
                        mimetype='application/json')

    # Synthesize the command
    args = []
    for i in xrange(len(query_param_obj['fields'])):
        args[i] = query_param_obj['fields'][i] + '_' + \
                  query_param_obj['types'][i]
    cmd = 'def {0}({1})'.format(entity, ",".join(args))

    # Send cmd to databayes daemon
    redisio.DataIORedis().write(config.DBY_CMD_QUEUE_PREFIX + qid, cmd)

    return Response(json.dumps(['Command Inserted']),
                    mimetype='application/json')


def add_relation(entity_1, entity_2):
    """
    Handles remote requests to databayes for adding relations
    Translation:    add rel e1(<f1_1>_<v1_1>,...) e2(<f2_1>_<v2_1>,...) ->
                    /add/rel/e1/e2?fields1=f1_1,...&types1=t1_1,...
                        &fields2=f2_1,...&types2=t2_1,...
    :return:    JSON response indicating status of action & output
    """
    redisio.DataIORedis().connect()
    query_param_obj = unpack_query_params(request)
    if (not query_param_obj['ok']):
        return Response(json.dumps([query_param_obj['message']]),
                        mimetype='application/json')

    # Retrieve a valid queue item
    qid = handle_queue_validation()
    if qid == -1:
        return Response(json.dumps(['Queue is full, try again later.']),
                        mimetype='application/json')

    # Synthesize the command
    args1 = []
    args2 = []
    for i in xrange(len(query_param_obj['fields1'])):
        args1[i] = query_param_obj['fields1'][i] + '=' + \
                   query_param_obj['values1'][i]
    for i in xrange(len(query_param_obj['fields2'])):
        args2[i] = query_param_obj['fields2'][i] + '=' + \
                   query_param_obj['values2'][i]
    cmd = 'add rel {0}({1}) {2}({3})'.format(entity_1, ",".join(args1),
                                             entity_2, ",".join(args2))

    # Send cmd to databayes daemon
    redisio.DataIORedis().write(config.DBY_CMD_QUEUE_PREFIX + qid, cmd)

    return Response(json.dumps(['Command Inserted']),
                    mimetype='application/json')


def generate(entity_1, entity_2):
    """
    Handles remote requests to databayes for generating samples
    Translation:    gen e1(<f1_1>_<v1_1>,...) constrain e2(<f2_1>_<v2_1>,...) ->
                    /gen/e1/e2?fields1=f1_1,...&types1=t1_1,...&fields2=f2_1,...&types2=t2_1,...
    :return:    JSON response indicating status of action & output
    """
    return Response(json.dumps(['Command Inserted']), mimetype='application/json')


def list_entity(pattern):
    """
    Handles remote requests to databayes for listing entities
    Translation:    lst ent regex -> /lst/ent/regex
    :return:    JSON response indicating status of action & output
    """
    redisio.DataIORedis().connect()
    query_param_obj = unpack_query_params(request)
    if (not query_param_obj['ok']):
        return Response(json.dumps([query_param_obj['message']]),
                        mimetype='application/json')

    # Retrieve a valid queue item
    qid = handle_queue_validation()
    if qid == -1:
        return Response(json.dumps(['Queue is full, try again later.']),
                        mimetype='application/json')

    # Synthesize the command
    cmd = 'lst ent {0}'.format(pattern)

    # Send cmd to databayes daemon
    redisio.DataIORedis().write(config.DBY_CMD_QUEUE_PREFIX + qid, cmd)

    # Fetch response
    rsp = wait_for_response(qid)
    if not rsp:
        rsp = "Could not find response before max retires expired."

    return Response(json.dumps([rsp]), mimetype='application/json')


def list_relation(pattern_1, pattern_2):
    """
    Handles remote requests to databayes for listing relations
    Translation:    lst rel regex1 regex2 -> /lst/ent/regex1/regex2
    :return:    JSON response indicating status of action & output
    """
    return Response(json.dumps(['Command Inserted']),
                    mimetype='application/json')


def remove_entity(entity):
    """
    Handles remote requests to databayes for removing entities
    Translation:    rm ent e -> /rm/ent/e
    :return:    JSON response indicating status of action & output
    """
    redisio.DataIORedis().connect()
    # Retrieve a valid queue item
    qid = handle_queue_validation()
    if qid == -1:
        return Response(json.dumps(['Queue is full, try again later.']),
                        mimetype='application/json')

    # Synthesize the command
    cmd = 'rm ent {0}'.format(entity)

    # Send cmd to databayes daemon
    redisio.DataIORedis().write(config.DBY_CMD_QUEUE_PREFIX + qid, cmd)

    # TODO - check response to ensure that the entity was indeed removed

    return Response(json.dumps(['Command Inserted']),
                    mimetype='application/json')


# TODO - this code is very similar to "add relation" ... refactor
def remove_relation(entity_1, entity_2):
    """
    Handles remote requests to databayes for removing relations

    Translation:    rm rel e1(<f1_1>_<v1_1>,...) e2(<f2_1>_<v2_1>,...)
        -> /rm/rel/e1/e2?fields1=f1_1,...&values1=t1_1,...&fields2=f2_1,
        ...&values2=t2_1,...
    :return:    JSON response indicating status of action & output
    """
    redisio.DataIORedis().connect()
    query_param_obj = unpack_query_params(request)
    if (not query_param_obj['ok']):
        return Response(json.dumps([query_param_obj['message']]),
                        mimetype='application/json')

    # Retrieve a valid queue item
    qid = handle_queue_validation()
    if qid == -1:
        return Response(json.dumps(['Queue is full, try again later.']),
                        mimetype='application/json')

    # Synthesize the command
    args1 = []
    args2 = []
    for i in xrange(len(query_param_obj['fields1'])):
        args1[i] = query_param_obj['fields1'][i] + '=' + \
                   query_param_obj['values1'][i]
    for i in xrange(len(query_param_obj['fields2'])):
        args2[i] = query_param_obj['fields2'][i] + '=' + \
                   query_param_obj['values2'][i]
    cmd = 'rm rel {0}({1}) {2}({3})'.format(entity_1, ",".join(args1),
                                             entity_2, ",".join(args2))
    # Send cmd to databayes daemon
    redisio.DataIORedis().write(config.DBY_CMD_QUEUE_PREFIX + qid, cmd)

    # TODO - check response to ensure that the relation was indeed removed

    return Response(json.dumps(['Command Inserted']),
                    mimetype='application/json')


# Stores view references in structure
view_list = {
    define_entity.__name__: define_entity,
    add_relation.__name__: add_relation,
    generate.__name__: generate,
    list_entity.__name__: list_entity,
    list_relation.__name__: list_relation,
    remove_entity.__name__: remove_entity,
    remove_relation.__name__: remove_relation,
}

route_deco = {
    define_entity.__name__: app.route('/def/<entity>', methods=['GET', 'POST']),
    add_relation.__name__: app.route('/add/<entity_1>/<entity_2>', methods=['GET', 'POST']),
    generate.__name__: app.route('/gen', methods=['GET', 'POST']),
    list_entity.__name__: app.route('/lst/ent/<pattern>', methods=['GET', 'POST']),
    list_relation.__name__: app.route('/lst/rel/<pattern_1>/<pattern_2>', methods=['GET', 'POST']),
    remove_entity.__name__: app.route('/rm/ent/<entity>', methods=['GET', 'POST']),
    remove_relation.__name__: app.route('/rm/rel/<entity_1>/<entity_2>', methods=['GET', 'POST']),
}

# Apply decorators to views
def init_views():
    for key in route_deco:
        log.info('Registering view - {0}'.format(key))
        route = route_deco[key]
        view_method = view_list[key]
        view_list[key] = route(view_method)
