RegisterClientScript()

local vec2meta = RegisterMetatable("vec2")
vec2meta.__index = vec2meta

function vec2meta:Length()
	return math.sqrt(self:SquaredLength())
end

function vec2meta:Normalize()
	local length = self:Length()
	self.x = self.x / length
	self.y = self.y / length

	return self, length
end

function vec2meta:SquaredLength()
	return self.x * self.x + self.y * self.y
end

function vec2meta:__add(other)
	if (getmetatable(other) == vec2meta) then
		return Vec2(self.x + other.x, self.y + other.y)
	else
		error("Must be a vector2")
	end
end

function vec2meta:__mul(other)
	if (getmetatable(other) == vec2meta) then
		return Vec2(self.x * other.x, self.y * other.y)
	elseif (type(other) == "number") then
		return Vec2(self.x * other, self.y * other)
	else
		error("Factor must be a vector2 or a number")
	end
end

function vec2meta:__div(divider)
	if (getmetatable(divider) == vec2meta) then
		return Vec2(self.x / divider.x, self.y / divider.y)
	elseif (type(divider) == "number") then
		return Vec2(self.x / divider, self.y / divider)
	else
		error("Divider must be a vector2 or a number")
	end
end

function vec2meta:__sub(other)
	if (getmetatable(other) == vec2meta) then
		return Vec2(self.x - other.x, self.y - other.y)
	else
		error("Must be a vector")
	end
end

function vec2meta:__unm()
	return Vec2(-self.x, -self.y)
end

function vec2meta:__tostring()
	return "vec2(" .. tostring(self.x) .. ", " .. tostring(self.y) .. ")"
end

function Vec2(x, y)
	local vec2 = {x = x or 0, y = y or 0}
	return setmetatable(vec2, vec2meta)
end
