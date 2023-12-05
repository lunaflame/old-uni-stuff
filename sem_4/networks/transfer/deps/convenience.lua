function _G.printf(f, ...) 			print(f:format(...)) 			end
function _G.errorf(f, ...) 			error(f:format(...), 2) 		end
function _G.exitf(...) 				printf(...) os.exit() 			end
function _G.assertf(c, fmt, ...) 	assert(c, fmt:format(...))	end

local types = {
	["number"] = true,
	["string"] = true,
	["function"] = true,
	["table"] = true,
	["boolean"] = true,
	["bool"] = "boolean",
}

for k,v in pairs(types) do
	local typ = (v ~= true and v) or k

	_G["is" .. k] = function(val) return type(val) == typ end
end

function _G.parseArgs(t, consumes, aliases)
	t = t or args
	consumes = consumes or {}

	local curFlag

	local out = {
		-- [1] = standalone_arg, [2] = standalone_arg...
		-- ["paramname"] = {...}
	}

	local curArgs = {}

	for _, arg in ipairs(t) do
		local isParam = arg:match("^%-")
		local minConsume, maxConsume = 0, 0

		do
			local c = consumes[curFlag]
			if istable(c) then
				minConsume, maxConsume = c[1], c[2]
			elseif isnumber(c) then
				minConsume, maxConsume = c, c
			end
		end

		local toEat = maxConsume - #curArgs

		if isParam then
			if curFlag then
				if toEat > minConsume then
					return false, ("%s requires %s args, got %s"):format(curFlag, consumes[curFlag] or 0, #curArgs)
				end

				-- flush
				out[curFlag] = curArgs
				curArgs = {}
			end

			curFlag = arg
		else
			if toEat > 0 then
				-- add this arg to the flag's arglist
				table.insert(curArgs, arg)
			else
				-- standalone arg?
				table.insert(out, arg)
			end
		end

		if curFlag then out[curFlag] = curArgs end
	end

	return out
end

function coroutine.Resumer()
	local cor = coroutine.running()

	return function(...)
		local ok, err = coroutine.resume(cor, ...)
		if not ok then
			errorf("coroutine.Resumer error: %s", err)
		end
	end
end