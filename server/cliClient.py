#!/usr/bin/env python
import asyncio
from websockets.asyncio.client import connect

exiting = False

async def ping(ws):
     global exiting
     while not exiting:
        try:
            await ws.send("/ping")
            await asyncio.sleep(10)
        except asyncio.exceptions.CancelledError:
            exiting = True

async def loop(ws):
    global exiting
    while not exiting:
        try:
            req = await asyncio.to_thread(input, "> ")
            msg = req.strip()
            await ws.send(msg)
        except asyncio.exceptions.CancelledError:
            exiting = True

async def reader(ws):
    global exiting
    while not exiting:
        try:
            message = await ws.recv()
            msgStr = str(message)
            
            if msgStr == "/ping":
                continue
            
            print(msgStr)
        except asyncio.exceptions.CancelledError:
            pass

async def hello():
    while not exiting:
        try:
            async with connect("ws://localhost:19992") as websocket:
                await websocket.send("/settype 2")
                t1 = asyncio.create_task(loop(websocket))
                t2 = asyncio.create_task(ping(websocket))
                t3 = asyncio.create_task(reader(websocket))

                await t1
                await t2
                await t3
                print("Finished!")
        except Exception as e:
            print(e)
        
if __name__ == "__main__":
    try:
        thing = asyncio.run(hello())
    except KeyboardInterrupt:
        print("Bye")