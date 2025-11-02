import asyncio
import sqlite3
from websockets.asyncio.server import serve

class Types:
    Undefined = -1
    Player = 0
    PointsReader = 1
    CommandLine = 2
    TypeAmount = 3

class Session:
    cx = sqlite3.connect("points.db")
    cursor = cx.cursor()
    exiting = False
    port = 19992
    completed: list[Client] = []
    lastLevelID = -1

    async def submitPoints(client: Client = None):
        Session.cursor.execute("SELECT teamID, points FROM Teams")
        usersInfo = Session.cursor.fetchall()

        res = ""

        for row in usersInfo:
            if len(res) != 0:
                res += "~~||~~"
            displayName = row[0]
            points = row[1]
            res += f"{displayName}~|~{points}"

        if client != None:
            await client.send(f"/submitpoints {res}")
            return

        for cl in Client.clients:
            if cl.type != Types.PointsReader:
                continue

            await cl.send(f"/submitpoints {res}")

Session.cursor.execute("CREATE TABLE IF NOT EXISTS Users (name TEXT UNIQUE, displayName TEXT, points INTEGER, teamID INTEGER)")
Session.cursor.execute("CREATE VIEW IF NOT EXISTS Teams AS SELECT teamID, SUM(points) AS points FROM Users GROUP BY teamID")
Session.cx.commit()
    
class Client:
    lastClientID = 0
    clients: set[Client] = set()

    def __init__(self, ws):
        self.websocket = ws
        self.ip = ws.remote_address[0]
        self.name: str = ""
        self.team: int = -1
        self.id: int = Client.lastClientID
        self.isAdmin: bool = False
        self.levelID: int = -1
        self.type: int = Types.Undefined

        Client.lastClientID += 1
        Client.clients.add(self)

        print(f"Client #{self.id} connected ({self.ip})")
    
    def setName(self, name):
        self.name = name

    async def kickFromLevel(self):
        await self.send("/levelkick")
        self.levelID = -1

    async def play(self, id: int):
        await self.send(f"/play {id}")
        self.levelID = id

    async def setType(self, id: int):
        self.type = id

        if id == Types.Player and Session.lastLevelID != -1:
            await self.play(Session.lastLevelID)
        if id == Types.PointsReader:
            await Session.submitPoints(self)
                
    
    async def send(self, msg):
        await self.websocket.send(msg)

    async def sendAlert(self, title: str, message: str, button: str = None):
        additionalInfo = ""
        
        if button != None:
            additionalInfo = f"~|~{button}"

        if self.type == Types.Player:
            await self.send(f"/alert {title}~|~{message}{additionalInfo}")
        else:
            await self.send(f"{title}: {message}")
    
    async def sendErrorAlert(self, content: str):
        await self.sendAlert("Error!", content)

async def handleCommand(client: Client, msg: str, command: str, func: function, alone: bool = False):
    extrachar = ""
    if not alone:
        extrachar = " "

    cmd = msg.split(" ")

    if not cmd[0] == command:
        return
    
    commandLength: int = len(command) + len(extrachar)
    content: str = msg[commandLength:].strip()

    await func(client, content)

async def login(client: Client, content: str):
    Session.cursor.execute("SELECT * FROM Users WHERE name = ?", (content.lower(),))
    userInfo = Session.cursor.fetchone()

    if userInfo == None:
        await client.sendErrorAlert(f"<cr>\"{content}\"</c> hasn't been registered.\nPlease ask the organizer to register your username!")
        return

    if client.name.lower() == content.lower():
        await client.sendAlert("Info", "You're already playing!")
        return

    for cl in Client.clients:
        if content.lower() == cl.name.lower():
            await client.sendErrorAlert(f"<cr>{cl.name}</c> is already playing!")
            return
        
    client.name = content
    client.team = userInfo[3]

    Session.cursor.execute("UPDATE Users SET displayName = ? WHERE name = ?", (content, content.lower()))
    Session.cx.commit()

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
    if client.type != Types.Player:
        await client.sendErrorAlert("Your type is not \"player\"!")
        return
    
    if client.team == -1:
        await client.sendErrorAlert("Points not updated because you're not logged in.")
        return
    
    print(f"{client.name} completed the level with ID {content}")

    if client in Session.completed:
        await client.sendErrorAlert("You already finished the level!")
        return

    playerCount = 0

    for cl in Client.clients:
        if cl.team != -1 and cl.type == Types.Player:
            playerCount += 1

    pointsEarned = playerCount - len(Session.completed)
    pointsEarned *= 2

    Session.completed.append(client)

    Session.cursor.execute("UPDATE Users SET points = points + ? WHERE name = ?", (pointsEarned, client.name.lower()))
    Session.cx.commit()

    Session.submitPoints()

