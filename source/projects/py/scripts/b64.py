import base64
  
def encode(s: str):
    s_bytes = s.encode("ascii")
    s_bytes_b64 = base64.b64encode(s_bytes)
    s_b64 = s_bytes_b64.decode("ascii")
    print("original length:", len(s))
    print(f"encoded: {s_b64}")
    print("encoded length:", len(s_b64))
    print("increase factor:", len(s_b64)/len(s))
    return s_b64

def decode(s_b64: str):
    s_bytes_b64 = s_b64.encode("ascii")
    s_bytes = base64.b64decode(s_bytes_b64)
    s = s_bytes.decode("ascii")
    print(f"decoded: {s}")
    return s

py_module = """
import base64
  
def encode(s: str):
    s_bytes = s.encode("ascii")
    s_bytes_b64 = base64.b64encode(s_bytes)
    s_b64 = s_bytes_b64.decode("ascii")
    print(f"encoded: {s_b64}")
    return s_b64

def decode(s_b64: str):
    s_bytes_b64 = s_b64.encode("ascii")
    s_bytes = base64.b64decode(s_bytes_b64)
    s = s_bytes.decode("ascii")
    print(f"decoded: {s}")
    return s
"""

s_b64 = encode(py_module)
s = decode(s_b64)
assert s == py_module
