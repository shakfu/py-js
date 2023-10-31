from pathlib import Path

import api


def test_db_init():
	db_path = Path("/tmp/test_db.sqlite3")
	db = api.Database("mydb", str(db_path))
	db.open()
	db.close()
	assert db_path.exists()

