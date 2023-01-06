# watchmedo auto-restart --pattern "*.py" --recursive --signal SIGTERM python main.py

import asyncio
import json
import os
import websockets
import logging
import serial
import atexit

logging.basicConfig(level=os.environ.get("LOGLEVEL", "INFO"), format='%(asctime)s - %(message)s', datefmt='%H:%M:%S')


USER_ID = None
INITIAL_VOICE_SETTINGS = None
WEBSOCKET = None
SERIAL = serial.Serial("COM12", baudrate=9600, timeout=0.3)

async def message(websocket):
    global USER_ID, INITIAL_VOICE_SETTINGS, WEBSOCKET

    WEBSOCKET = websocket
    
    try:
        logging.info("websocket loop listening...")
        await asyncio.get_event_loop().run_in_executor(None, serial_send, "<#D1>")

        async for message in websocket:
                try:
                    parsed = json.loads(message)
                    logging.info("ws received: " + message)

                    serial_message = None

                    # ? Cant use switch here
                    if parsed["type"] == "SYN":
                        USER_ID = parsed["userId"]
                        INITIAL_VOICE_SETTINGS = parsed["initialVoiceSettings"]
                        response = json.dumps({"type": "ACK", "userId": USER_ID, "initialVoiceSettings": INITIAL_VOICE_SETTINGS})
                        local = INITIAL_VOICE_SETTINGS["local"]
                        serial_message = "<#S{}{}{}>".format(int(not local["mute"]), int(not local["deaf"]), 1 if INITIAL_VOICE_SETTINGS["currentVoiceChannelId"] else 0)
                        logging.info("responding to SYN with: " + response)
                        await websocket.send(response)
                        await asyncio.get_event_loop().run_in_executor(None, serial_send, serial_message)
                    elif parsed["type"] == "VOICE_STATE_UPDATES":
                        # ? Could use "AUDIO_TOGGLE" for everything, but this allows for handling other users in future
                        voice = parsed["voiceStates"][0]
                        if voice["userId"] == USER_ID:
                            serial_message = "<#V{}{}{}>".format(int(not voice["selfMute"]), int(not voice["selfDeaf"]), 1 if voice["channelId"] else 0)
                            await asyncio.get_event_loop().run_in_executor(None, serial_send, serial_message)
                    elif parsed["type"] == "RTC_CONNECTION_STATE":
                        serial_message = "<#C{}>".format(1 if parsed["state"] == "RTC_CONNECTED" else 0)
                        await asyncio.get_event_loop().run_in_executor(None, serial_send, serial_message)
                    elif parsed["type"] == "AUDIO_TOGGLE":
                        serial_message = "<#A{}{}>".format(int(not parsed["isMute"]), int(not parsed["isDeaf"]))
                        await asyncio.get_event_loop().run_in_executor(None, serial_send, serial_message)

                except json.JSONDecodeError:
                    logging.info(message)
    except websockets.exceptions.ConnectionClosed:
        logging.info("connection closed")
        INITIAL_VOICE_SETTINGS = None
    
    logging.info("websocket loop closing...")
    await asyncio.get_event_loop().run_in_executor(None, serial_send, "<#D0>")

def serial_send(data):
    global SERIAL

    if SERIAL.isOpen():
        logging.info("sending serial data: " + data)
        SERIAL.write(data.encode('ascii'))
        SERIAL.flush()
    else:
        logging.info("opening error")

def serial_read():
    incoming = SERIAL.read_until(b'>')
    return incoming.decode('ascii')

async def serial_server(loop=None):
    global SERIAL, WEBSOCKET

    if SERIAL.isOpen():
        logging.info("serial server listening...")

        while True:
            if SERIAL.in_waiting > 4:
                logging.info("serial receiving bytes...")
                serial_message = await loop.run_in_executor(None, serial_read)
                logging.info("serial received: " + serial_message)

                if serial_message[2] == "P":
                    await loop.run_in_executor(None, serial_send, "<#P1>")
                else:
                    await WEBSOCKET.send(json.dumps({"type": "SERIAL", "data": serial_message}))
            else:
                await asyncio.sleep(SERIAL.timeout)
    else:
        print ("serial not open")

    logging.info("serial server closing...")

def exit_handler():
    logging.info("exiting python...")
    logging.info("closing serial port")
    if SERIAL.isOpen():
        SERIAL.write("<#P0>".encode('ascii'))
        SERIAL.flush()
    SERIAL.close()

if __name__ == "__main__":
    try:
        atexit.register(exit_handler)
        loop = asyncio.get_event_loop()
        start_ws_server = websockets.serve(message, "localhost", 8000)
        loop.run_until_complete(start_ws_server)
        loop.run_until_complete(serial_server(loop=loop))
        loop.run_forever()
    except KeyboardInterrupt:
        pass