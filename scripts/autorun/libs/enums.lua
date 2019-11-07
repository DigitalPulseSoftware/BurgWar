RegisterClientScript()

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

-- FIXME: This shouldn't be here
NoLayer = 0xFFFF

PropertyType = enums.Create({
	Boolean         = 0,
	Float           = 1,
	FloatPosition   = 2,
	FloatSize       = 3,
	Integer         = 4,
	IntegerPosition = 5,
	IntegerSize     = 6,
	Layer           = 7,
	String          = 8,
	Texture         = 9
})
