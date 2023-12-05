require("./clientlist")

setfenv(1, _G)
-- local IP = "2001:0db8:85a3:0000:0000:8a2e:0370:7334"

local cfg = _G.Config

function parseIP(IP)
	local outTyp
	-- parse out the ip and make sure its correct

	if not IP:match(":") and not IP:match("%.") then
		return false, "bad IP given (neither IPv4 nor IPv6)"
	end

	if IP:match(":") then -- IPv6?
		if IP:match("[^%x:]") then
			return false, "bad IPv6 given (illegal characters)"
		end

		local n = 0
		for s in IP:gmatch("(%x+):?") do
			--[[if #s ~= 4 then
				return false, ("bad IPv6 given (%d-th quadrant is %d chars long)"):format(n, #s)
			end]]

			n = n + 1
		end

		if n ~= 8 then
			return false, ("bad IPv6 given (given %d quadrants, need 8)"):format(n)
		end

		outTyp = "udp6"
	else
		if IP:match("[^%d.]") then
			return false, "bad IPv4 given (illegal characters)"
		end

		local n = 0
		for s in IP:gmatch("(%d+).?") do
			if tonumber(s) > 255 then
				return false, "bad IPv4 given (%dth quarant > 255)"
			end

			n = n + 1
		end

		if n ~= 4 then
			return false, ("bad IPv6 given (given %d quadrants, need 4)"):format(n)
		end

		outTyp = "udp4"
	end

	return outTyp
end