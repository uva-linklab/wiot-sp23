#!/usr/bin/env python3

import base64
import configparser
from datetime import datetime
import json

import paho.mqtt.client as mqtt

from groups import group1, group2, group3, group5, group6, group7, group8, group9
from groups import group10, group11, group12, group13, group15, group16
from groups import group17, group18, group19, group20, group22

# Read in config file with MQTT details.
config = configparser.ConfigParser()
config.read("config.ini")

# MQTT broker details
broker_address = config["mqtt"]["broker"]
username = config["mqtt"]["username"]
password = config["mqtt"]["password"]

# MQTT topic to subscribe to. We subscribe to all uplink messages from the
# devices.
topic = "v3/+/devices/+/up"

decoders = {
    1: group1.decode,
    2: group2.decode,
    3: group3.decode,
    5: group5.decode,
    6: group6.decode,
    7: group7.decode,
    8: group8.decode,
    9: group9.decode,
    10: group10.decode,
    11: group11.decode,
    12: group12.decode,
    13: group13.decode,
    15: group15.decode,
    16: group16.decode,
    17: group17.decode,
    18: group18.decode,
    19: group19.decode,
    20: group20.decode,
    22: group22.decode,
}

# Callback when successfully connected to MQTT broker.
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker.")

    if rc != 0:
        print(" Error, result code: {}".format(rc))


# Callback function to handle incoming MQTT messages
def on_message(client, userdata, message):
    # Timestamp on reception.
    current_date = datetime.now()

    # Handle TTN packet format.
    message_str = message.payload.decode("utf-8")
    message_json = json.loads(message_str)
    encoded_payload = message_json["uplink_message"]["frm_payload"]
    raw_payload = base64.b64decode(encoded_payload)

    if len(raw_payload) == 0:
        # Nothing we can do with an empty payload.
        return

    # First byte should be the group number, remaining payload must be parsed.
    group_number = raw_payload[0]
    remaining_payload = raw_payload[1:]

    # See if we can decode this payload.
    if group_number in decoders:
        try:
            temperature = decoders[group_number](remaining_payload)
        except:
            print("Failed to decode payload for Group {}".format(group_number))
            print("  payload: {}".format(remaining_payload))
            return

        if temperature == None:
            print("Undecoded message from Group {}".format(group_number))
        else:
            print("{} temperature: {}".format(current_date.isoformat(), temperature))

    else:
        print("Received message with unknown group: {}".format(group_number))


# MQTT client setup
client = mqtt.Client()

# Setup callbacks.
client.on_connect = on_connect
client.on_message = on_message

# Connect to broker.
client.username_pw_set(username, password)
client.tls_set()
client.connect(broker_address, 8883)

# Subscribe to the MQTT topic and start the MQTT client loop
client.subscribe(topic)
client.loop_forever()
