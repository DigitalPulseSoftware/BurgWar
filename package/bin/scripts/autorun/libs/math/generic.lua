RegisterClientScript()

function math.clamp(val, min, max)
	return math.min(math.max(val, min), max)
end

function math.lerp(from, to, factor)
	return from + factor * (to - from)
end
