RegisterClientScript("vector2.lua")

local vec2meta = RegisterMetatable("vec2")
vec2meta.__index = vec2meta

function vec2meta:__add(other)
	if (getmetatable(other) == vec2meta) then
		return Vec2(self.x + other.x, self.y + other.y)
	else
		error("Must be a vector")
	end
end

function vec2meta:__mul(factor)
	if (type(factor) == "number") then
		return Vec2(self.x * factor, self.y * factor)
	else
		error("Factor must be a number")
	end
end

function vec2meta:__div(divider)
	if (type(divider) == "number") then
		return Vec2(self.x / divider, self.y / divider)
	else
		error("Divider must be a number")
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
