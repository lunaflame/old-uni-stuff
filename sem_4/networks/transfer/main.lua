local uv = require("uv")
require("./deps/convenience")
_G.process = require("process").globalProcess()

CLIENT = {
	reqPath = "./sender/main.lua",
}

SERVER = {
	reqPath = "./receiver/main.lua",
}

local acceptable = {
	client 		= CLIENT,
	cl 			= CLIENT,
	sender 		= CLIENT,
	send 		= CLIENT,

	server 		= SERVER,
	sv 			= SERVER,
	recv 		= SERVER,
	receiver 	= SERVER
}

local mode

for k,v in pairs(args) do
	local cur = acceptable[v]

	if cur then
		if mode and mode ~= cur then
			errorf("already selected mode `%s`; tried to switch to %s", mode, cur)
		end

		mode = cur
	end
end

if not mode then
	printf("No mode selected; select either `client` or `server`.")
	return
end

_G.me = mode
require(mode.reqPath)

uv.run()