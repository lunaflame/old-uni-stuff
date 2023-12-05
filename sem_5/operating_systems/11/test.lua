local uv = require("uv")
local childprocess = require("childprocess")

local checks = 250
local simul = 4

local name = args[#args]
if not name or name:match("%.lua$") then
	print("pass the executable name")
	return
end

print("testing", name)

coroutine.wrap(function()
	local coro = coroutine.running()

	for go = 1, checks do
		for sim = 1, simul do
			local proc = childprocess.spawn(name)
			local fullStr = ""
			proc.stdout:on("data", function(str)
				fullStr = fullStr .. str
			end)

			proc.stdout:on("close", function(...)
				if not fullStr:match("[\r\n]") then
					print("epic fail (no output?)", fullStr)
					return
				end

				local lastLine
				local linesAmt = 0

				for s in fullStr:gmatch("[^\r\n]+") do
					if s == lastLine -- one print repeated twice
						or linesAmt == 0 and s ~= "Parent print" -- wrong print started
						then
						print("epic fail", fullStr)
						return
					end

					lastLine = s
					linesAmt = linesAmt + 1
				end

				if (linesAmt ~= 20) then
					print("epic fail (" .. linesAmt .. "/20 lines)")
					return
				end

				coroutine.resume(coro)
			end)
		end

		for i=1, simul do
			coroutine.yield()
		end
		print("check #" .. go .. " complete, all good (*" .. simul .. " simultaneous)")
	end

	print(("-- %s total processes ran, all passed --"):format(checks * simul))
	os.exit()
end)()