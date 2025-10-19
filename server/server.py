import asyncio
from websockets.asyncio.server import serve

exiting = False

port = 19992

clients = set()

class Client:
    def __init__(self, ws):
        self.websocket = ws
        self.ip = ws.remote_address[0]
        self.name = ""
        self.team = -1
    
    def setName(self, name):
        self.name = name
    
    async def send(self, msg):
        await self.websocket.send(msg)


async def echo(websocket):
    client = Client(websocket)
    clients.add(client)
    try:
        async for message in websocket:
            messageStr = str(message)
            if messageStr.startswith("/changename"):
                name = messageStr[12:].strip()
                client.setName(name)
                print(f"{client.ip} is now used by {name}")
                await websocket.send("/success")
            
            elif messageStr.startswith("/login"):
                content = messageStr[7:].strip().split(" ")
                name = content[0]
                team = content[1]

                client.name = name
                client.team = team

                print(f"{name} logged into team {team}")
                await websocket.send("/success")

            elif messageStr.startswith("/getclients"):
                for c in clients:
                    print(c.ip, c.name)
                
                await websocket.send(f"/return {len(clients)} clients")

            elif messageStr.startswith("/say"):
                content = message[5:]
                print("RECEIVED:", content)
                await websocket.send(f"/return {content}")
            
            elif messageStr.startswith("/ping"):
                print(f"Received ping from {client.ip}")
                await websocket.send("/ping")

            elif messageStr.startswith("/broadcast"):
                content = messageStr[11:]
                print(content)
                deleteQueue = []
                for cl in clients:
                    try:
                        print(cl.ip)
                        await cl.send(content)
                    except Exception as e:
                        deleteQueue.append(cl)
                for cl in deleteQueue:
                    clients.discard(cl)
                        
            
            elif messageStr == "/exit":
                print("Disconnected")
                clients.discard(client)
                await websocket.close()
            
            else:
                print("error")
                await websocket.send("/error")
    except Exception as e:
        #print(e)
        clients.discard(client)
        await websocket.close()


async def main():
    global exiting, port
    while not exiting:
        try:
            async with serve(echo, "0.0.0.0", port) as server:
                print("Serving on port", port)
                await server.serve_forever()
        except Exception as e:
            print(e)
            clients.clear()
        


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        exiting = True