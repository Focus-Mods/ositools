BEGIN_CLS(ComponentHandleWithType)
P_RO(TypeId)
P_RO(Handle)
END_CLS()

BEGIN_CLS(BaseComponent)
P_RO(Entity)
P_REF(Component)
END_CLS()


BEGIN_CLS(IGameObject)
P_REF(Base)
P_RO(MyGuid)
P_RO(NetID)

P_GETTER(Handle, LuaGetHandle)
P_GETTER_SETTER(Translate, LuaGetTranslate, LuaSetTranslate)
P_GETTER_SETTER(Rotation, LuaGetRotate, LuaSetRotate)
P_GETTER_SETTER(Scale, LuaGetScale, LuaSetScale)
P_GETTER(Velocity, LuaGetVelocity)
P_GETTER(Height, LuaGetHeight)
P_GETTER(Visual, LuaGetVisual)

P_FUN(IsTagged, LuaIsTagged)
P_FUN(HasTag, LuaIsTagged)
P_FUN(GetTags, LuaGetTags)
END_CLS()

BEGIN_CLS(IEoCServerObject)
INHERIT(IGameObject)
P_GETTER(DisplayName, LuaGetDisplayName)
P_FUN(GetStatus, LuaGetStatus)
P_FUN(GetStatusByType, LuaGetStatusByType)
P_FUN(GetStatusByHandle, LuaGetStatusByHandle)
P_FUN(GetStatuses, LuaGetStatusIds)
P_FUN(GetStatusObjects, LuaGetStatuses)
P_FUN(CreateCacheTemplate, CreateCacheTemplate)
P_FUN(TransformTemplate, LuaTransformTemplate)
P_FUN(ForceSyncToPeers, ForceSyncToPeers)
END_CLS()

BEGIN_CLS(IEoCClientObject)
INHERIT(IGameObject)
P_GETTER(DisplayName, LuaGetDisplayName)
P_FUN(GetStatus, LuaGetStatus)
P_FUN(GetStatusByType, LuaGetStatusByType)
P_FUN(GetStatuses, LuaGetStatusIds)
P_FUN(GetStatusObjects, LuaGetStatuses)
END_CLS()