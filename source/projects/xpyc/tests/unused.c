
void xpc_dump_type(xpc_object_t obj, const char* key)
{
    xpc_type_t obj_type = xpc_get_type(obj);

    if (obj_type == XPC_TYPE_CONNECTION) {
        post("XPC_TYPE_CONNECTION");

    }
    else if (obj_type == XPC_TYPE_ENDPOINT) {
        post("XPC_TYPE_ENDPOINT");

    }
    else if (obj_type == XPC_TYPE_BOOL) {
        post("XPC_TYPE_BOOL");
        // bool result = xpc_dictionary_get_bool(obj, key);
        // post("result: %d", result);
    }
    else if (obj_type == XPC_TYPE_INT64) {
        post("XPC_TYPE_INT64");
        // int64_t result = xpc_dictionary_get_int64(obj, key);
        // post("result: %lld", result);
    }
    else if (obj_type == XPC_TYPE_UINT64) {
        post("XPC_TYPE_UINT64");
        // uint64_t result = xpc_dictionary_get_uint64(obj, key);
        // post("result: %" PRIu64, result);
    }
    else if (obj_type == XPC_TYPE_DOUBLE) {
        post("XPC_TYPE_DOUBLE");
        // double result = xpc_dictionary_get_double(obj, key);
        // post("result: %f", result);
    }
    else if (obj_type == XPC_TYPE_DATE) {
        post("XPC_TYPE_DATE");
        // char date_string[20];
        // int64_t result = xpc_dictionary_get_date(obj, key);
        // time_t time_value = (time_t)result;
        // struct tm *local_time = localtime(&time_value);
        // if (local_time == NULL) {
        //     error("Error converting to local time");
        //     return;
        // }
        // strftime(date_string, sizeof(date_string), "%Y-%m-%d %H:%M:%S", local_time);
        // post("result: %s", date_string);
    }
    else if (obj_type == XPC_TYPE_DATA) {
        post("XPC_TYPE_DATA");
        // size_t length = 0;
        // const void * result = xpc_dictionary_get_data(obj, key, &length);
    }
    else if (obj_type == XPC_TYPE_UUID) {
        post("XPC_TYPE_UUID");
    }
    else if (obj_type == XPC_TYPE_FD) {
        post("XPC_TYPE_FD");
    }
    else if (obj_type == XPC_TYPE_SHMEM) {
        post("XPC_TYPE_SHMEM");
    }
    else if (obj_type == XPC_TYPE_ARRAY) {
        post("XPC_TYPE_ARRAY");
    }
    else if (obj_type == XPC_TYPE_DICTIONARY) {
        post("XPC_TYPE_DICTIONARY");
        // xpc_dictionary_apply(obj, ^(const char * _key, xpc_object_t _Nonnull value) {
        //     // Do iteration.
        //     post("key: %s", _key);
        //     post("XPC_TYPE_INT64");
        //     post("result: %lld", result);
        //     return (bool)true;
        // });
    }
    else if (obj_type == XPC_TYPE_ERROR) {
        post("XPC_TYPE_ERROR");
    }
    else if (obj_type == XPC_TYPE_RICH_ERROR) {
        post("XPC_TYPE_RICH_ERROR");
    }
}
