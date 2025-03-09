import api

mem = {}

def test_coll_init():
    c = mem['c'] = api.Coll("mycoll")
    api.bang_success()

def test_coll_bang():
    c = mem['c']
    c.bang()
    
def test_coll_get():
    c = mem['c']
    c.get(1)
