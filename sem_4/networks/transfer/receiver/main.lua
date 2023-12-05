local arg, err = parseArgs(args, {
	["--port"] = 1,
})

if not arg then
	exitf(err)
end

local port = arg["--port"]
port = port and port[1]

if port and not tonumber(port) then
	exitf("port must be a number (got: `%s`)", port)
end

if not port then
	for k,v in ipairs(arg) do
		if tonumber(v) then
			port = tonumber(v)
			printf("interpreting `%s` as port (use of --port is preferred)", port)
		end
	end

	if not port then
		exitf("no port provided (use --port)")
		return
	end
end

port = tonumber(port)
if port < 1 or port > 65535 then
	exitf("port must be in range of [1-65535] (got: %s)", port)
end

me.port = port
printf("using port %s", port)
require("./protocol")