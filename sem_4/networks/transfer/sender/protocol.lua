local uv = require("uv")
local net = require("net")
local fs = require("fs")
local path = require("path")

local dest = me.dest
local fp = me.file

local client = net.createConnection(me.dest[2], me.dest[1], coroutine.Resumer())

client:on("error", function(err)
	if err then
		exitf("error: %s", err)
		return
	end
end)

-- while we wait for the socket to connect we can do FS stuff synchronously
local fd = fs.openSync(fp, "r")
local stat, err = fs.statSync(fp)
if not stat then errorf("what %s", err) return end

local err, wat = coroutine.yield()

if err then
	print("error:", err)
	return
end


assertf(fd, "failed to open file!? this should have been checked!!! (%s)", fp)

local readsend
local total = 0

local function send(err, read)
	if #read == 0 then
		print("total sent:", total)
		return
	end

	if err then errorf("failed reading file: %s", err) return end

	total = total + #read
	-- printf("sending %s bytes... (%d-%d-%d-%d)", #read, read:byte(1, 4))

	client:write(read, function(err)
		if err then print("write error:", err) return end

		readsend()
	end)
end

local cursor = 0
function readsend()
	fs.read(fd, 65536, cursor, send)
	cursor = cursor + 65536
end

local header

do
	local ffi = require("ffi") -- PAIN

	-- not sure if there's a way to get a ptr to a scalar ? ? ?
	local ull = ffi.new("uint64_t[1]", stat.size) -- ffi.new("uint64_t[1]", stat.size)
	local bytes = ffi.cast("char*", ull) -- ffi.string(ull, 8)
	bytes = ffi.string(bytes, 8)

	header = path.basename(fp)
		.. "\0" .. bytes
end

client:write(header, function(err)
	if err then print("write error:", err) return end
	readsend()
end)
