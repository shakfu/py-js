# Zeromq c language libraries

## CZMQ

main site: http://czmq.zeromq.org

diffs with libzmq (quote from https://github.com/zeromq/czmq)

    CZMQ has these goals:

    - To wrap the ØMQ core API in semantics that lead to shorter, more readable applications.
    
    - To hide as far as possible the differences between different versions of ØMQ (2.x, 3.x, 4.x).
    
    - To provide a space for development of more sophisticated API semantics.
    
    - To wrap the ØMQ security features with high-level tools and APIs.
    
    - To become the basis for other language bindings built on top of CZMQ.

Sounds promising. 


## ZMQ Threading Model

see: http://wiki.zeromq.org/blog:multithreading-magic#toc6

If done right, we eliminate the problems of the traditional approach and gain some extra advantages:

    - Our code is thread-naive. All data is private. Without exception, all of the "likely problems of multithreading code" disappear: no critical sections, no locks, no semaphores, no race conditions. No lock convoys, 3am nightmares about optimal granularity, no two-step dances.

    - Although it takes care to break an application into tasks that each run as one thread, it becomes trivial to scale an application. Just create more instances of a thread. You can run any number of instances, with no synchronization (thus no scaling) issues.
    
    - The application never blocks. Every thread runs asynchronously and independently. A thread has no timing dependencies on other threads. When you send a message to a working thread, the thread's queue holds the message until the thread is ready for more work.
    
    - Since the application overall has no lock states, threads run at full native speed, making full use of CPU caches, instruction reordering, compiler optimization, and so on. 100% of CPU effort goes to real work, no matter how many threads are active.

## Stackoverflow

- https://stackoverflow.com/questions/26182965/multi-threaded-publish-with-zmq
- https://stackoverflow.com/questions/38102504/how-to-stop-a-thread-that-is-waiting-for-client-to-connect