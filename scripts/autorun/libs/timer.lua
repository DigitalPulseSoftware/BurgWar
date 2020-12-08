RegisterClientScript()

function timer.Sleep(ms)
	local co, main = coroutine.running()
	assert(not main, "must be called from a coroutine")

	timer.Create(ms, function () assert(coroutine.resume(co)) end)
	coroutine.yield()
end
