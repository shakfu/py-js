import api


def test_table_init():
    t = api.Table("mytable")
    assert t.size == 128


def test_table_populate():
    t = api.Table("mytable")
    t.populate([i * 2 for i in range(128)])


def test_table_as_list():
    t = api.Table("mytable")
    xs = t.as_list()
    assert len(xs) == 128
