animation = {}
function animation.Rotate(entity, from, to, time)
	local co = coroutine.running()
	engine_AnimateRotation(entity, from, to, time, function () coroutine.resume(co) end)
	coroutine.yield()
end
