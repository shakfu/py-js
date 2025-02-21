
def fourchar(code: str) -> int:
   """Convert fourc chars to an int
   
   >>> fourchar('HUVL')
   1213552204
   """
   assert len(code) == 4, "should be four characters only"
   return ((ord(code[0]) << 24) | (ord(code[1]) << 16) |
           (ord(code[2]) << 8)  | ord(code[3]))
