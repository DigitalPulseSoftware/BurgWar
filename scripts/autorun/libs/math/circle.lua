RegisterClientScript()

local circlemeta = RegisterMetatable("circle")
circlemeta.__index = circlemeta



function Circle(center, radius)
	AssertMetatable(center, "vec2")
	assert(type(radius) == "number", "vec2")

	local circle = {
		origin = center,
		radius = radius
	}
	return setmetatable(circle, circlemeta)
end
