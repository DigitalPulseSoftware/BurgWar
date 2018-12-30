RegisterClientScript("animations.lua")

animation = {}
function animation.Rotate(entity, from, to, time)
	local co, main = coroutine.running()
	assert(not main, "Must be called from a coroutine")

	print("oh hi", co, coroutine.running())
	engine_AnimateRotation(entity, from, to, time, function () print("oh hi", co, coroutine.running()) print(coroutine.status(co)) coroutine.resume(co) end)
	coroutine.yield()
end
