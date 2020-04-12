RegisterClientScript()

local vec3meta = RegisterMetatable("vec3")
vec3meta.__index = vec3meta

function vec3meta:Distance(vec)
	return (self - vec):Length()
end

function vec3meta:Length()
	return math.sqrt(self:SquaredLength())
end

function vec3meta:Normalize()
	local length = self:Length()
	self.x = self.x / length
	self.y = self.y / length
	self.z = self.z / length

	return self, length
end

function vec3meta:SquaredLength()
	return self.x * self.x + self.y * self.y + self.z * self.z
end

function vec3meta:SquaredDistance(vec)
	return (self - vec):SquaredLength()
end

function vec3meta:__add(other)
	if (getmetatable(other) == vec3meta) then
		return Vec2(self.x + other.x, self.y + other.y, self.z + other.z)
	else
		error("Must be a vector3")
	end
end

function vec3meta:__mul(other)
	if (getmetatable(other) == vec3meta) then
		return Vec2(self.x * other.x, self.y * other.y, self.z * other.z)
	elseif (type(other) == "number") then
		return Vec2(self.x * other, self.y * other, self.z * other)
	else
		error("Factor must be a vector3 or a number")
	end
end

function vec3meta:__div(divider)
	if (getmetatable(divider) == vec3meta) then
		return Vec2(self.x / divider.x, self.y / divider.y, self.z / divider.z)
	elseif (type(divider) == "number") then
		return Vec2(self.x / divider, self.y / divider, self.z / divider)
	else
		error("Divider must be a vector3 or a number")
	end
end

function vec3meta:__sub(other)
	if (getmetatable(other) == vec3meta) then
		return Vec2(self.x - other.x, self.y - other.y, self.z - other.z)
	else
		error("Must be a vector")
	end
end

function vec3meta:__unm()
	return Vec2(-self.x, -self.y, -self.z)
end

function vec3meta:__tostring()
	return "vec3(" .. tostring(self.x) .. ", " .. tostring(self.y) .. ", " .. tostring(self.z) .. ")"
end

function Vec3(x, y, z)
	local vec3 = {x = x or 0, y = y or 0, z = z or 0}
	return setmetatable(vec3, vec3meta)
end
