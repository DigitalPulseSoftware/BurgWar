RegisterClientScript()

local pidmeta = RegisterMetatable("pidcontroller")
pidmeta.__index = pidmeta

function pidmeta:Update(currentError, elapsedTime)
	local incrIntegral = currentError * elapsedTime
	if (self.integral) then
		self.integral = self.integral + incrIntegral
	else
		self.integral = incrIntegral
	end

	local deriv
	if (self.lastError) then
		deriv = (currentError - self.lastError) / elapsedTime
	else
		deriv = currentError / elapsedTime
	end

	self.lastError = currentError

	return currentError * self.p + self.integral * self.i + deriv * self.d
end

function pidmeta:__tostring()
	return "PID"
end

function PID(p, i, d)
	return setmetatable({
		p = p,
		i = i,
		d = d
	}, pidmeta)
end
