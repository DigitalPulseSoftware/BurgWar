RegisterClientScript()

local rectmeta = RegisterMetatable("rect")
rectmeta.__index = rectmeta


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
