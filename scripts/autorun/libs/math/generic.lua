RegisterClientScript()

function math.approach(from, target, incr)
	if (target > from) then
		return math.min(target, from + math.abs(incr))
	else
		return math.max(target, from - math.abs(incr))
	end
end

function math.clamp(val, min, max)
	return math.min(math.max(val, min), max)
end

function math.lerp(from, to, factor)
	return from + factor * (to - from)
end
