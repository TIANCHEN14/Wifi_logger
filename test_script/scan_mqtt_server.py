import paho.mqtt.client as mqtt
import streamlit as st
import pandas
import sqlite3
import queue
import threading
import time
import datetime

# define on connect functions for when the client receive a CONAACK respose 
def on_connect(client, userdata, flags, rc, properties):
        
    print(f"Connect with reason code {rc} ")
    
    client.subscribe("#")

# define callback function to get infomation is recevied from the server
def on_message(client, userdata, msg):

    # receiving messages
    payload = msg.payload.decode()

    # marking current time stamp
    right_now = datetime.datetime.now()
    current_date = right_now.strftime("%Y_%m_%d")
    #current_time = right_now.strftime("%X")
    current_time = int(time.time() * 1000000)

    # put it the queue so it is an thread safe envrioment
    message_queue.put((msg.topic, payload, current_date ,current_time))

    # print the message and payload
    print(f"Receivied message on {msg.topic} with |{payload}|")

# initlized the sqlite database
def inti_sqlite(db_path):
    # connect to the database
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    conn.commit()
    conn.close()

# put all the variable from queue to sqlite database
def queue_to_sqlite(db_path):
    # connect to the sqlite
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    # pull the element out of the queue
    while not message_queue.empty():
        topic, payload, current_date, current_time = message_queue.get()

        # append the variable in to the sqlite 
        # Create the messages table if it doesn't exist
        table_name = f"date_{current_date}"
        cursor.execute(f'''
            CREATE TABLE IF NOT EXISTS {table_name} (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                topic TEXT NOT NULL,
                payload TEXT NOT NULL,
                timestamp INTEGER
            )
        ''')

    
        # SQLite operation to appened information
        cursor.execute(f'''
        INSERT INTO {table_name} (topic, payload, timestamp) VALUES (?, ?, ?)
        ''', (topic, payload, current_time))

        conn.commit()

    conn.close()



# initilized the sqlite 
db_path = "./database/data.db"
inti_sqlite(db_path)


# define broker and port for for the clients
broker_address = '10.40.40.1'
broker_port = 1883

# define message queue object for store all the information
message_queue = queue.Queue()

# connect to the mqtt broker
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

# set callback function
client.on_connect = on_connect
client.on_message = on_message

# connect to the MQTT broker
client.connect(broker_address , broker_port)

# Function to periodically process the queue
def process_queue():
    while True:
        queue_to_sqlite(db_path)
        time.sleep(1)  # Adjust the sleep time as needed

# Start the queue processing thread
threading.Thread(target=process_queue, daemon=True).start()

# start the network loop in an non blocking therad
client.loop_forever()
