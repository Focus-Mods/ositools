local _I = Ext._Internal

local SubscribableEvent = {}

function SubscribableEvent:New(name)
	local o = {
		First = nil,
		NextIndex = 1,
		Name = name
	}
	setmetatable(o, self)
    self.__index = self
    return o
end

function SubscribableEvent:Subscribe(handler, opts)
	opts = opts or {}
	local index = self.NextIndex
	self.NextIndex = self.NextIndex + 1

	local sub = {
		Handler = handler,
		Index = index,
		Priority = opts.Priority or 100,
		Options = opts
	}

	self:DoSubscribe(sub)
	return index
end

function SubscribableEvent:DoSubscribeBefore(node, sub)
	sub.Prev = node.Prev
	sub.Next = node
	
	if node.Prev ~= nil then
		node.Prev.Next = sub
	else
		self.First = sub
	end

	node.Prev = sub
end

function SubscribableEvent:DoSubscribe(sub)
	if self.First == nil then 
		self.First = sub
		return
	end

	local cur = self.First
	local last

	while cur ~= nil do
		last = cur
		if sub.Priority > cur.Priority then
			self:DoSubscribeBefore(cur, sub)
			return
		end

		cur = cur.Next
	end

	last.Next = sub
	sub.Prev = last
end

function SubscribableEvent:RemoveNode(node)
	if node.Prev ~= nil then
		node.Prev.Next = node.Next
	end

	if node.Next ~= nil then
		node.Next.Prev = node.Prev
	end

	node.Prev = nil
	node.Next = nil
end

function SubscribableEvent:Unsubscribe(handlerIndex)
	local cur = self.First
	while cur ~= nil do
		if cur.Index == handlerIndex then
			self:RemoveNode(cur)
			return true
		end

		cur = cur.Next
	end
	
	Ext.PrintWarning("Attempted to remove subscriber ID " .. handlerIndex .. " for event '" .. self.Name .. "', but no such subscriber exists (maybe it was removed already?)")
	return false
end

function SubscribableEvent:Throw(event)
	local cur = self.First
	while cur ~= nil do
        local ok, result = xpcall(cur.Handler, debug.traceback, event)
        if not ok then
            Ext.PrintError("Error while dispatching event " .. self.Name .. ": ", result)
        end

		cur = cur.Next
	end
end

local MissingSubscribableEvent = {}

function MissingSubscribableEvent:New(name)
	local o = {
		Name = name
	}
	setmetatable(o, self)
    self.__index = self
    return o
end

function MissingSubscribableEvent:Subscribe(handler, opts)
    Ext.PrintError("Attempted to subscribe to nonexistent event: " .. self.Name)
end

function MissingSubscribableEvent:Unsubscribe(handlerIndex)
    Ext.PrintError("Attempted to unsubscribe from nonexistent event: " .. self.Name)
end

function MissingSubscribableEvent:Throw(event)
    Ext.PrintError("Attempted to throw nonexistent event: " .. self.Name)
end

_I._Events = {}

Ext.Events = {}
setmetatable(Ext.Events, {
	__index = function (self, event)
		return _I._Events[event] or MissingSubscribableEvent:New(event)
	end,

	__newindex = function (self, k, v)
		error("Cannot write to Ext.Events directly!")
	end
})

_I._ThrowEvent = function (event)
	Ext.Events[event.Name]:Throw(event)
end

_I._RegisterEngineEvent = function (event)
	_I._Events[event] = SubscribableEvent:New(event)
end

_I._CallLegacyEvent = function (fn, event)
	if event.Name == "GameStateChanged" then
		fn(event.FromState, event.ToState)
	elseif event.Name == "UIObjectCreated" then
		fn(event.UI)
	elseif event.Name == "InputEvent" then
		fn(event.InputEvent)
		-- FIXME - add other events!
	else
		fn()
	end
end

Ext.RegisterListener = function (evt, fn)
	Ext._WarnDeprecated56("Ext.RegisterListener(" .. evt .. ") is deprecated! Use Ext.Events." .. evt .. ":Subscribe() instead!")

	Ext.Events[evt]:Subscribe(function (event) 
		_I._CallLegacyEvent(fn, event)
	end)
end

_I._RegisterEvents = function ()
	_I._RegisterEngineEvent("NetMessageReceived")
	
	for i,ev in pairs(_I._PublishedSharedEvents) do
		_I._RegisterEngineEvent(ev)
	end
	
	for i,ev in pairs(_I._PublishedEvents) do
		_I._RegisterEngineEvent(ev)
	end

	-- Support for Ext.RegisterNetListener()
	Ext.Events.NetMessageReceived:Subscribe(function (e)
		_I._NetMessageReceived(e.Channel, e.Payload, e.UserID)
	end)
end

_I._NetListeners = {}

Ext.RegisterNetListener = function (channel, fn)
	if _I._NetListeners[channel] == nil then
		_I._NetListeners[channel] = {}
	end

	table.insert(_I._NetListeners[channel], fn)
end

_I._NetMessageReceived = function (channel, payload, userId)
	if _I._NetListeners[channel] ~= nil then
		for i,callback in pairs(_I._NetListeners[channel]) do
			local ok, err = xpcall(callback, debug.traceback, channel, payload, userId)
			if not ok then
				Ext.PrintError("Error during NetMessageReceived: ", err)
			end
		end
	end
end

_I._ConsoleCommandListeners = {}

_I.DoConsoleCommand = function (cmd)
	local params = {}
	for param in string.gmatch(cmd, "%S+") do
		table.insert(params, param)
	end

	local listeners = _I._ConsoleCommandListeners[params[1]]
	if listeners ~= nil then
		for i,callback in pairs(listeners) do
			local status, result = xpcall(callback, debug.traceback, table.unpack(params))
			if not status then
				Ext.PrintError("Error during console command callback: ", result)
			end
		end
	end
end

Ext.RegisterConsoleCommand = function (cmd, fn)
	if _I._ConsoleCommandListeners[cmd] == nil then
		_I._ConsoleCommandListeners[cmd] = {}
	end

	table.insert(_I._ConsoleCommandListeners[cmd], fn)
end
