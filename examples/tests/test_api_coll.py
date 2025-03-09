import api

mem = {}

def test_coll_init():
    mem['c'] = api.Coll("mycoll")
    api.bang_success()

def test_coll_bang():
    """Retrieve the next data set"""
    c = mem['c']
    c.bang()

def test_coll_get_int():
    """Retrieve data by index"""
    c = mem['c']
    c.get(2)

def test_coll_get_float():
    """Retrieve data by index"""
    c = mem['c']
    c.get(2.0)

def test_coll_set():
    """Store index and data"""
    c = mem['c']
    c.set(1, 4, 1, 2.2, "nice one")

def test_coll_anything():
    """Retrieve data by symbol"""

def test_coll_append():
    """Add item associated with an index"""

def test_coll_assoc():
    """Associate a name with an index"""

def test_coll_clear():
    """Clear all data"""

def test_coll_deassoc():
    """De-associate a name with an index"""

def test_coll_delete():
    """Remove data and renumber"""

def test_coll_dump():
    """Output all data"""

def test_coll_end():
    """Move to last address"""

def test_coll_filetype():
    """Set the recognized file types"""

def test_coll_flags():
    """Set the file-save flag"""

def test_coll_goto():
    """Move to an index"""

def test_coll_insert():
    """Insert data at a specific address"""

def test_coll_insert2():
    """Insert data at a specific address"""

def test_coll_length():
    """Retrieve the number of entries"""

def test_coll_max():
    """Return the highest numeric value"""

def test_coll_merge():
    """Merge data at an existing address"""

def test_coll_min():
    """Return the lowest numeric value"""

def test_coll_next():
    """Move to the next address"""

def test_coll_nstore():
    """Store data with both number and symbol index"""

def test_coll_nsub():
    """Replace a single data element"""

def test_coll_nth():
    """Return a single data element"""

def test_coll_open():
    """Open a data editing window"""

def test_coll_prev():
    """Move to the previous address"""

def test_coll_read():
    """Choose a file to load"""

def test_coll_readagain():
    """Reload a file"""

def test_coll_refer():
    """Change data reference"""

def test_coll_remove():
    """Remove an entry"""

def test_coll_renumber():
    """Renumber entries"""

def test_coll_renumber2():
    """Increment indices by one"""

def test_coll_separate():
    """Creates an open entry index"""

def test_coll_sort():
    """Sort the data"""

def test_coll_start():
    """Move to the first entry"""

def test_coll_store():
    """Store data at a symbolic index"""

def test_coll_sub():
    """Replace a data element, output data"""

def test_coll_subsym():
    """Changes an index symbol"""

def test_coll_swap():
    """Swap two indices"""

def test_coll_symbol():
    """Retrieve data by symbol"""

def test_coll_wclose():
    """Close the data editing window"""

def test_coll_write():
    """Write the data to a disk file"""

def test_coll_writeagain():
    """Rewrite a file"""