async def playLevel(client: Client, content: str):
    if not client.isAdmin:
        await client.sendErrorAlert("Missing permissions!")
        return
    
    if len(content) == 0:
        await client.sendErrorAlert("No ID given")
        return
    
    try:
        int(content)
    except ValueError:
        await client.sendErrorAlert("Invalid ID")
        return

    id = int(content)

    Session.lastLevelID = id
    Session.completed.clear()

    for cl in Client.clients:
        await cl.play(id)

async def exitLevels(client: Client, content: str):
    if not client.isAdmin:
        await client.sendErrorAlert("Missing permissions!")
        return
    
    Session.completed.clear()

    for cl in Client.clients:
        await cl.kickFromLevel()

async def setType(client: Client, content: str):
    if content == "":
        await client.sendErrorAlert("No type specified")
        return
    
    try:
        int(content)
    except ValueError:
        await client.sendErrorAlert("Parameter needs to be an ID")
        return
    
    typeID = int(content)

    if typeID < 0 or typeID >= Types.TypeAmount:
        await client.sendErrorAlert("Invalid type ID")
        return
    
    print(f"Setting type to {typeID}")
    await client.setType(typeID)


async def register(client: Client, content: str):
    if not client.isAdmin:
        await client.sendErrorAlert("Missing permissions!")
        return
    
    params = content.split(" ")

    if len(params) < 2:
        await client.sendErrorAlert("Not enough parameters.")
        return

    name = params[0]
    teamID = params[1]

    Session.cursor.execute("SELECT * FROM Users WHERE name = ?", (name,))
    userInfo = Session.cursor.fetchone()
    
    if userInfo != None:
        await client.sendErrorAlert("User already registered.")
        return

    Session.cursor.execute("INSERT INTO Users VALUES (?, ?, ?, ?)", (name.lower(), name, 0, teamID))
    Session.cx.commit()

    await client.send("/success")


async def serverLoop(websocket):
    client = Client(websocket)
    try:
        async for message in websocket:
            messageStr = str(message)
            await handleCommand(client, messageStr, "/getclients", getClients, True)
            await handleCommand(client, messageStr, "/exitlevels", exitLevels, True)
            await handleCommand(client, messageStr, "/broadcast", broadcast)
            await handleCommand(client, messageStr, "/completed", completed)
            await handleCommand(client, messageStr, "/register", register)
            await handleCommand(client, messageStr, "/login", login)
            await handleCommand(client, messageStr, "/auth", authCommand)
            await handleCommand(client, messageStr, "/ping", ping, True)
            await handleCommand(client, messageStr, "/play", playLevel)
            await handleCommand(client, messageStr, "/exit", exitCommand, True)
            await handleCommand(client, messageStr, "/to", toClient)
            await handleCommand(client, messageStr, "/settype", setType)
    except Exception as e:
        print(e)
        Client.clients.discard(client)
        await websocket.close()


async def main():
    while not Session.exiting:
        try:
            async with serve(serverLoop, "0.0.0.0", Session.port) as server:
                print("Serving on port", Session.port)
                await server.serve_forever()
        except Exception as e:
            print(e)
            Client.clients.clear()
        


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        Session.exiting = True