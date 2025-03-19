# xpc service

## Creating an XPC Service (low-level c-code)

To create an XPC service in Xcode, do the following:

1. Add a new target to your project, using the XPC Service template.

2. Add a Copy Files phase to your application’s build settings, which copies the XPC service into the `Contents/XPCServices` directory of the main application bundle.

3. Add a dependency to your application’s build settings, to indicate it depends on the XPC service bundle.

4. If you are writing a low-level (C-based) XPC service, implement a minimal main function to register your event handler, as shown in the following code listing. Replace my_event_handler with the name of your event handler function.

5. Add the appropriate key/value pairs to the helper’s `Info.plist` to tell `launchd` the name of the service. These are described in [XPC Service Property List Keys](#xpc-service-property-list-keys).

```c
int main(int argc, const char *argv[]) {
	xpc_main(my_event_handler);
	// The xpc_main() function never returns.
	
	exit(EXIT_FAILURE);
}
```

## Using the C XPC Services API (low-level c-code)

Typical program flow is as follows:

1. An application calls `xpc_connection_create` to create an an XPC connection object.

2. The application calls some combination of `xpc_connection_set_event_handler` or `xpc_connection_set_target_queue` as needed to configure connection parameters prior to actually connecting to the service.

3. The application calls `xpc_connection_resume` to begin communication.

4. The application sends messages to the service using `xpc_connection_send_message`, `xpc_connection_send_message_with_reply`, or `xpc_connection_send_message_with_reply_sync`.

5. When you send the first message, the `launchd` daemon searches your application bundle for a service bundle whose `CFBundleIdentifier` value matches the specified name, then launches that XPC service daemon on demand.

6. The event handler function (specified in the service’s `Info.plist` file) is called with the message. The event handler function runs on a queue
whose name is the name of the XPC service.

7. If the original message was sent using `xpc_connection_send_message_with_reply` or `xpc_connection_send_message_with_reply_sync`, the service must reply using `xpc_dictionary_create_reply`, then uses `xpc_dictionary_get_remote_connection` to obtain the client connection and `xpc_connection_send_message`, `xpc_connection_send_message_with_reply`, or `xpc_connection_send_message_with_reply_sync` to send the reply dictionary back to the application.

The service can also send a message directly to the application with `xpc_connection_send_message`.

8. If a reply was sent by the service, the handler associated with the previous message is called upon receiving the reply. The reply can be put on a different queue than the one used for incoming messages. No serial relationship is guaranteed between reply messages and non-reply messages.

9. If an error occurs (such as the connection closing), the connection’s event handler (set by a previous call to `xpc_connection_set_event_handler`) is called with an appropriate error, as are (in no particular order) the handlers for any outstanding messages that are still awaiting replies.

10. At any time, the application can call `xpc_connection_suspend` when it needs to suspend callbacks from the service. All suspend calls must be balanced with resume calls. It is not safe to release the last reference to a suspended connection.

11. Eventually, the application calls `xpc_connection_cancel` to terminate the connection.

Note: Either side of the connection can call `xpc_connection_cancel`. There is no
functional difference between the application canceling the connection and the service canceling the connection.

## XPC Service Property List Keys

XPC requires you to specify a number of special key-value pairs in the `Info.plist` file within the service helper’s bundle. These keys are listed below.

- `CFBundleIdentifier`: String. The name of the service in reverse-DNS style (for example, `com.example.myapp.myservice`). (This value is filled in by the XPC Service Template.

- `CFBundlePackageType`: String. Value must be `XPC!` to identify the bundle as an XPC service. (This value is filled in by the XPC Service template.)

- `XPCService`: Dictionary. Contains the following keys:

	- `EnvironmentVariables`: Dictionary. The variables which are set in the environment of the service.
	
	- `JoinExistingSession`: Boolean. Indicates that your service runs in the same security session as the caller. The default value is `False`, which indicates that the service is run in a new security session. Set the value to `True` if the service needs to access to the user’s keychain, the pasteboard, or other per-session resources and services.
	
	- `RunLoopType`: String. Indicates the type of run loop used for the service. The default value is `dispatch_main`, which uses the dispatch_main function to set up a GCD-style run loop. The other supported value is `NSRunLoop`, which uses the `NSRunLoop` class to set up a run loop.
