from backend.bottle import route, run, request, static_file, get, post
# from backend.naive import searcher
from backend.triebased import searcher

handler = searcher()

@route('/')
def show_index():
    return load_static_file('index.html')

@route('/static/<filepath:path>')
def load_static_file(filepath):
    return static_file(filepath, root='./static/')

@get('/search')
def handle_query():
    qstr = request.params.get('q')
    lat = float(request.params.get('lat'))
    lng = float(request.params.get('lng'))
    return handler.query(qstr, lat, lng)

run(host = 'localhost', port = 8080)
