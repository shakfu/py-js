#include <stdio.h>
#include <xpc/xpc.h>
#include <CoreFoundation/CoreFoundation.h>

static void xpc_peer_event_handler(xpc_connection_t peer, xpc_object_t event) 
{
	xpc_type_t type = xpc_get_type(event);
	if (type == XPC_TYPE_ERROR) {
		if (event == XPC_ERROR_CONNECTION_INVALID) {
			// The client process on the other end of the connection has either
			// crashed or cancelled the connection. After receiving this error,
			// the connection is in an invalid state, and you do not need to
			// call xpc_connection_cancel(). Just tear down any associated state
			// here.
		} else if (event == XPC_ERROR_TERMINATION_IMMINENT) {
			// Handle per-connection termination cleanup.
		}
	} else {
		assert(type == XPC_TYPE_DICTIONARY);
		// Handle the message.
        double x = xpc_dictionary_get_double(event, "x");
        double y = xpc_dictionary_get_double(event, "y");
        printf("Got message x = %f, y = %f", x, y);
        double result = x * y;
        xpc_object_t response = xpc_dictionary_create_reply(event);
        xpc_dictionary_set_double(response, "result", result);
        xpc_connection_send_message(peer, response);
        xpc_release(response);
	}
}

static void xpc_service_event_handler(xpc_connection_t peer) 
{
	// By defaults, new connections will target the default dispatch
	// concurrent queue.

	xpc_connection_set_event_handler(peer, ^(xpc_object_t event) {
		xpc_peer_event_handler(peer, event);
	});
	
	// This will tell the connection to begin listening for events. If you
	// have some other initialization that must be done asynchronously, then
	// you can defer this call until after that initialization is done.
	xpc_connection_resume(peer);
}

int main(int argc, const char *argv[])
{
    printf("xpc multipler service starting");
	xpc_main(xpc_service_event_handler);
	return 0;
}
