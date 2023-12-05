local fs = require("fs")
local path = require("path")

require("./deps/promise")

local arg, err = parseArgs(args, {
	["--ip"] = {1, 2},
	["--port"] = 1,
	["--file"] = 1,
})

if not arg then
	exitf(err)
end

local ip, port

local ipArg = arg["--ip"]

if not ipArg then
	exitf("no IP/domain provided (use --ip)")
	return
end

--[==================================[
				extract IP
--]==================================]

local dnsProm, fsProm = Promise(), Promise()
local all = Promise.OnAll(dnsProm, fsProm)

do
	ip = ipArg[1]:gsub(":(%d+)$", "")

	local dns = require("dns")
	dns.resolve4(ip, function(err, t)
		if err then
			exitf("error during DNS resolve for %s: %s", ip, err)
			return
		end

		ip = t[1].address
		dnsProm:Resolve()
	end)
end

--[==================================[
			extract port
--]==================================]

do
	port = ipArg[1]:match(":(%d+)$") or ipArg[2] or arg["--port"]
	if not port then
		exitf("no port provided (use --port or embed into IP)")
		return
	end

	port = tonumber(port)

	if not port then
		exitf("no port provided (use --port or embed into IP)")
		return
	end

	if port < 1 or port > 65535 then
		exitf("port must be in range of [1-65535] (got: %s)", port)
	end
end

--[==================================[
		check file existence
--]==================================]
local fn
do
	fn = arg["--file"]
	fn = fn and fn[1]

	if not fn then
		exitf("no file provided (use --file)")
		return
	end

	fn = path.resolve(fn)

	fs.exists(fn, function(err, ex)
		if not ex then
			exitf("file `%s` does not exist", fn)
			return
		end

		fsProm:Resolve()
	end)
end

all:Then(function(_, dnsRet)
	assertf(ip, "no address resolved?")

	me.dest = {ip, port}
	me.file = fn

	require("./protocol")
end)