local Emitter = require("deps/emitter")

--[[
	Promise:
		i dont fucking remember
]]
Promise = Promise or Emitter:Callable()
Promise.IsPromise = true

function IsPromise(what)
	return istable(what) and what.IsPromise
end

function Promise:Initialize()
	self._success = {}
	self._branches = {}
	self._fail = {}

	self._curFillStep = 0
	self._curRunStep = 1

	self._Resolved = false
	self._Rejected = false
end

function Promise:Rewind()
	self._curRunStep = 1
end

function Promise:Reset()
	self._success = {}
	self._fail = {}
	self._branches = {}
	self._curFillStep = 0

	self:Rewind()
end

function Promise:IsFinished()
	return self._Resolved or self._Rejected
end

function Promise:IsResolved()
	return self._Resolved
end

function Promise:Then(full, rej)
	self._curFillStep = self._curFillStep + 1
	local key = self._curFillStep

	local nextPr = Promise()

	if isfunction(full) then
		nextPr = Promise()
		self._success[key] = coroutine.create(full)
		self._branches[key] = nextPr
	end

	if isfunction(rej) then
		self._branches[key] = nextPr
		self._fail[key] = coroutine.create(rej)
	end

	-- then'd an already resolved promise; instantly run it
	if self._Resolved then
		self:_run(unpack(self._Resolved, 1, table.maxn(self._Resolved)))
	elseif self._Rejected then
		self:_run(self._Rejected)
	end

	self:Emit("ThenAdded", full, rej)

	return nextPr
end

function Promise:_run(...)
	if self._running then errorf("Can't run promise %s again", self) return end
	local s, f = self._success, self._fail
	self._running = true

	while s[self._curRunStep] or f[self._curRunStep] do
		local key = self._curRunStep
		self._curRunStep = self._curRunStep + 1

		if not self._Rejected then
			if self._success[key] then
				local rets = { coroutine.resume(self._success[key], self, ...) }
				if not rets[1] then errorf("Promise error: %s", rets[2]) return end

				if self._branches[key] and table.maxn(rets) > 1 then
					self._branches[key]:Resolve(unpack(rets, 2))
				end
			end
		else
			if self._fail[key] then
				local rets = { coroutine.resume(self._fail[key], self, unpack(self._Rejected)) }
				if not rets[1] then errorf("Promise error: %s", rets[2]) return end

				if self._branches[key] then
					self._branches[key]:Reject(unpack(self._Rejected))
				end
			end
		end
	end

	self._running = false
	self:Emit(self._Rejected and "Rejected" or "Resolved", ...)
end

function Promise:__tostring()
	return ("Promise/%s(%d, %p)"):format(
		self._running and "running" or
			self._Rejected and "rejected" or
			self._Resolved and "resolved" or
			"suspended",
		#self._branches, self)
end

-- passed into the callbacks, intended to be ran for async
-- seems like in JS resolving a resolved promise is not a bug; it'll simply not do anything
-- sounds good to me, man

function Promise:Resolve(x, ...)
	if x == self then error("fuck you a+ [you passed self into resolve]") return end

	if self._Rejected then
		errorf("Can't resolve a rejected promise %s!", self)
		return
	end

	if self:IsFinished() then return end

	if IsPromise(x) and not x._running then
		x:Then(self:Resolver(), self:Rejector())
	else
		self._Resolved = {x, ...}
		return self:_run(x, ...)
	end
end

function Promise:Reject(...)
	if self:IsFinished() then
		return
	end

	self._Rejected = {...}
	return self:_run(err)
end

-- async functions suck and dont let you pass args in? got you covered fam
function Promise:Resolver()
	self._resolver = self._resolver or function(...)
		return self:Resolve(...)
	end

	return self._resolver
end

function Promise:Rejector()
	self._rejector = self._rejector or function(...)
		return self:Reject(...)
	end

	return self._rejector
end

Promise.Rejecter = Promise.Rejector -- lole

function Promise:Decide(ok, ...)
	if ok then self:Resolve(...) else self:Reject(...) end
end

local function typecheck(tbl)
	for k,v in ipairs(tbl) do
		if not IsPromise(v) then
			errorf("not a promise @ #%d (%q: %s)", k, type(v), v)
			return
		end
	end
end

local function toTbl(tbl, ...)
	local prs = {}

	if istable(tbl) and not IsPromise(tbl) then
		prs = tbl
	elseif IsPromise(tbl) then
		prs[1] = tbl
		for i=1, select("#", ...) do
			prs[i + 1] = select(i, ...)
		end
	end

	typecheck(tbl)

	return prs
end

function Promise.OnAll(tbl, ...)
	local prs = toTbl(tbl, ...)

	local left = #prs
	local prRets = {}

	local retPr = Promise()
	local imdeadbru = false

	local supa_secret_key = ("onAll_key:%p"):format(prs)

	local function checkDone()
		if left == 0 then
			if imdeadbru then
				retPr:Reject(prRets)
			else
				retPr:Resolve(prRets)
			end
		end
	end

	checkDone()

	local function incrResolve(self, ...)
		left = left - 1
		prRets[self[supa_secret_key]] = {...}
		checkDone()

		self[supa_secret_key] = nil
	end

	local function incrReject(self, ...)
		imdeadbru = true
		left = left - 1
		prRets[self[supa_secret_key]] = {...}
		checkDone()

		self[supa_secret_key] = nil
	end

	for k,v in ipairs(prs) do
		v[supa_secret_key] = k
		v:Then(incrResolve, incrReject)
	end

	return retPr
end

function Promise.OnFirst(tbl, ...)
	local prs = toTbl(tbl, ...)

	local retPr = Promise()
	local resolved = false

	local function survival_of_the_fastest(self, ...)
		if resolved then return end
		resolved = true
		retPr:Resolve(...)
	end

	for k,v in ipairs(prs) do
		v:Then(survival_of_the_fastest)
	end

	return retPr
end

return Promise