#!/usr/bin/env python3

from pythonosc import udp_client



client = udp_client.SimpleUDPClient('127.0.0.1', 7000)


while True:
    data = input(">>> ")
    #client.send_message("/py", data)
    client.send_message("/eval", data)

