RegisterClientScript("animations.lua")

animation = {}
function animation.Rotate(entity, from, to, time)
	local co, main = coroutine.running()
	assert(not main, "Must be called from a coroutine")

	engine_AnimateRotation(entity, from, to, time, function () coroutine.resume(co) end)
	coroutine.yield()
end
