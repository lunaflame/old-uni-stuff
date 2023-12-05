local net = require("net")
local uv = require("uv")
local timer = require("timer")
local fs = require("fs")
local path = require("path")

local server
server = net.createServer(function(...) server:emit("NewClient", ...) end)

server:on("error", error)
server:listen(me.port)
server.Clients = {}

server:on("NewClient", function(cl)
	print("New client...")

	cl.recvSize = 0
	cl.headerBuf = ""

	cl.flushBuf = ""
	cl.flushOff = 0

	cl.speedRecv = 0
	cl.when = uv.now()

	cl:on("data", function(...) server:OnClientData(cl, ...) end)
	cl:on("end", function(...) server:OnClientDisconnect(cl, ...) end)

	cl.OnFlushFile = function(...)
		cl.flushing = false
		server:FlushFile(cl, ...)
	end
end)

local ffi = require("ffi") -- PAIN

-- endianness is a myth
local union = ffi.typeof([[
	union {
		char bytes[8];
		uint64_t integer;
	}
]]) ()

fs.mkdirSync("uploads/")

local MiB = 1048576
function server:OnClientData(cl, dat)
	if not cl.needSize then
		cl.headerBuf = cl.headerBuf .. dat

		-- incomplete header; ignore until it arrives
		local nullWhere = cl.headerBuf:find("%z")
		if not nullWhere or #cl.headerBuf - nullWhere < 9 then return end
		-- (filename) .. \0 .. (uint64_size)

		local fn = cl.headerBuf:sub(1, nullWhere - 1)
		fn = fn:gsub("%.%.", "")

		local sz = cl.headerBuf:sub(nullWhere + 1, nullWhere + 9)

		union.bytes = sz
		sz = union.integer

		cl.needSize = sz
		cl.fn = fn

		dat = cl.headerBuf:sub(nullWhere + 9)
		cl.headerBuf = nil
		self.Clients[cl] = true

		local saveFn = path.join("uploads/", fn)
		cl.savePath = saveFn

		printf("Receiving file: %s (%.2fMiB)", saveFn, tonumber(sz) / MiB)
		local fd, err = fs.openSync(saveFn, "w")
		cl.saveFd = fd

		if err then
			printf("\tFailed to open file: `%s` (%s)", saveFn, err)
			cl:destroy()
			return
		end
	end

	cl.recvSize = cl.recvSize + #dat
	cl.speedRecv = cl.speedRecv + #dat
	cl.flushBuf = cl.flushBuf .. dat

	self:FlushFile(cl)
end

function server:FlushFile(client)
	if client.flushing then return end
	if #client.flushBuf == 0 then return end

	client.flushing = true

	fs.write(client.saveFd, client.flushOff, client.flushBuf, client.OnFlushFile)

	client.flushOff = client.flushOff + #client.flushBuf
	client.flushBuf = ""
end

function server:OnClientDisconnect(cl)
	cl.finished = uv.now()

	if cl.recvSize ~= cl.needSize then
		for i=1, 3 do
			printf("\t!!! Mismatched size, required: %s, have: %s !!!",
				cl.needSize, cl.recvSize)
		end

		print("\tDeleted the incomplete file.")
		fs.unlink(cl.savePath)
	else
		printf("Saved file `%s`", cl.fn)
	end
end

timer.setInterval(3000, function()
	local now = uv.now()

	for cl in pairs(server.Clients) do
		local recv = cl.speedRecv
		local passed = (math.min(cl.finished or math.huge, now) - cl.when) / 1000

		local curSpeed = recv / 3 -- how many bytes received since last print (= instant speed)
		local avgSpeed = cl.recvSize / passed
		printf("`%s`: %.2fMiB/%.2fMiB", cl.fn, cl.recvSize / MiB, tonumber(cl.needSize) / MiB)
		printf("\tCurrent speed: %.2fMiB/s.\tAverage speed: %.2fMiB/s.", curSpeed / MiB, avgSpeed / MiB)

		cl.speedRecv = 0
		if cl.finished then
			server.Clients[cl] = nil
		end
	end
end)