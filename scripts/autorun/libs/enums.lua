RegisterClientScript("enums.lua")

enums = {}

local enumMt = {}

function enumMt:__index(key)
	local val = self.__content[key]
	if (val == nil) then
		error("Key " .. key .. " does not belong in enum")
	end

	return val
end

function enumMt:__newindex(key, val)
	error("Enums are read-only")
end

function enums.Create(content)
	assert(type(content) == "table")

	return setmetatable({ __content = content }, enumMt)
end

-- FIXME
PropertyType = enums.Create({
	Boolean  = 0,
	Float    = 1,
	Integer  = 2,
	String   = 3,
	Texture  = 4
})
