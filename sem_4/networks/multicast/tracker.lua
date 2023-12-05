local dgram = require("dgram")
local timer = require("timer")
local uv = require("uv")

local cfg = _G.Config

require("./shared.lua")

-- listener socket
local listen = dgram.createSocket(cfg.socketType)
listen:on("error", print)

listen:bind(cfg.port, cfg.interfaceIP)
listen:addMembership(cfg.multicastIP)

listen:on("message", function(msg, who, flags)
	if cfg.printMessages then
		local tx = ("| From: %s:%s |"):format(who.ip, who.port)

		local spaces = (#tx - #" Message ") / 2
		printf("%s Message %s", ("-"):rep(spaces), ("-"):rep(spaces))
		printf(tx)
		printf(("-"):rep(#tx))

		local tx2 = msg
		printf(tx2)
		printf(("-"):rep(#tx))
	end

	local client, isNew = AddClient(("%s:%s"):format(who.ip, who.port))
	client.lastReply = uv.now()

	if cfg.printMessages or isNew then
		ListClients()
	end
end)


timer.setInterval(cfg.timeoutPoll, function()
	local now = uv.now()
	local changed = false

	for ip, cl in IterClients() do
		local passed = now - cl.lastReply

		if passed > cfg.timeout then
			-- we didn't receive anything from them within this timeout period; DUMP EET
			RemoveClient(ip)
			changed = true
		elseif passed > cfg.warnTimeout then
			print(ip, "is timing out...!", passed)
		end
	end

	if changed then
		print("New client list:")
		ListClients()
	end
end)