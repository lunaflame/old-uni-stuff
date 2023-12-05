
local input = [[
    A   T   G   C   S   W   R   Y   K   M   B   V   H   D   N
A   5  -4  -4  -4  -4   1   1  -4  -4   1  -4  -1  -1  -1  -2
T  -4   5  -4  -4  -4   1  -4   1   1  -4  -1  -4  -1  -1  -2
G  -4  -4   5  -4   1  -4   1  -4   1  -4  -1  -1  -4  -1  -2
C  -4  -4  -4   5   1  -4  -4   1  -4   1  -1  -1  -1  -4  -2
S  -4  -4   1   1  -1  -4  -2  -2  -2  -2  -1  -1  -3  -3  -1
W   1   1  -4  -4  -4  -1  -2  -2  -2  -2  -3  -3  -1  -1  -1
R   1  -4   1  -4  -2  -2  -1  -4  -2  -2  -3  -1  -3  -1  -1
Y  -4   1  -4   1  -2  -2  -4  -1  -2  -2  -1  -3  -1  -3  -1
K  -4   1   1  -4  -2  -2  -2  -2  -1  -4  -1  -3  -3  -1  -1
M   1  -4  -4   1  -2  -2  -2  -2  -4  -1  -3  -1  -1  -3  -1
B  -4  -1  -1  -1  -1  -3  -3  -1  -1  -3  -1  -2  -2  -2  -1
V  -1  -4  -1  -1  -1  -3  -1  -3  -3  -1  -2  -1  -2  -2  -1
H  -1  -1  -4  -1  -3  -1  -3  -1  -3  -1  -2  -2  -1  -2  -1
D  -1  -1  -1  -4  -3  -1  -1  -3  -1  -3  -2  -2  -2  -1  -1
N  -2  -2  -2  -2  -1  -1  -1  -1  -1  -1  -1  -1  -1  -1  -1]]


local mtrx = {}
local numToNuke = {}
local nukeToNum = {}

local lNum = 0


for line in input:gmatch("[^\r\n]+") do
	if lNum == 0 then
		for vx in line:gmatch("%w") do
			mtrx[vx] = {}
			local key = #numToNuke + 1
			numToNuke[key] = vx
			nukeToNum[vx] = key
		end

		goto cont
	end

	do
		local yNuke = line:match("^%w")
		local scoreNum = 0

		for score in line:gmatch("[%-%d]+") do
			scoreNum = scoreNum + 1
			local xNuke = numToNuke[scoreNum]
			local s = ("%s-%s = %d"):format(xNuke, yNuke, score)

			mtrx[xNuke][yNuke] = score
		end
	end

	::cont::	-- mfw no continue in lua
	lNum = lNum + 1
end

local numToNukeSz = #numToNuke

for i=0, 255 do
	if not nukeToNum[string.char(i)] then
		numToNuke[#numToNuke + 1] = {i, -1}
	end
end

local s = [[
const size_t mxSize = %d;
const char XtoXY[%d] = {
%s
};
]]

local s2 = [[
const char score_mtrx[%d] = {
	%s
};
]]

local XtoXY = ""
local arrS = ""

for k,v in ipairs(numToNuke) do
	if type(v) == "string" then
		local s = "	[%d] = %d,	// '%s'\n"
		XtoXY = XtoXY .. s:format(v:byte(), k - 1, v)
	else
		local s = "	[%d] = %d,"
		XtoXY = XtoXY .. s:format(v[1], -1)
	end
end
XtoXY = XtoXY:gsub("\n$", "")

local ruler = 80
local curLen = 0

local arrInit = {}

for xLetter, data in pairs(mtrx) do
	for yLetter, score in pairs(data) do
		arrInit[#arrInit + 1] = {(nukeToNum[xLetter] - 1) * numToNukeSz + (nukeToNum[yLetter] - 1), score}
	end
	--s = s .. "\n"
end


table.sort(arrInit, function(a, b) return a[1] < b[1] end)

for k,v in ipairs(arrInit) do
	local append = ("[%3d]=%2d,	"):format(v[1], v[2])
	if curLen + #append > ruler then
		arrS = arrS .. "\n	"
		curLen = 0
	end
	arrS = arrS .. append
	curLen = curLen + #append
end

s = s:format( numToNukeSz, 255, XtoXY )
s2 = s2:format( numToNukeSz ^ 2, arrS )
print(s)
print(s2)