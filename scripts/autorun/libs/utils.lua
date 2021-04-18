RegisterClientScript()

-- Table extension
function table.count(tab)
	local size = 0
	for _, _ in pairs(tab) do
		size = size + 1
	end

	return size
end

function table.join(...)
	local t = {}
	for _, tab in pairs({...}) do
		for k, v in ipairs(tab) do
			table.insert(t, v)
		end
	end

	return t
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
