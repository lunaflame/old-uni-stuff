local dgram = require("dgram")
require("./shared.lua")
local cfg = _G.Config
local timer = require("timer")

if not cfg.is_host then return end

print("server sending!")
local server = dgram.createSocket(cfg.socketType)
server:setBroadcast(true)
server:addMembership(cfg.multicastIP)

server:on("error", print)
server:on("message", print)

server:send("Hi its a message from " .. jit.os, cfg.port, cfg.multicastIP)

timer.setInterval(cfg.replyFrequency, function()
	server:send("Hi please keep me alive thanks", cfg.port, cfg.multicastIP)
end)