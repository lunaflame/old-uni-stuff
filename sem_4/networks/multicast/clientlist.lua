local core = require("core")

setfenv(1, _G)
ClientList = core.Emitter:new()
ClientList.clients = {}

local cls = ClientList.clients

function AddClient(addr)
	if cls[addr] then return cls[addr], false end -- already added, dont care

	cls[addr] = {}
	ClientList:emit("NewClient", addr)
	return cls[addr], true
end

function RemoveClient(addr)
	if not cls[addr] then
		p(cls)
		errorf("Tried to remove a client that was never there (%s)", addr)
	end

	cls[addr] = nil
	ClientList:emit("RemovedClient", addr)
end

function ListClients()
	print("Current clients:")
	for k,v in pairs(cls) do
		printf("\t%s", k)
	end
end

function IterClients()
	return pairs(cls)
end