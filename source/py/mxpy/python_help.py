# python_help.py : Python class to support the help patch for the [mxpy] object.

# Import the callback module built in to the external.
#import mxgui

# Define a class which can generate objects to be instantiated in Pd [python] objects.
# This would be created in Pd as [mxpy demo_class DemoClass].

class PyHelpClass:

    # The class initializer receives the arguments given to the Max [mxpy] object.
    def __init__( self, *creation ):
 #       mxgui.post( "Creating PyHelpClass object with creation args: %s." % str(creation))
        return

    # The following methods are called in response to messages.
    def bang(self):
  #      mxgui.post( "Python PyHelpClass object received bang.")
        return True

    def float(self, number):
  #      mxgui.post( "Python PyHelpClass object received %f." % number)
        return number

    def symbol(self, string ):
  #      mxgui.post( "Python PyHelpClass object received symbol: '%s'." % string)
        return string

    def list(self, *args ):
        # note that the *args form provides a tuple
  #      mxgui.post( "Python PyHelpClass object received list: %s." % str(args))
        return list(args)


    # Messages with selectors are analogous to function calls, and call the
    # corresponding method.  E.g., the following could called by sending the Max
    # message [goto 33(.
    def goto(self, location):
  #      mxgui.post( "Python PyHelpClass object received goto message:" + location)
        return location

    # This method demonstrates returning a list, which returns as a Max list.
    def moveto(self, x, y, z):
  #      mxgui.post( "Python PyHelpClass object received move message with %f, %f, %f." % ( x, y, z))
        return [x, y, z]

    def blah(self):
  #      mxgui.post( "Python PyHelpClass object received blah message.")
        return 42.0

    # This method demostrates returning a tuple, each element of which generates
    # a separate Pd outlet message.
    def tuple(self):
  #      mxgui.post( "Python PyHelpClass object received tuple message.")
        return ( ['element', 1], ['element', 2], ['element', 3], ['element', 4] )

