# devnotes


## Examples - session

- <https://github.com/pookjw/Silicon/blob/3884a2c0b70ea54df05807c254ca6fc96e0983a7/Silicon/Utils/XPCManager.mm#L14>



## Examples - connection

- <https://github.com/st3fan/osx-10.9/blob/34e34a6a539b5a822cda4074e56a7ced9b57da71/libutil-34/tzlinkd/tzlinkd.c#L24>

- <https://github.com/aosm/crontabs/blob/6686f61e53d462c7738aacb52ba979054b8ee83c/periodic/periodic-wrapper.c#L6>

- <https://github.com/alfonsotesauro/desktop/blob/75bbc06b6f26684f3c6a774a73f23fafb2fc9d65/extras/installer/mac/helper/main.mm#L23>

- <https://github.com/Karmaz95/Snake_Apple/blob/7c5d4459805b78fc666576922def91fb17aedeea/X.%20NU/custom/mach_ipc/client_server_XPC/crimson_xpc_service.c#L5>

- <https://github.com/mbsanchez/QtPrivilegedHelperExample/blob/653b1248e64a98da254ca1d93b4dc8aee9d587d3/PrivilegedHelper/main.cpp#L55>

- <https://github.com/Apple-FOSS-Mirror/Security/blob/5bcad85836c8bbb383f660aaf25b555a805a48e4/OSX/trustd/trustd.c#L32>

- <https://github.com/AmesianX/electra/blob/2915d552cc7008757cc7983b704bf22f31f8780e/basebinaries/jailbreakd/main.m#L2>

<https://github.com/grisp/rtems-libbsd/blob/f60ac53420f36060c13104f9a555f39ebc619b09/mDNSResponder/Clients/dnsctl.c#L22>

- <https://github.com/trukhinyuri/nimble-commander/blob/972f4dcdae8764c38f0c60ca99a2a2bc7c6158ad/RoutedIO/source/PrivilegedIOHelper.cpp#L5>




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
