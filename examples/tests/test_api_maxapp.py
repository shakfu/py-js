# https://docs.cycling74.com/max8/vignettes/messages_to_max#Sending_a_message_to_the_Max_application

import api


def test_maxapp_init():
    app = api.MaxApp()

def test_maxapp_midilistr():
    app = api.MaxApp()
    app.midilist()


def test_maxapp_clean():
    app = api.MaxApp()
    app.clean()

def test_maxapp_maxwindow():
    app = api.MaxApp()
    app.maxwindow()

def test_maxapp_clearmaxwindow():
    app = api.MaxApp()
    app.clearmaxwindow()

def test_maxapp_paths():
    app = api.MaxApp()
    app.paths()

def test_maxapp_externaleditor():
    app = api.MaxApp()
    app.externaleditor("Sublime Editor")
