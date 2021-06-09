# python_help.py : Python class to support the help patch for the [mxpy] object.

# Define a class which can generate objects to be instantiated in Max [mxpy] objects.
# This would be created in Max as [mxpy demo_class DemoClass].

class PyHelpClass:

    # The class initializer receives the arguments given to the Max [mxpy] object.
    def __init__( self, *creation ):
        return

    # The following methods are called in response to messages.
    def mx_bang(self):
        return True

    def mx_int(self, number):
        return number

    def mx_float(self, number):
        return number

    def mx_symbol(self, string):
        return string

    def mx_list(self, *args):
        # note that the *args form provides a tuple
        return list(args)

    # Messages with selectors are analogous to function calls, and call the
    # corresponding method.  E.g., the following could called by sending the Max
    # message [goto 33(.
    def goto(self, location):
        return location

    # This method demonstrates returning a list, which returns as a Max list.
    def moveto(self, x, y, z):
        return [x, y, z]

    def blah(self):
        return 42.0

    # This method demostrates returning a tuple, each element of which generates
    # a separate Pd outlet message.
    def tuple(self):
        return ( ['element', 1], ['element', 2], ['element', 3], ['element', 4] )

