local vec2meta = RegisterMetatable("vec2")
vec2meta.__index = vec2meta

function vec2meta:__div(divider)
	if (type(divider) == "number") then
		return Vec2(self.x / divider, self.y / divider)
	else
		error("Divider must be a number")
	end
end

function Vec2(x, y)
	local vec2 = {x = x or 0, y = y or 0}
	return setmetatable(vec2, vec2meta)
end
