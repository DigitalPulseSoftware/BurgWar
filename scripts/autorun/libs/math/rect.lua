RegisterClientScript()

local rectmeta = RegisterMetatable("rect")
rectmeta.__index = rectmeta

function rectmeta:ComputeArea()
	return self.width * self.height
end

function rectmeta:ComputeIntersection(rect)
	AssertMetatable(rect, "rect")

	local left = math.max(self.x, rect.x)
	local right = math.min(self.x + self.width, rect.x + rect.width)
	if (left >= right) then
		return nil
	end

	local top = math.max(self.y, rect.y)
	local bottom = math.min(self.y + self.height, rect.y + rect.height)
	if (top >= bottom) then
		return nil
	end

	return Rect(Vec2(left, top), Vec2(right, bottom))
end

function rectmeta:Contains(pos)
	AssertMetatable(pos, "vec2")

	return pos.x >= self.x and pos.y >= self.y and
	       pos.x < self.x + self.width and pos.y < self.y + self.height
end

function rectmeta:ExtendToPosition(pos)
	local newWidth = math.max(self.x + self.width, pos.x)
	local newHeight = math.max(self.y + self.height, pos.y)

	self.x = math.min(self.x, pos.x)
	self.y = math.min(self.y, pos.y)
	
	self.width = newWidth - self.x
	self.height = newHeight - self.y
end

function rectmeta:ExtendToRect(rect)
	local newWidth = math.max(self.x + self.width, rect.x + rect.width)
	local newHeight = math.max(self.y + self.height, rect.y + rect.height)

	self.x = math.min(self.x, rect.x)
	self.y = math.min(self.y, rect.y)
	
	self.width = newWidth - self.x
	self.height = newHeight - self.y
end

function rectmeta:GetCenter()
	return Vec2(self.x + self.width * 0.5, self.y + self.height * 0.5)
end

function rectmeta:GetCorner(maxX, maxY)
	local x = self.x
	if (maxX) then
		x = x + self.width
	end

	local y = self.y
	if (maxY) then
		y = y + self.height
	end

	return Vec2(x, y)
end

function rectmeta:Intersect(rect)
	AssertMetatable(rect, "rect")

	local left = math.max(self.x, rect.x)
	local right = math.min(self.x + self.width, rect.x + rect.width)
	if (left >= right) then
		return false
	end

	local top = math.max(self.y, rect.y)
	local bottom = math.min(self.y + self.height, rect.y + rect.height)
	if (top >= bottom) then
		return false
	end

	return true
end

function rectmeta:GetPosition()
	return Vec2(self.x, self.y)
end

function rectmeta:GetSize()
	return Vec2(self.width, self.height)
end

function rectmeta:Scale(scale)
	local diffX = self.width * scale - self.width
	local diffY = self.height * scale - self.height

	self.x = self.x - diffX * 0.5
	self.y = self.y - diffY * 0.5
	self.width = self.width + diffX
	self.height = self.height + diffY
end

function rectmeta:__tostring()
	return "rect(" .. tostring(self.x) .. ", " .. tostring(self.y) .. ", " .. tostring(self.width) .. ", " .. tostring(self.height) .. ")"
end


function Rect(mins, maxs)
	if (not mins and not maxs) then
		return setmetatable({x = 0, y = 0, width = 0, height = 0}, rectmeta)
	end

	AssertMetatable(mins, "vec2")
	AssertMetatable(maxs, "vec2")

	local rect = {
		x = math.min(mins.x, maxs.x),
		y = math.min(mins.y, maxs.y),
		width = (maxs.x > mins.x) and (maxs.x - mins.x) or (mins.x - maxs.x),
		height = (maxs.y > mins.y) and (maxs.y - mins.y) or (mins.y - maxs.y)
	}
	return setmetatable(rect, rectmeta)
end
