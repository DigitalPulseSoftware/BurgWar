RegisterClientScript("timer.lua")

timer = {}
function timer.Sleep(ms)
	local co, main = coroutine.running()
	assert(not main, "Must be called from a coroutine")

	engine_SetTimer(ms, function () coroutine.resume(co) end)
	coroutine.yield()
end
