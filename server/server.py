import asyncio
import sqlite3
from websockets.asyncio.server import serve

exiting = False
port = 19992

cx = sqlite3.connect("points.db")
cursor = cx.cursor()

cursor.execute("CREATE TABLE IF NOT EXISTS Users (name TEXT UNIQUE, displayName TEXT, points INTEGER, teamID INTEGER)")
cx.commit()

class Client:
    lastClientID = 0
    clients = set()

    def __init__(self, ws):
        self.websocket = ws
        self.ip = ws.remote_address[0]
        self.name = ""
        self.team = -1
        self.id = Client.lastClientID
        self.isAdmin = False

        Client.lastClientID += 1
        Client.clients.add(self)

        print(f"Client #{self.id} connected ({self.ip})")
    
    def setName(self, name):
        self.name = name
    
    async def send(self, msg):
        await self.websocket.send(msg)

    async def sendAlert(self, title: str, message: str, button: str = None):
        additionalInfo = ""
        
        if button != None:
            additionalInfo = f"~|~{button}"

        await self.send(f"/alert {title}~|~{message}{additionalInfo}")
    
    async def sendErrorAlert(self, content: str):
        await self.sendAlert("Error!", content)

async def handleCommand(client: Client, msg: str, command: str, func: function, alone: bool = False):
    extrachar = ""
    if not alone:
        extrachar = " "

    if not msg.startswith(command + extrachar):
        return
    
    commandLength: int = len(command) + len(extrachar)
    content: str = msg[commandLength:].strip()

    await func(client, content)

async def login(client: Client, content: str):
    global cursor, cx

    cursor.execute("SELECT * FROM Users WHERE name = ?", (content.lower(),))
    userInfo = cursor.fetchone()

    if userInfo == None:
        await client.sendErrorAlert(f"<cr>\"{content}\"</c> hasn't been registered.\nPlease ask the organizer to register your username!")
        return

    client.name = content
    client.team = userInfo[3]

    cursor.execute("UPDATE Users SET displayName = ?", (content,))
    cx.commit()

    print(f"{content} (#{client.id}) logged in")
    await client.send("/success login")

async def getClients(client: Client, content: str):
    print("Getting clients")

    res = ""

    for c in Client.clients:
        additionalInfo = ""

        if c.name != "":
            additionalInfo = f" {c.name} {c.team}"
        
        res += f"{c.ip} {c.id}{additionalInfo}\n"
    
    res = res[:-1] # remove newline

    print(res)

    await client.send(res)

async def authCommand(client: Client, content: str):
    if content != "OWT.Admin!":
        await client.sendErrorAlert("Wrong password!")
        return
    
    client.isAdmin = True
    print(f"Client #{client.id} authenticated as an admin!")
    await client.send("/success")

async def ping(client: Client, content: str):
    #print(f"Received ping from {client.ip}")
    await client.send("/ping")

async def broadcast(client: Client, content: str):
    if not client.isAdmin:
        await client.sendErrorAlert("Permissions missing!")
        return
    
    print(content)
    deleteQueue = []
    for cl in Client.clients:
        try:
            if cl == client:
                continue
            
            print(cl.ip)
            await cl.send(content)
        except Exception as e:
            deleteQueue.append(cl)
    for cl in deleteQueue:
        Client.clients.discard(cl)

async def toClient(client: Client, content: str):
    if not client.isAdmin:
        await client.sendErrorAlert("Permissions missing!")
        return
    
    params = content.split("~~~|||~~~")

    if len(params) < 2:
        await client.sendErrorAlert("Not enough parameters!")
        return

    try:
        clientID = int(params[0])
    except ValueError:
        print("Invalid Client ID")
        await client.sendErrorAlert("Invalid client ID!")
        return
    
    command = params[1]

    print(f"{clientID} gets {command}")
    deleteQueue = []
    for cl in Client.clients:
        try:
            if cl.id != clientID:
                continue
            
            await cl.send(command)
        except Exception as e:
            deleteQueue.append(cl)
    for cl in deleteQueue:
        Client.clients.discard(cl)

async def exitCommand(client: Client, content: str):
    print(f"ID {client.id} disconnected")
    Client.clients.discard(client)
    await client.websocket.close()

async def completed(client: Client, content: str):
    global cx, cursor
    
    if client.team == -1:
        await client.sendErrorAlert("Points not updated because you're not logged in.")
        return
    
    print(f"{client.name} completed the level with ID {content}")

    cursor.execute("UPDATE Users SET points = points + ? WHERE name = ?", (1, client.name.lower()))
    cx.commit()

async def playLevel(client: Client, content: str):
    if not client.isAdmin:
        await client.sendErrorAlert("Missing permissions!")
        return
    
    if len(content) < 3:
        await client.sendErrorAlert("No ID given")
        return
    
    try:
        int(content)
    except ValueError:
        await client.sendErrorAlert("Invalid ID")
        return

    await broadcast(client, f"/play {content}")

async def exitLevels(client: Client, content: str):
    if not client.isAdmin:
        await client.sendErrorAlert("Missing permissions!")
        return
    
    await broadcast(client, "/levelkick")

async def register(client: Client, content: str):
    global cursor, cx
    if not client.isAdmin:
        await client.sendErrorAlert("Missing permissions!")
        return
    
    params = content.split(" ")

    if len(params) < 2:
        await client.sendErrorAlert("Not enough parameters.")
        return

    name = params[0]
    teamID = params[1]

    cursor.execute("SELECT * FROM Users WHERE name = ?", (name,))
    userInfo = cursor.fetchone()
    
    if userInfo != None:
        await client.sendErrorAlert("User already registered.")
        return

    cursor.execute("INSERT INTO Users VALUES (?, ?, ?, ?)", (name.lower(), name, 0, teamID))
    cx.commit()

    await client.send("/success")


async def serverLoop(websocket):
    client = Client(websocket)
    try:
        async for message in websocket:
            messageStr = str(message)
            await handleCommand(client, messageStr, "/register", register)
            await handleCommand(client, messageStr, "/login", login)
            await handleCommand(client, messageStr, "/getclients", getClients, True)
            await handleCommand(client, messageStr, "/ping", ping, True)
            await handleCommand(client, messageStr, "/broadcast", broadcast)
            await handleCommand(client, messageStr, "/to", toClient)
            await handleCommand(client, messageStr, "/exit", exitCommand, True)
            await handleCommand(client, messageStr, "/auth", authCommand)
            await handleCommand(client, messageStr, "/completed", completed)
            await handleCommand(client, messageStr, "/play", playLevel)
            await handleCommand(client, messageStr, "/exitLevels", exitLevels, True)
    except Exception as e:
        print(e)
        Client.clients.discard(client)
        await websocket.close()


async def main():
    global exiting, port
    while not exiting:
        try:
            async with serve(serverLoop, "0.0.0.0", port) as server:
                print("Serving on port", port)
                await server.serve_forever()
        except Exception as e:
            print(e)
            Client.clients.clear()
        


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        exiting = True