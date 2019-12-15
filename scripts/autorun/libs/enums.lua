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
NoEntity = 0
NoLayer = 0xFFFF

PropertyType = enums.Create({
	Boolean         = 0,
	Entity          = 1,
	Float           = 2,
	FloatPosition   = 3,
	FloatSize       = 4,
	Integer         = 5,
	IntegerPosition = 6,
	IntegerSize     = 7,
	Layer           = 8,
	String          = 9,
	Texture         = 10
})
