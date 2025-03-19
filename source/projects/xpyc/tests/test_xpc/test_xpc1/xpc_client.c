#include <stdio.h>
#include <xpc/xpc.h>
#include <CoreFoundation/CoreFoundation.h>

int main()
{
    // double x = [_xField doubleValue];
    // double y = [_yField doubleValue];
    double x = 0.0;
    double y = 0.0;
    printf("x = %f, y = %f\n", x, y);
    
    xpc_connection_t conn = xpc_connection_create("com.demo.XPCMultiplierService", NULL);
    xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
    // You have to set an event handler or else xpc_connection_resume will crash.
    xpc_connection_set_event_handler(conn, ^(xpc_object_t object) {
    });
    xpc_connection_resume(conn);
    
    xpc_dictionary_set_double(message, "x", x);
    xpc_dictionary_set_double(message, "y", y);
    xpc_connection_send_message_with_reply(conn, message, dispatch_get_main_queue(), ^(xpc_object_t object) {
        double result = xpc_dictionary_get_double(object, "result");
        printf("Result is %f\n", result);
        // [_resultField setDoubleValue:result];
    });
    xpc_release(message);
    xpc_release(conn);
}