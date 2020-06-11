RegisterClientScript()

local segmentMeta = RegisterMetatable("segment")
segmentMeta.__index = segmentMeta


function segmentMeta:__tostring()
	return "segment(" .. tostring(self.first) .. "; " .. tostring(self.second) .. ")"
end


function Segment(from, to)
	AssertMetatable(from, "vec2")
	AssertMetatable(to, "vec2")

	local segment = {
		from = from,
		to = to
	}
	return setmetatable(segment, segmentMeta)
end
