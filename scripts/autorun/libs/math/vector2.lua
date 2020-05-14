RegisterClientScript()

local vec2meta = RegisterMetatable("vec2")
vec2meta.__index = vec2meta

function vec2meta:Distance(vec)
	return (self - vec):Length()
end

function vec2meta:DistanceTest(vec, distance)
	return self:SquaredDistance(vec) < distance*distance
end

function vec2meta:GetNormalized()
	local length = self:Length()
	local normal = Vec2(self.x / length, self.y / length)
	return normal, length
end

function vec2meta:Length()
	return math.sqrt(self:SquaredLength())
end

function vec2meta:Normalize()
	local length = self:Length()
	self.x = self.x / length
	self.y = self.y / length

	return length
end

function vec2meta:SquaredLength()
	return self.x * self.x + self.y * self.y
end

function vec2meta:SquaredDistance(vec)
	return (self - vec):SquaredLength()
end

function vec2meta:__add(other)
	if (getmetatable(other) == vec2meta) then
		return Vec2(self.x + other.x, self.y + other.y)
	else
		error("Must be a vector2")
	end
end

function vec2meta.__mul(lhs, rhs)
	if (getmetatable(lhs) == vec2meta) then
		if (getmetatable(rhs) == vec2meta) then
			return Vec2(lhs.x * rhs.x, lhs.y * rhs.y)
		elseif (type(rhs) == "number") then
			return Vec2(lhs.x * rhs, lhs.y * rhs)
		end
	elseif (type(lhs) == "number") then
		assert(getmetatable(rhs) == vec2meta)

		return Vec2(lhs * rhs.x, lhs * rhs.y)
	else
		error("Factors must be a vector2 or a number")
	end
end

function vec2meta.__div(lhs, rhs)
	if (getmetatable(lhs) == vec2meta) then
		if (getmetatable(rhs) == vec2meta) then
			return Vec2(lhs.x / rhs.x, lhs.y / rhs.y)
		elseif (type(rhs) == "number") then
			return Vec2(lhs.x / rhs, lhs.y / rhs)
		end
	elseif (type(lhs) == "number") then
		assert(getmetatable(rhs) == vec2meta)

		return Vec2(lhs / rhs.x, lhs / rhs.y)
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
