from: https://developer.apple.com/forums/thread/696281


You should rather link with the framework provided by your installed version of Python3.

Locate your installation of Python3 by using this command in Terminal: 

ls -la $(which python3)

The framework is in the "Frameworks" folder located one level above "bin", 
for example: `/usr/local/Cellar/python@3.9/3.9.7_1/Frameworks`

Once your have the location of the Python3 framework, add it as a framework in your XCode project. In the Build Settings, don't forget to add:

1. the Frameworks folder location in Framework Search Path (e.g. "/usr/local/Cellar/python@3.9/3.9.7_1/Frameworks")

2. the framework's Headers folder in Header Search Path (e.g. "/usr/local/Cellar/python@3.9/3.9.7_1/Frameworks/Python.framework/Headers")


