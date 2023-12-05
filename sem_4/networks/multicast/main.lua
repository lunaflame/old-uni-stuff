require("./shared.lua")

_G.Config = {
	port = 42000,

	interfaceIP = "192.168.1.225",
	socketType = nil,

	multicastIP = "230.69.13.37", -- "239.255.255.250"

	is_host = jit.os == "Linux",

	timeout = 3000, -- in ms. ; not receiving a keep-alive within this time will remove the connection
	timeoutPoll = 100, -- in ms. ; how often these timeouts are checked

	warnTimeout = 2000, -- in ms; once a client times out for this often, we print stuff
	-- mostly for debug, really

	replyFrequency = 500, -- in ms. ; keep-alives are sent to the server this often

	printMessages = false,
}

function _G.printf(f, ...) print(f:format(...)) end
function _G.errorf(...) error(f:format(...), 2) end
function _G.exitf(...) printf(...) os.exit() end

local parsers = {
	["--multicastip"] = function(v)
		local ok, why = parseIP(v)
		if not ok then
			exitf("Failed to parse --multicastIP: %s", why)
		end

		_G.Config.multicastIP = v
	end,
}

local flags = {
	["--idk"] = false,
}

local aliases = {
	["-m"] = "--multicastip",
}

local curFlag

for idx, arg in ipairs(args) do
	if arg:match("^-") and not curFlag then
		arg = arg:lower()
		arg = aliases[arg] or arg

		if parsers[arg] then
			curFlag = arg
		elseif flags[arg] ~= nil then
			flags[arg] = true
		else
			exitf("Unknown flag: %s", arg)
		end
	elseif curFlag then
		curFlag = parsers[curFlag] (arg)
	end
end

if curFlag then
	exitf("Unfinished parameter: %s", curFlag)
end

local typ, why = parseIP(Config.interfaceIP)
if not typ then
	exitf("Failed to parse interface IP: %s", why)
end

Config.socketType = typ

require("./broadcaster.lua")
require("./tracker.lua")

