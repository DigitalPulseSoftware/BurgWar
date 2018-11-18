local vec2meta = RegisterMetatable("vec2")
vec2meta.__index = vec2meta



function Vec2(x, y)
	local vec2 = {x = x or 0, y = y or 0}
	return setmetatable(vec2, vec2meta)
end
