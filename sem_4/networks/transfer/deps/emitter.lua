local Class = require("deps/class")
local pairs = pairs

local recursiveParentCopy = function(newobj, parent)
	--recursively copy every parent's listeners
	local events = newobj.__Events

	while parent do
		local ev = rawget(parent, "__Events")
		if ev then
			--loop over every [event_name] : { [ev_id] : {args} }
			-- 					 (name)				(parevs)

			for name, parevs in pairs(ev) do
				local myevs = events[name] or {}
				events[name] = myevs

				for id, args in pairs(parevs) do
					myevs[id] = args
				end
			end

		end

		parent = parent.__parent
	end

end

Emitter = Emitter or Class:callable()

Emitter.IsEmitter = true

function IsEmitter(what)
	return istable(what) and what.IsEmitter
end

function Emitter:Initialize()
	self.__Events = {}
	if not self.__instance and self ~= Emitter then setmetatable(self, Emitter) end

	if self.__instance and self.__instance.__Events then
		local par = self.__instance
		recursiveParentCopy(self, par)
	end
end

function Emitter:__tostring()
	local evs = {}
	for k,v in pairs(self.__Events) do evs[#evs + 1] = k end

	return ("Emitter %p [%s]"):format(self, table.concat(evs, ", "))
end

function Emitter:OnExtend(new)
	new.__Events = {}
	recursiveParentCopy(new, self)
end

function Emitter.Make(t)
	Emitter.Initialize(t)
	return t
end

Emitter.make = Emitter.Make

function Emitter:On(event, name, cb, ...)
	self.__Events = self.__Events or {}
	local events = self.__Events

	events[event] = events[event] or {}
	local theseEvs = events[event]

	local vararg

	if isfunction(name) then
		vararg = cb
		cb = name
		name = #theseEvs + 1

		local t = {cb, vararg, ...}
		theseEvs[name] = t
	else
		if name == nil then
			name = #theseEvs + 1
			ErrorNoHalt("Adding an Emitter listener with name == nil!\n")
		end
		theseEvs[name] = {cb, ...}
	end

	return name
end

function Emitter:OnAny(events, name, cb, ...)
	CheckArg(1, events, istable, "table of events")

	for _, evName in ipairs(events) do
		self:On(evName, name, cb, ...)
	end
end

function Emitter:OnceAny(events, name, cb, ...)
	CheckArg(1, events, istable, "table of events")

	for _, evName in ipairs(events) do
		self:Once(evName, name, cb, ...)
	end
end

function Emitter:Once(event, name, cb, ...)

	if isfunction(name) then
		local name2

		name2 = self:On(event, function(...)
			self:RemoveListener(event, name2)
			name(...)
		end, ...)
	else
		self:On(event, name, function(...)
			self:RemoveListener(event, name)
			cb(...)
		end, ...)
	end

end

function Emitter:Emit(event, ...)
	local events = self.__Events
	if not events then return end

	local evs = events[event]
	if not evs then return end

	for k,v in pairs(evs) do
		--if event name isn't a string, isn't a number and isn't valid then bail
		if not (isstring(k) or isnumber(k) or k:IsValid()) then
			evs[k] = nil
		else
			--v[1] is the callback function
			--every other key-value is what was passed by On

			if #v > 1 then --AHHAHAHAHAHAHAHAHAHAHHA
				local t = {unpack(v, 2)}
				table.InsertVararg(t, ...)

				local a, b, c, d, e, why = v[1](self, unpack(t))
				--hook.Call intensifies
				if a ~= nil then
					return a, b, c, d, e, why
				end
			else
				local a, b, c, d, e, why = v[1](self, ...)
				if a ~= nil then
					return a, b, c, d, e, why
				end
			end
		end
	end
end

function Emitter:GEmit(errName, event, ...)
	local events = self.__Events
	if not events then return end

	local evs = events[event]
	if not evs then return end

	for k,v in pairs(evs) do
		if not (isstring(k) or isnumber(k) or k:IsValid()) then
			evs[k] = nil
		else
			if #v > 1 then
				local t = {unpack(v, 2)}
				table.InsertVararg(t, ...)

				local a, b, c, d, e, why = gpcall(errName, v[1], self, unpack(t))
				if a and b ~= nil then
					return b, c, d, e, why
				end
			else
				local a, b, c, d, e, why = gpcall(errName, v[1], self, ...)
				if a and b ~= nil then
					return b, c, d, e, why
				end
			end
		end
	end

end

function Emitter:RemoveListeners(event, name)
	if not self.__Events then return end

	if name ~= nil then
		local these = self.__Events[event]
		if these then these[name] = nil end
	else
		self.__Events[event] = nil
	end
end

Emitter.RemoveListener = Emitter.RemoveListeners

function Emitter:GetListeners(event, name)
	local evs = self.__Events
	if not evs then return false end

	if name then return evs[event] and evs[event][name] end
	return evs[event]
end

Emitter.GetListener = Emitter.GetListeners

function MakeEmitter(t)
	t.On = Emitter.On
	t.Emit = Emitter.Emit
	t.Once = Emitter.Once
	t.RemoveListener = Emitter.RemoveListener
	t.RemoveListeners = Emitter.RemoveListener
	t.GetListener = Emitter.GetListener
	t.GetListeners = Emitter.GetListener
end

return Emitter