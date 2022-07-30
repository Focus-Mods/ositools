BEGIN_CLS(InputValue)
P_RO(Value)
P_RO(Value2)
P_RO(State)
END_CLS()

BEGIN_CLS(InputDevice)
P_RO(InputPlayerIndex)
P_RO(field_8)
P_RO(ControllerMapping)
P_RO(DeviceId)
P_REF(field_14)
P_RO(field_24)
END_CLS()

BEGIN_CLS(InputEventDesc)
P_RO(EventID)
P_RO(Flags1)
P_RO(Flags2)
P_RO(CategoryName)
P_RO(field_18)
P_RO(field_20)
P_RO(field_28)
P_RO(field_30)
P_RO(EventName)
P_REF(EventDesc)
P_RO(field_E8)
END_CLS()

BEGIN_CLS(InputRaw)
P_RO(InputType)
P_GETTER_SETTER(DeviceId, LuaGetDeviceId, LuaSetDeviceId)
END_CLS()

BEGIN_CLS(InputRawChange)
P_REF(Input)
P_REF(Value)
END_CLS()

BEGIN_CLS(InjectInputData)
INHERIT(InputRawChange)
END_CLS()

BEGIN_CLS(InjectTextData)
P_RO(DeviceId)
// TODO - text
END_CLS()

BEGIN_CLS(InjectDeviceEvent)
P_RO(EventId)
P_RO(DeviceId)
END_CLS()

BEGIN_CLS(InputEventText)
P_RO(PlayerId)
P_RO(Text)
P_RO(TextLength)
END_CLS()

BEGIN_CLS(InputEvent)
P_RO(EventId)
P_RO(InputPlayerIndex)
P_GETTER_SETTER(DeviceId, LuaGetDeviceId, LuaSetDeviceId)
P_REF(OldValue)
P_REF(NewValue)
P_BITMASK(Type)
P_RO(WasPreferred)
END_CLS()

BEGIN_CLS(FireEventDesc)
P_REF(EventDesc)
P_RO(PlayerIndex)
P_REF(Event)
P_RO(DeviceId)
END_CLS()

BEGIN_CLS(HoldRepeatEventDesc)
P_RO(field_0)
P_RO(field_8)
P_RO(field_10)
P_RO(field_18)
P_RO(field_20)
P_RO(field_28)
P_REF(Event)
P_RO(field_58)
P_RO(field_5C)
END_CLS()

BEGIN_CLS(InputBinding)
INHERIT(InputRaw)
P_RO(Modifiers)
P_BITMASK(Modifiers)
END_CLS()

BEGIN_CLS(InputBindingDesc)
P_RO(PlayerIndex)
P_RO(field_4)
P_REF(Binding)
P_RO(field_14)
P_RO(field_18)
END_CLS()

BEGIN_CLS(InputScheme::Binding)
P_RO(PlayerId)
P_RO(field_8)
P_REF(Binding)
END_CLS()

BEGIN_CLS(InputScheme::BindingSet)
P_REF(Bindings)
P_RO(Initialized)
END_CLS()

BEGIN_CLS(InputScheme)
P_REF(Bindings)
P_REF(PerPlayerBindings)
P_REF(DeviceIdToPlayerId)
END_CLS()

BEGIN_CLS(InputSetState)
P_REF(Inputs)
P_RO(Initialized)
END_CLS()

BEGIN_CLS(InputManager::HoldRepeatEvent)
P_REF(FireEvents)
P_REF(ReleaseEvents)
P_REF(ValueChangeEvents)
P_REF(HoldEvents)
P_REF(RepeatEvents)
P_REF(PressEvents)
END_CLS()

BEGIN_CLS(InputManager)
P_REF(PerPlayerHoldRepeatEvents)
P_REF(InputStates)
P_REF(InputDefinitions)
P_REF(InputScheme)
P_RO(PressedModifiers)
P_BITMASK(PressedModifiers)
P_RO(LastUpdateTime)
P_RO(ControllerAllowKeyboardMouseInput)
P_REF(RawInputs)
P_REF(PlayerDevices)
P_REF(PlayerDeviceIDs)
P_REF(CurrentRemap)
P_REF(RawInputs2)
// P_REF(TextInjects)
P_REF(InputInjects)
P_REF(DeviceEventInjects)
P_REF(PerDeviceData)
END_CLS()