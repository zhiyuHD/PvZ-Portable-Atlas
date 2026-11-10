#!/usr/bin/env python3
"""Run a compiled game_agent_transport test against a local fake BYOK server."""
import http.server
import json
import subprocess
import sys
import threading

class Handler(http.server.BaseHTTPRequestHandler):
    def log_message(self, *args):
        pass

    def do_POST(self):
        body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
        assert self.headers['Authorization'] == 'Bearer mock-key'
        assert body['model'] == 'mock-model'
        assert {tool['function']['name'] for tool in body['tools']} == {'get_state', 'get_seed_bank', 'get_threats', 'inspect_cell', 'collect_sun', 'collect_coins', 'wait', 'place_plant', 'hover_shovel', 'confirm_shovel', 'cancel_shovel', 'get_menu_state', 'get_seed_choices', 'select_seed', 'deselect_seed', 'select_imitater', 'toggle_seed_page', 'start_level', 'start_adventure', 'continue_game', 'advance_intro'}
        kind = self.path.split('/')[1]
        name = 'place_plant' if kind == 'plant' else 'hover_shovel'
        if kind == 'unknown':
            name = 'remove_plant'
        args = {'row': 2, 'col': 3}
        if kind == 'plant':
            args['packet'] = 1
        call = {'type': 'function', 'function': {'name': name, 'arguments': json.dumps(args)}}
        response = json.dumps({'choices': [{'message': {'content': '已分析', 'tool_calls': [call] * (2 if kind == 'multiple' else 1)}}]}).encode()
        if kind == 'badjson':
            response = b'{invalid}'
        self.send_response(401 if kind == 'unauthorized' else 200)
        self.end_headers()
        try:
            self.wfile.write(response)
        except BrokenPipeError:
            pass

server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), Handler)
threading.Thread(target=server.serve_forever, daemon=True).start()
try:
    subprocess.run([sys.argv[1], f'http://127.0.0.1:{server.server_port}'], check=True)
finally:
    server.shutdown()
