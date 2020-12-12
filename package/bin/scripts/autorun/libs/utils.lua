RegisterClientScript()

-- Table extension
function table.count(tab)
	local size = 0
	for _, _ in pairs(tab) do
		size = size + 1
	end

	return size
end

-- Utils table
utils = {}
function utils.OverrideFunction(originalFunc, newFunc)
	if (not originalFunc) then
		return newFunc
	end

	return function (...)
		originalFunc(...)
		newFunc(...)
	end
end
