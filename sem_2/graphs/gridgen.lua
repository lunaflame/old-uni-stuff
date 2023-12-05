local W, H = 10, 10
local Distance = 10
local EnableLabels = false

local edges = {}
local verts = {}

local function xyToID(x, y)
	return (x - 1) * H + y
end

for x = 1, W do
	verts[x] = {}
	for y = 1, H do
		verts[x][y] = xyToID(x, y)
	end
end

for x = 1, W do
	for y = 1, H do

		local cur_id = xyToID(x, y)

		if verts[x-1] and verts[x-1][y] then
			table.insert(edges, {xyToID(x - 1, y), cur_id, Distance})
		end

		if verts[x][y-1] then
			table.insert(edges, {xyToID(x, y - 1), cur_id, Distance})
		end

		if verts[x-1] and verts[x-1][y-1] then
			local dist = math.sqrt( Distance^2 * 2 )
			table.insert(edges, {xyToID(x - 1, y - 1), cur_id, dist})
		end
	end
end

local fs = require"fs"

local prvTbl = {
	"digraph {"
}

for _, dat in ipairs(edges) do
	local ins = ('	"%d" -> "%d"'):format(unpack(dat))
	if EnableLabels then
		ins = ins .. (' [label="%.1f"]'):format(dat[3])
	end
	table.insert(prvTbl, ins)
end

table.insert(prvTbl, "}")

local previewStr = table.concat(prvTbl, "\n")

local fd, err = fs.openSync("preview.txt", "w")
fs.writeSync(fd, 0, previewStr)

local gridTbl = {}

for _, dat in ipairs(edges) do
	local ins = ('%d %d %g'):format(unpack(dat))
	table.insert(gridTbl, ins)
end

local fd, err = fs.openSync("grid.txt", "w")
fs.writeSync(fd, 0, table.concat(gridTbl, "\n"))
