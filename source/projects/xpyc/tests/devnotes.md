
## What can be sent

```c
void xpc_dictionary_set_value(xpc_object_t xdict, const char *key, xpc_object_t value);
void xpc_dictionary_set_bool(xpc_object_t xdict, const char *key, bool value);
void xpc_dictionary_set_int64(xpc_object_t xdict, const char *key, int64_t value);
void xpc_dictionary_set_uint64(xpc_object_t xdict, const char *key, uint64_t value);
void xpc_dictionary_set_double(xpc_object_t xdict, const char *key, double value);
void xpc_dictionary_set_date(xpc_object_t xdict, const char *key, int64_t value);
void xpc_dictionary_set_data(xpc_object_t xdict, const char *key, const void *bytes, size_t length);
void xpc_dictionary_set_string(xpc_object_t xdict, const char *key, const char *string);
void xpc_dictionary_set_uuid(xpc_object_t xdict, const char *key, const uuid_t uuid);
void xpc_dictionary_set_fd(xpc_object_t xdict, const char *key, int fd);
```

## What can be received

```c
xpc_object_t xpc_dictionary_get_value(xpc_object_t xdict, const char *key);
bool xpc_dictionary_get_bool(xpc_object_t xdict, const char *key);
int64_t xpc_dictionary_get_int64(xpc_object_t xdict, const char *key);
uint64_t xpc_dictionary_get_uint64(xpc_object_t xdict, const char *key);
double xpc_dictionary_get_double(xpc_object_t xdict, const char *key);
int64_t xpc_dictionary_get_date(xpc_object_t xdict, const char *key);
const void * xpc_dictionary_get_data(xpc_object_t xdict, const char *key, size_t * length);
const char * xpc_dictionary_get_string(xpc_object_t xdict, const char *key);
const uint8_t * xpc_dictionary_get_uuid(xpc_object_t xdict, const char *key);
xpc_object_t xpc_dictionary_get_dictionary(xpc_object_t xdict, const char *key);
xpc_object_t xpc_dictionary_get_array(xpc_object_t xdict, const char *key);
```
