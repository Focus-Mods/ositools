
### Lua API v53 Documentation

### Table of Contents  
 - [Getting Started](#getting-started)
    - [Bootstrap Scripts](#bootstrap-scripts)
 - [Client / Server States](#client-server)
    - [Persistent Variables](#persistent-vars)
 - [Console](#console)
 - [Object Identifiers](#object-identifiers)
 - [Calling Lua from Osiris](#calling-lua-from-osiris)
    * [Calls](#l2o_calls)
    * [Queries](#l2o_queries)
    * [Events](#l2o_events)
    * [Custom Calls](#l2o_custom_calls)
    * [Capturing Events/Calls](#l2o_captures)
 - [Calling Osiris from Lua](#calling-osiris-from-lua)
    * [Calls](#o2l_calls)
    * [Queries](#o2l_queries)
    * [Events](#o2l_events)
    * [PROCs](#o2l_procs)
    * [User Queries](#o2l_qrys)
    * [Databases](#o2l_dbs)
 - [UI](#ui)
 - [Stats](#stats)
    * [Stats Objects](#stats-objects)
    * [Custom Skill Properties](#stats-custom-skill-properties)
    * [Additional Types](#stats-additional-types)
    * [Character Stats](#character-stats)
    * [Character Dynamic Stats](#character-dynamic-stats)
    * [Item Stats](#item-stats)
    * [Item Dynamic Stats](#item-dynamic-stats)
 - [Mod Info](#mod-info)
 - [Server Objects](#server-objects)
    * [Characters](#server-characters)
    * [Player Custom Data](#player-custom-data)
    * [Items](#server-items)
    * [Projectiles](#server-projectiles)
    * [Surfaces](#server-surfaces)
    * [Statuses](#server-statuses)
    * [Combat](#server-combat)
 - [Surface Actions](#surface-actions)
 - [Root Templates](#root-templates)
     * [Surface](#surface-templates)
 - [Damage lists](#damage-lists)
 - [Utility functions](#ext-utility)
 - [JSON Support](#json-support)
 - [Engine Events](#engine-events)
    * [Load Events](#event-load-events)
    * [SkillGetDescriptionParam](#event-skillgetdescriptionparam)
    * [StatusGetDescriptionParam](#event-statusgetdescriptionparam)
    * [GetSkillDamage](#event-getskilldamage)
    * [ComputeCharacterHit](#event-computecharacterhit)
    * [BeforeCharacterApplyDamage](#event-beforecharacterapplydamage)
    * [StatusGetEnterChance](#event-statusgetenterchance)
    * [GetHitChance](#event-gethitchance)
 - [Upgrading](#upgrading)
   * [Migrating from v51 to v52](#migrating-from-v51-to-v52)
   * [Migrating from v44 to v45](#migrating-from-v44-to-v45)
   * [Migrating from v43 to v44](#migrating-from-v43-to-v44)
   * [Migrating from v42 to v43](#migrating-from-v42-to-v43)
   * [Migrating from v41 to v42](#migrating-from-v41-to-v42)

<a id="getting-started"></a>
## Getting Started

To start using the extension in your mod, a configuration file must be created that describes what features are utilized by your mod.

Create a file at `Mods\YourMod_11111111-2222-...\OsiToolsConfig.json` with the following contents, then tweak the values as desired:
```json
{
    "RequiredExtensionVersion": 52,
    "ModTable": "YOUR_MOD_NAME_HERE",
    "FeatureFlags": [
        "OsirisExtensions",
        "Lua"
    ]
}
```

Meaning of configuration keys:

| Key | Meaning |
|--|--|
| `RequiredExtensionVersion` | Osiris Extender version required to run the mod. It is recommended to use the version number of the Script Extender you used for developing the mod since the behavior of new features and backwards compatibility functions depends on this version number. |
| `ModTable` | Name of the mod in the global mod table (`Mods`) when using Lua. |
| `FeatureFlags` | A list of features that the mod is using. For performance reasons it is recommended to only list features that are actually in use. |

The following features are accepted in `FeatureFlags`:

| Value| Meaning |
|--|--|
| `Lua` | Enables Lua scripting |
| `OsirisExtensions` | Enables Osiris (Story) extension functions (see [Osiris API Documentation](https://github.com/Norbyte/ositools/blob/master/APIDocs.md)) |
| `Preprocessor` | Enables the use of preprocessor definitions in Story scripts. (See [Preprocessor](https://github.com/Norbyte/ositools/blob/master/APIDocs.md#preprocessor)) |
| `DisableFolding` | Disable folding of dynamic item stats |
| `CustomStats` | Activates the custom stats system in non-GM mode (see [Custom Stats](https://github.com/Norbyte/ositools/blob/master/APIDocs.md#custom-stats) for more details). Custom stats are always enabled in GM mode. |
| `CustomStatsPane` | Replaces the Tags tab with the Custom Stats tab on the character sheet |

<a id="bootstrap-scripts"></a>
### Bootstrap Scripts  

If Lua is enabled for the mod, the extender will attempt to load `BootstrapServer.lua` on the server side, and `BootstrapClient.lua` on the client side. These scripts should be created in this folder (create the Story\RawFiles\Lua folders if necessary):
```
Mods\ModName_UUID\Story\RawFiles\Lua\
```
**Required Scripts**  
| Name | State |
|--|--|
| `BootstrapServer.lua` | Server Side |
| `BootstrapClient.lua` | Client Side |

From here, these scripts can load other scripts with `Ext.Require`. The path to scripts are relative to the Lua folder, so if you had a file setup like this:
```
BootstrapClient.lua
BootstrapServer.lua
Server/SkillMechanics.lua
```
BootstrapServer would load `SkillMechanics.lua` with `Ext.Require("Server/SkillMechanics.lua")`. Script loading only needs to happen once.

See below for further information on the client/server states, as certain scripting functions are only available on a specific side (i.e. only Osiris functions work on the server-side, in unrestricted contexts).

<a id="client-server"></a>
## Client / Server States

Internally the game is split into two components, a client and a server component. When a new game is started/loaded, a new server is created and client connect to this server. The server component is only created on the host; client components are created on both the host and all peers. Because of this, the game technically always runs in multiplayer. Single player is just a special form of multiplayer where only one local peer is connected to the server.

Osiris and behavior scripts (gamescripts) always run on the server. Since Lua has access to features that require client-side code (UI modification, level scaling formulas, status chances, skill damage calculation, etc.) the extender keeps multiple Lua states: one state for the server and one for each client (including the single player "fake client"). These states are completely separated from each other and cannot access the internal state of each other (Lua globals, functions, variables).

Because they run in different environments, server and client states can access a different set of features. Functions/classes in this document are annotated with the following letters, which indicate where they are available:
 - **C** - The function is only available on the client
 - **S** - The function is only available on the server
 - **R** - Restricted; the function is only callable in special contexts/locations

<a id="persistent-vars"></a>
### Persistent Variables

The Lua state and all local variables are reset after each game reload. For keeping data through multiple play sessions it is possible to store them in the savegame by storing them in the mod-local table `Mods[ModTable].PersistentVars`. By default the table is `nil`, i.e. a mod should create the table and populate it with data it wishes to store in the savegame. The contents of `PersistentVars` is saved when a savegame is created, and restored before the `SessionLoaded` event is triggered.

(Note: There is no global `PersistentVars` table, i.e. mods that haven't set their `ModTable` won't be able to use this feature).

Example:
```lua
PersistentVars = {}
...
-- Variable will be restored after the savegame finished loading
function doStuff()
    PersistentVars['Test'] = 'Something to keep'
end

function OnSessionLoaded()
    -- Persistent variables are only available after SessionLoaded is triggered!
    Ext.Print(PersistentVars['Test'])
end

Ext.RegisterListener("SessionLoaded", OnSessionLoaded)
```


## Console

Extender versions v44 and above allow commands to be entered to the console window.

Press `<enter>` to enter console mode; in this mode the normal log output is disabled to avoid log spam while typing commands.

Client/server context can be selected by typing `client` or `server`. This selects in which Lua environment the console commands will execute. By default the console uses the server context.
The `reset` command reinitializes the server and client Lua VM.

Typing `exit` returns to log mode.

Commands prefixed by a `!` will trigger callbacks registered via the `RegisterConsoleCommand` function.
Example:
```lua
local function testCmd(cmd, a1, a2, ...)
    Ext.Print("Cmd: " .. cmd .. ", args: ", a1, ", ", a2);
end
Ext.RegisterConsoleCommand("test", testCmd);
```
The command `!test 123 456` will call `testCmd("test", 123, 456)` and prints `Cmd: test, args: 123, 456`.

Anything else typed in the console will be executed as Lua code in the current context. (eg. typing `Ext.Print(1234)` will print `123`). 
The console has full access to the underlying Lua state, i.e. server console commands can also call builtin/custom Osiris functions, so Osiris calls like `CharacterGiveReward(CharacterGetHostCharacter(), "CheatShieldsAllRarities", 1)` are possible using the console.
Variables can be used just like in Lua, i.e. variable in one command can later on be used in another console command. Be careful, console code runs in global context, so make sure console variable names don't conflict with globals (i.e. `Mods`, `Ext`, etc.)! Don't use `local` for console variables, since the lifetime of the local will be one console command. (Each console command is technically a separate chunk).

<a id="object-identifiers"></a>
## Object Identifiers

Game objects have multiple identifiers that are used for different purposes.

### GUID

The UUID or GUID (Globally Unique IDentifier) is a unique textual identifier (e.g. `123e4567-e89b-12d3-a456-426614174000`). It can reference any character or item. 

Use GUIDs to hardcode references to pre-made characters/items in scripts or to reference newly created characters/items on the server.

Usage Notes:
 - For objects that are created in the editor, the GUID is guaranteed to stay the same in all playthroughs, i.e. it is safe to hardcode the GUID in scripts. This is the identifier Osiris functions use (using the types `GUIDSTRING`, `ITEMGUID`, etc.)
 - Objects that are created during the game are assigned a randomly generated GUID. This GUID will not change during the lifetime of the object, i.e. it's safe to store it to reference the object later on.
 - Some object types (eg. projectiles, statuses, etc.) have no GUID, so they must be referenced using their object handle or NetID. Usually if an object is global (i.e. can appear anywhere in any context), it has a GUID. If it is local (i.e. it can only be assigned to a specific character etc.) it won't have a GUID.
 - GUIDs should not be used to reference objects on the client side (it is safe to use GUIDs on the server), as there are bugs on the client that cause the GUID to not replicate properly or it may be missing entirely. Use object handles (in client-only code) or NetID-s (when communicating with the server) to reference client objects instead.


### Object Handle

Object handles are used by the game engine internally; they're 64-bit numbers. They're used for performance reasons, i.e. it's significantly faster for the engine to find objects using a handle than by a GUID (it's also smaller). Most references in the savegame are also handles, not GUIDs. eg. the parent inventory, child items, etc. are all saved using their handle, not their GUID.

Since everything has an object handle, you can use it to reference any object on both the server and the client.

Usage Notes:
 - This is a very commonly used identifier in Lua; most Lua game objects refer to others using object handles.
 - Each object is assigned a new handle when it is created. Unlike GUIDs, handles for objects that are created in the editor will _not_ be the same in different playthroughs, so handles cannot be hardcoded in scripts.
 - Unlike GUIDs, the client and server use different handles to reference the same object. (Technically, client and server characters are different objects altogether.)
 - After an object was created, the _server_ handle will not change during the lifetime of the object, i.e. it's safe to store it to reference the object later on. If a savegame is reloaded, the server handle will stay the same.
 - Client handles can change between play sessions (i.e. after a savegame reload), but they'll remain the same during the play session. They can safely be kept in temporary structures (eg. Lua variables that get reset after a savegame load), but should not be persisted in Osiris databases or via `PersistentVariables`.

### NetID

The NetID (Network Identifier) is a number the game uses in client-server communication to reference objects. Since object handles differ on the server and the client and not every object type has a GUID, NetID is the only identifier that can be reliably used to identify objects on both sides.

Usage:
 - Each object is assigned a new NetID at the start of every play session. If a savegame is reloaded, the NetID may change.
 - Unlike object handles, both the server and the client use the same NetID to reference the same object.
 - Since they're only valid for the duration of the session, they can safely be kept in temporary structures (eg. Lua variables that get reset after a savegame load), but should not be persisted in Osiris databases or via `PersistentVariables`.


### Identifier Matrix

This table describes which identifiers are present/can be used for which object.

| Object | GUID | Object Handle | NetID |
|--|--|--|--|
| Server Character | ✔ | ✔ | ✔ |
| Server Item | ✔ | ✔ | ✔ |
| Server Projectile | | ✔ | ✔ |
| Server Status | | ✔ | ✔ |
| Server Surface | | ✔ | ✔ |
| Server Surface Action | | ✔ | |
| Server Game Action | | ✔ | |
| Client Character | * | ✔ | ✔ |
| Client Item | * | ✔ | ✔ |
| Client Projectile | | ✔ | ✔ |
| Client Status | | ✔ | ✔ |
| Client UI Object | | ✔ | |

\* Although client characters/items have a GUID, it cannot be used reliably.


<a id="calling-lua-from-osiris"></a>
## Calling Lua from Osiris <sup>S</sup>

By default, functions defined in Lua are not visible to Osiris. 
During the Lua server bootstrap process, it is possible to declare new functions (calls/queries/events) that will be accessible to Osiris during compilation and execution. Since Osiris only runs on the server, Osiris functions are inaccessible in Lua client states.

Lua functions are registered through the story header (`story_header.div`). This means that each time a function is added, changed or removed, the story header must be regenerated in the editor. (The game regenerates its own story header, so it is always up to date.)

<a id="l2o_types"></a>
### Types

The following table describes how Lua values are converted to Osiris values and vice versa.

| Lua Type | Osiris Type | Notes |
|--|--|--|
| `nil` | - | `nil` is not convertible to an Osiris value, however it has a special meaning when calling/returning from queries (see [Queries](#l2o_queries)). |
| `boolean` | `INTEGER` | Osiris has no true boolean type, it uses the integer value 1 to represent `true` and 0 to represent `false`. |
| `number` (integer) | `INTEGER` | Although Lua only has one `number` type, its internal representation can vary depending on whether it stores an integer or floating point number. |
| `number` (float) | `REAL` | |
| `string` | `STRING`, `GUIDSTRING` | Any `GUIDSTRING` alias (eg. `CHARACTERGUID`, `ITEMGUID`, ...) is also convertible to string. |
| `table` | - | Osiris only supports scalar values, so tables cannot be passed to/from Osiris functions. |



<a id="l2o_calls"></a>
### Calls

Calls can be registered using the `Ext.NewCall(name, parameters)` function. The first parameter is the name of the call to create. The second parameter is the argument list of the Osiris call; it should follow the same syntax that the Osiris story header uses.

It is strongly recommended to follow the Osiris naming scheme, i.e. the name of calls should start with the name prefix of your mod.

Examples:
```lua
-- Call with a single argument
local function TestLog(msg)
    print(msg)
end
Ext.NewCall(TestLog, "NRD_EXT_TestLog", "(STRING)_Msg");

-- Call with multiple arguments
local function Multiply(a, b)
    print(a * b)
end
Ext.NewCall(Multiply, "NRD_EXT_Multiply", "(REAL)_A, (REAL)_B");
```

Functions exported from Lua can be called in Osiris just like any other call:
```c
IF
[...]
THEN
NRD_EXT_TestLog("Test");
NRD_EXT_Multiply(10, 5);
```

<a id="l2o_queries"></a>
### Queries

Unlike `QRY`s defined in Osiris code, Lua queries can return values just like the built-in queries.
Queries can be registered using the `Ext.NewQuery(name, parameters)` function. The first parameter is the name of the query. The second parameter is the argument list of the Osiris query; it should follow the same syntax that the Osiris story header uses.

Queries have two outcomes: they can either succeed or fail. A successful query returns a value for all of its `out` arguments; a failed query doesn't return any values.
To indicate whether a query succeeded or failed, Lua uses the following mechanism:
 - For 0 `out` parameters (i.e. when the query returns no values) the function should return `true` when it succeeded and `false` when it failed.
 - For N (1 or more) `out` parameters the function should return N non-`nil` values when it succeeded and N `nil` values when it failed. It is not permitted to return a mixture of `nil` and non-`nil` values.

The following table summarizes the expected return values:

| Number of params | Result | Return values |
|--|--|--|
| 0 | Successful | `true` |
| 0 | Failed | `false` |
| 1 | Successful | non-`nil` return value |
| 1 | Failed | `nil` |
| 3 | Successful | 3 non-`nil` return values - (v1, v2, v3) |
| 3 | Failed | `nil, nil, nil` |

Example:
```lua
local function Divide(a, b)
    if b == 0 then
        return nil
    end
    return a / b
end
Ext.NewQuery(Divide, "NRD_EXT_Divide", "[in](REAL)_A, [in](REAL)_B, [out](REAL)_Result");
```

Functions exported from Lua can be called in Osiris just like any other call:
```c
IF
[...]
AND
NRD_EXT_Divide(100, 5, _Result)
THEN
[...]
```

<a id="l2o_events"></a>
### Events

New Osiris events can be created by calling `Ext.NewEvent(name, parameters)`. The first parameter is the name of the event. The second parameter is the argument list of the Osiris event; it should follow the same syntax that the Osiris story header uses.

```lua
Ext.NewEvent("NRD_EXT_TestEvent", "(STRING)_Name");
```

Custom events can be thrown by calling them like a function:
```LUA
NRD_EXT_TestEvent("Whatever");
```

<a id="l2o_custom_calls"></a>
### Custom calls/queries

It is possible to call Lua functions by name, without exporting them to the Osiris story header. For this purpose multiple polymorphic functions are provided, `NRD_LuaCall*`, `NRD_ModCall*`, `NRD_LuaQuery*` and `NRD_ModQuery*`.

`NRD_LuaCall(Func, ...)` is a call (i.e. usable from the `THEN` part of the rule) and returns no results. Its first parameter is the function name to call, and it accepts an arbitrary number of arguments to pass to the Lua function. Only functions in the global table can be called this way, i.e. it is meant to be used for mods targeting v42 or below.
`NRD_ModCall(Mod, Func, ...)` is similar to `NRD_LuaCall`, but its first two parameters are the mod name in the global table (i.e. the `ModTable` setting from `OsiToolsConfig.json`) and the function name to call. It calls the function `Mods[Mod].Func(...)` in Lua, i.e. it is meant to be used for mods targeting v43 or above.

```lua
function TestFunc()
    Ext.Print("Woohoo!")
end

-- 2-arg version
function TestFunc2(str, int)
    Ext.Print(str .. ", " .. int)
end
```

```c
// Zero argument call - before v42
NRD_LuaCall("TestFunc");
// v43+
NRD_ModCall("YourMod", "TestFunc");

// Two argument call
[...]
AND
IntegerSum(1, 2, _Sum)
THEN
NRD_LuaCall("TestFunc2", "string arg", (STRING)_Sum);
// v43+
NRD_ModCall("YourMod", "TestFunc2", "string arg", (STRING)_Sum);
```

`NRD_LuaQuery*(Func, ...)` is a query (i.e. usable from the `IF` part of the rule). Its first parameter is the function name to call, and it accepts an arbitrary number of arguments to pass to the Lua function as well as an arbitrary number of results. The last character of the function name indicates the number of IN parameters (i.e. `NRD_LuaQuery2` for a query with 2 input parameters). Only functions in the global table can be called this way, i.e. it is meant to be used for mods targeting v42 or below.
`NRD_ModQuery*(Mod, Func, ...)` is a version of `NRD_LuaQuery` that calls functions from mod-local tables, i.e. it should be used for mods targeting version 43+.

```lua
-- 0 input, 0 output
function TestFunc()
    return "test"
end

-- 2 inputs, 2 outputs
function TestFunc2(str1, str2)
    return str1 .. str2, "something else"
end
```

Using `LuaQuery` (v42):
```c
[...]
AND
// Zero argument, zero return value query
NRD_LuaQuery0("TestFunc")
THEN
[...]

[...]
AND
// Two argument, two return value query
NRD_LuaQuery2("TestFunc2", "asdf", "ghjk", _Out1, _Out2)
THEN
DebugBreak(_Out1);
```

Using `ModQuery` (v43+):
```c
[...]
AND
// Zero argument, zero return value query
NRD_ModQuery0("YourMod", "TestFunc")
THEN
[...]

[...]
AND
// Two argument, two return value query
NRD_ModQuery2("YourMod", "TestFunc2", "asdf", "ghjk", _Out1, _Out2)
THEN
DebugBreak(_Out1);
```

<a id="l2o_captures"></a>
### Capturing Events/Calls

Since extender version v50 it is possible to capture certain Osiris events from Lua without adding Osiris boilerplate code. The `Ext.RegisterOsirisListener(name, arity, event, handler)` function registers a listener that is called in response to Osiris events.
It currently supports capturing events, built-in queries, databases, user-defined PROCs and user-defined QRYs. Capture support for built-in calls will be added in a later version.

Parameters: 
 - `name` is the function or database name
 - `arity` is the number of columns for DBs or the number of parameters (both IN and OUT) for functions
 - `event` is the type of event to capture; possible values:
   - `before` - Trigger event before a call/DB insert is performed
   - `after` - Trigger event after a call/DB insert is performed
   - `beforeDelete` - Trigger event before a DB delete is performed (databases only!)
   - `afterDelete` - Trigger event after a DB delete is performed (databases only!)
 - `handler` is a Lua function that is called when the specified event is triggered. The function receives all parameters of the original DB/function.

Example:
```lua
Ext.RegisterOsirisListener("ItemDropped", 1, "after", function (itemGuid)
    Ext.Print("ItemDropped - " .. itemGuid)
end)
```


## Calling Osiris from Lua <sup>S</sup>

Lua server contexts have a special global table called `Osi` that contains every Osiris symbol. In addition, built-in functions (calls, queries, events), functions added by the Osiris extender and functions registered from Lua via `Ext.NewCall`, `Ext.NewQuery` and `Ext.NewEvent` are also added to the global table.

<a id="o2l_calls"></a>
### Calls

Simply call the method from Lua with the same parameters:
```lua
-- Built-in functions are in the global table (_G)
CharacterResetCooldowns(player)
-- Equivalent to the above
Osi.CharacterResetCooldowns(player)
```

Implementation detail: Technically, name resolution is only performed when the function is called, since Osiris allows multiple overloads of the same name and the function to call is resolved based on the number of arguments. Because of this, getting any key from the `Osi` table will return an object, even if no function with that name exists. Therefore, `Osi.Something ~= nil` and similar checks cannot be used to determine whether a given Osiris symbol exists.

<a id="o2l_queries"></a>
### Queries

The query behavior is a mirror of the one described in the [Exporting Lua functions to Osiris](#exporting-lua-functions-to-osiris) chapter.

For queries with zero OUT arguments, the function will return a boolean indicating whether the query succeeded (true) or failed (false).
```lua
local succeeded = SysIsCompleted("TestGoal")
```

Queries with OUT arguments will have a number of return values corresponding to the number of OUT arguments. 
```lua
-- Single return value
local player = CharacterGetHostCharacter()
-- Multiple return values
local x, y, z = GetPosition(player)
```

<a id="o2l_events"></a>
### Events

Osiris events can be triggered by calling them like a function. Events are not buffered and the event is triggered synchronously, i.e. the function call returns when every Osiris rule that handles the event has finished.
```lua
StoryEvent(player, "event name")
```

<a id="o2l_procs"></a>
### PROCs

Calling PROCs is equivalent to built-in calls, however they are not added to the global table.

```lua
Osi.Proc_CharacterFullRestore(player)
```

<a id="o2l_qrys"></a>
### User Queries

User queries (`QRY`) behave just like built-in queries do. Since they can't have OUT arguments (i.e. can't return values), the function will just return a boolean indicating whether the query succeeded or not. User queries are not added to the global table.

```lua
local succeeded = Osi.Qry_IsHealingStatus("DAMAGE")
```

<a id="o2l_dbs"></a>
### Databases

Databases can be read using the `Get` method. The method checks its parameters against the database and only returns rows that match the query.

The number of parameters passed to `Get` must be equivalent to the number of columns in the target database. Each parameter defines an (optional) filter on the corresponding column; if the parameter is `nil`, the column is not filtered (equivalent to passing `_` in Osiris). If the parameter is not `nil`, only rows with matching values will be returned.

Example:
```lua
-- Fetch all rows from DB_GiveTemplateFromNpcToPlayerDialogEvent
local rows = Osi.DB_GiveTemplateFromNpcToPlayerDialogEvent:Get(nil, nil, nil)

-- Fetch rows where the first column is CON_Drink_Cup_A_Tea_080d0e93-12e0-481f-9a71-f0e84ac4d5a9
local rows = Osi.DB_GiveTemplateFromNpcToPlayerDialogEvent:Get("CON_Drink_Cup_A_Tea_080d0e93-12e0-481f-9a71-f0e84ac4d5a9", nil, nil)
```

It is possible to insert new tuples to Osiris databases by calling the DB like a function.

```lua
Osi.DB_CharacterAllCrimesDisabled(player)
```

The `Delete` method can be used to delete rows from databases. The number of parameters passed to `Delete` must be equivalent to the number of columns in the target database.
Each parameter defines an (optional) filter on the corresponding column; if the parameter is `nil`, the column is not filtered (equivalent to passing `_` in Osiris). If the parameter is not `nil`, only rows with matching values will be deleted.
Example:
```lua
-- Delete all rows from DB_GiveTemplateFromNpcToPlayerDialogEvent
Osi.DB_GiveTemplateFromNpcToPlayerDialogEvent:Delete(nil, nil, nil)

-- Delete rows where the first column is CON_Drink_Cup_A_Tea_080d0e93-12e0-481f-9a71-f0e84ac4d5a9
Osi.DB_GiveTemplateFromNpcToPlayerDialogEvent:Delete("CON_Drink_Cup_A_Tea_080d0e93-12e0-481f-9a71-f0e84ac4d5a9", nil, nil)
```


# UI

#### Ext.CreateUI(name, path, layer) <sup>C</sup>

Creates a new UI element. Returns the UI object on success and `nil` on failure.
 - `name` is a user-defined unique name that identifies the UI element. To avoid name collisions, the name should always be prefixed with the mod name (e.g. `NRD_CraftingUI`)
 - `path` is the path of the SWF file relative to the data directory (e.g. `"Public/ModName/GUI/CraftingUI.swf"`)
 - `layer` specifies the stack order of the UI element. Overlapping elements with a larger layer value cover those with a smaller one.

#### Ext.GetUI(name) <sup>C</sup>

Retrieves a UI element with the specified name. If no such element exists, the function returns `nil`.

#### Ext.GetBuiltinUI(path) <sup>C</sup>

Retrieves a built-in UI element at the specified path. If no such element exists, the function returns `nil`.

#### Ext.DestroyUI(name) <sup>C</sup>

Destroys the specified UI element.

#### Ext.UISetDirty (character: ObjectHandle, flags: integer) <sup>C</sup>

**Experimental!** Forces an UI refresh for the specified character.
Supported flag values: 
 - 0x1 - AP
 - 0x10 - Abilities
 - 0x60 - Status icons
 - 0x40000 - Health
 - 0x80000 - Skill set
 - 0x1000000 - Inventory
 - 0x10000000 - Character transform
 - 0x80000000 - Relations

### Interacting with the UI Element

#### UIObject:Invoke(func, ...) <sup>C</sup>

The `Invoke` method calls a method on the main timeline object of the UI element. The first argument (`func`) is the name of the ActionScript function to call; all subsequent arguments are passed to the ActionScript function as parameters.
Only `string`, `number` and `boolean` arguments are supported.

Example:
```lua
local ui = Ext.GetUI(...)
ui:Invoke("exposedMethod", "test")
```

ActionScript:
```actionscript
function exposedMethod(val: String): * {
	this.testLabel.text = val;
}
```

#### UIObject:ExternalInterfaceCall(func, ...) <sup>C</sup>

The `ExternalInterfaceCall` method simulates an `ExternalInterface.call(...)` call from Flash, i.e. it calls an UI handler function in the game engine. The first argument (`func`) is the name of the UI function to call; all subsequent arguments are passed to the engine as parameters.
Only `string`, `number` and `boolean` arguments are supported.

Example:
```lua
local ui = Ext.GetBuiltinUI("Public/Game/GUI/characterSheet.swf")
ui:ExternalInterfaceCall("show")
```

#### Ext.RegisterUICall(object, name, handler) <sup>C</sup>

The `Ext.RegisterUICall` function registers a listener that is called when the `ExternalInterface.call` function is invoked from ActionScript. ([ExternalInterface docs](https://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/external/ExternalInterface.html))
 - `object` is the UI object that is returned from `Ext.CreateUI`, `Ext.GetUI` or `Ext.GetBuiltinUI`
 - `name` is the ExternalInterface function name
 - `handler` is a Lua function that is called when the call is fired from Flash. The function receives the UI object and the function name as parameters followed by the arguments passed to the `ExternalInterface.call` call.

Example:
```lua
local function handleTextEvent(ui, call, arg1, arg2)
    ...
end

local ui = Ext.GetUI(...)
Ext.RegisterUICall(ui, "sendTextEvent", handleTextEvent)
```

```actionscript
ExternalInterface.call("sendTextEvent", "argument 1", "argument 2");
```

#### Ext.RegisterUITypeCall(typeId, name, handler) <sup>C</sup>

The `Ext.RegisterUITypeCall` function registers a listener that is called when the `ExternalInterface.call` function is invoked from ActionScript. It is similar to `Ext.RegisterUICall`, but registers the listener for all UI objects of the same type, not just one object. It can also be called before an UI object was created (i.e. there is no need to poll for an UI object or use the `UIObjectCreated` event).
 - `typeId` is the engine type ID of the UI object
 - `name` is the ExternalInterface function name
 - `handler` is a Lua function that is called when the call is fired from Flash. The function receives the UI object and the function name as parameters followed by the arguments passed to the `ExternalInterface.call` call.

#### Ext.RegisterUINameCall(name, handler) <sup>C</sup>

The `Ext.RegisterUINameCall` function registers a listener that is called when the `ExternalInterface.call` function is invoked from ActionScript. It is similar to `Ext.RegisterUICall`, but registers the listener for all UI objects, not just one object or one object type. It can also be called before an UI object was created (i.e. there is no need to poll for an UI object or use the `UIObjectCreated` event).
 - `name` is the ExternalInterface function name
 - `handler` is a Lua function that is called when the call is fired from Flash. The function receives the UI object and the function name as parameters followed by the arguments passed to the `ExternalInterface.call` call.

#### Ext.RegisterUIInvokeListener(object, name, handler) <sup>C</sup>

The `Ext.RegisterUIInvokeListener` function registers a listener that is called when the engine invokes a method on the Flash main timeline object.
 - `object` is the UI object that is returned from `Ext.CreateUI`, `Ext.GetUI` or `Ext.GetBuiltinUI`
 - `name` is the Flash method name
 - `handler` is a Lua function that is called when the call is fired. The function receives the UI object and the method name as parameters followed by the arguments passed to the Flash method.

Example:
```lua
local function onHelmetOptionChanged(ui, call, state)
    ...
end

local ui = Ext.GetBuiltinUI("Public/Game/GUI/characterSheet.swf")
Ext.RegisterUICall(ui, "setHelmetOption", onHelmetOptionChanged)
```

#### Ext.RegisterUITypeInvokeListener(typeId, name, handler) <sup>C</sup>

The `Ext.RegisterUITypeInvokeListener` function registers a listener that is called when the engine invokes a method on the Flash main timeline object. It is similar to `Ext.RegisterUIInvokeListener`, but registers the listener for all UI objects of the same type, not just one object. It can also be called before an UI object was created (i.e. there is no need to poll for an UI object or use the `UIObjectCreated` event).
 - `typeId` is the engine type ID of the UI object
 - `name` is the Flash method name
 - `handler` is a Lua function that is called when the call is fired. The function receives the UI object and the method name as parameters followed by the arguments passed to the Flash method.

#### Ext.RegisterUINameInvokeListener(name, handler) <sup>C</sup>

The `Ext.RegisterUINameInvokeListener` function registers a listener that is called when the engine invokes a method on the Flash main timeline object. It is similar to `Ext.RegisterUIInvokeListener`, but registers the listener for all UI objects, not just one object or one object type. It can also be called before an UI object was created (i.e. there is no need to poll for an UI object or use the `UIObjectCreated` event).
 - `name` is the Flash method name
 - `handler` is a Lua function that is called when the call is fired. The function receives the UI object and the method name as parameters followed by the arguments passed to the Flash method.


#### UIObject:SetValue(name, value, [arrayIndex]) <sup>C</sup>

The `SetValue` method updates the specified public property of the main timeline object. 


#### UIObject:GetValue(name, type, [arrayIndex]) <sup>C</sup>

The `GetValue` method retrieves the specified public property of the main timeline object. 
`type` contains the type of value to retrieve and must be `string`, `number` or `boolean`.


#### UIObject:Show() <sup>C</sup>

Displays the UI element.

#### UIObject:Hide() <sup>C</sup>

Hides the UI element.

#### UIObject:SetPosition(x, y) <sup>C</sup>

Updates the position of the UI element.

Example:
```lua
local ui = Ext.GetUI(...)
UI:SetPosition(100, 100)
```

#### UIObject:GetTypeId() <sup>C</sup>

Returns the engine UI type ID of the UI element. Useful for determining what type ID should be passed to the `Ext.RegisterUITypeInvokeListener` and `Ext.RegisterUITypeCall` functions.

#### UIObject:GetPlayerHandle() <sup>C</sup>

Attempts to determine the handle of the player that this UI element is assigned to. If the element is not assigned to a player, the function returns `nil`.
Only certain elements have a player assigned, like character sheet, inventory; others don't have player handles at all.

#### UIObject:GetRoot() <sup>C</sup>

Returns the Flash root object (main timeline object). This allows navigation of the Flash object hierarchy from Lua.
Unlike the `SetValue` and `GetValue` methods, this provides full access to all properties and sub-properties in Flash.

Examples:
```lua
local root = Ext.GetBuiltinUI("Public/Game/GUI/mainMenu.swf"):GetRoot()
-- Object properties on the main timeline and all child objects are readable
Ext.Print(root.mainMenu_mc.debugText_txt.htmlText)
-- ... and writeable
root.mainMenu_mc.debugText_txt.htmlText = "TEST TEST TEST"

-- Fetching array length supported via the # operator
Ext.Print(#root.events)
-- Indexed access to arrays supported
Ext.Print(root.events[2])
-- Array write support
root.events[2] = "TEST"

-- Method call support
root.mainMenu_mc.addMenuLabel("TEST LABEL")
Ext.Print(root.getHeight())
```

# Stats

## TODO - stat creation workflow, stat update workflow, note about persistent properties in case of dynamic stat updates (AVOID!)

<a id="stats-GetStatEntries"></a>
### GetStatEntries(type: string): string[]

Returns a table with the names of all stat entries.
When the optional parameter `type` is specified, it'll only return stats with the specified type.
The following types are supported: `StatusData`, `SkillData`, `Armor`, `Shield`, `Weapon`, `Potion`, `Character`, `Object`, `SkillSet`, `EquipmentSet`, `TreasureTable`, `ItemCombination`, `ItemComboProperty`, `CraftingPreviewData`, `ItemGroup`, `NameGroup`, `DeltaMod`


<a id="stats-objects"></a>
## Stats Objects

The following functions are only usable for Skill and Status, Armor, Shield, Weapon, Potion, Character and Object stats entries. Other stats types (eg. DeltaMods, TreasureTables) have their own separate sections in the docs and cannot be manipulated using these functions.


### GetStatEntriesLoadedBefore(modGuid: string, type: string): string[]

Returns a table with the names of all stat entries that were loaded before the specified mod.
This function is useful for retrieving stats that can be overridden by a mod according to the module load order.
When the optional parameter `type` is specified, it'll only return stats with the specified type. (The type of a stat entry is specified in the stat .txt file itself (eg. `type "StatusData"`).

### CreateStat(name: string, type: string, template: string|nil): StatEntry

Creates a new stats entry. 
If a stat object with the same name already exists, the specified modifier type is invalid or the specified template doesn't exist, the function returns `nil`.
After all stat properties were initialized, the stats entry must be synchronized by calling `SyncStat()`. 

 - `name` is the name of stats entry to create; it should be globally unique
 - `type` is the stats entry type (eg. `SkillData`, `StatusData`, `Weapon`, etc.)
 - If the `template` parameter is not null, stats properties are copied from the template entry to the newly created entry
 - If the entry was created on the server, `SyncStat()` will replicate the stats entry to all clients. If the entry was created on the client, `SyncStat()` will only update it locally.

Example:
```lua
local stat = Ext.CreateStat("NRD_Dynamic_Skill", "SkillData", "Rain_Water")
stat.RainEffect = "RS3_FX_Environment_Rain_Fire_01"
stat.SurfaceType = "Fire"
Ext.SyncStat("NRD_Dynamic_Skill")
```

### SyncStat(statId: string, persist: bool)

Synchronizes the changes made to the specified stats entry to each client.
`SyncStat` must be called each time a stats entry is modified dynamically (after `ModuleLoading`/`StatsLoaded`) to ensure that the host and all clients see the same properties.
The optional `persist` attribute determines whether the stats entry is persistent, i.e. if it will be written to savegames. If not specified, the `persist` parameter defaults to `true`.

### StatSetPersistence(statId: string, persist: bool) <sup>S</sup>

Toggles whether the specified stats entry should be persisted to savegames.
Changes made to non-persistent stats will be lost the next time a game is reloaded. 
If a dynamically created stats entry is marked as non-persistent, the entry will be deleted completely after the next reload. Make sure that you don't delete entries that are still in use as it could break the game in various ways.

### StatGetAttribute(stat, attribute)

Returns the specified `attribute` of the stat entry.
If the stat entry does not exist, the stat entry doesn't have an attribute named `attribute`, or the attribute is not supported, the function returns `nil`.

**Notes:**
 - For enumerations, the function will return the enumeration label (eg. `Corrosive`). See `ModifierLists.txt` or `Enumerations.xml` for a list of enumerations and enumeration labels.
 - The following fields are not supported: `AoEConditions`, `TargetConditions`, `ForkingConditions`, `CycleConditions`

`Requirements` and `MemorizationRequirements` are returned in the following format:
```js
[
    {
        "Not" : false, // Negated condition?
        "Param" : 1, // Parameter; number for ability/attribute level, string for Tag
        "Requirement" : "FireSpecialist" // Requirement name
    },
    {
        "Not" : false,
        "Param" : 1,
        "Requirement" : "Necromancy"
    }
]
```

### StatSetAttribute(stat, attribute, value) <sup>R</sup>

Updates the specified `attribute` of the stat entry. This essentially allows on-the-fly patching of stat .txt files from script without having to override the while stat entry.
If the function is called while the module is loading (i.e. from a `ModuleLoading`/`StatsLoaded` listener) no additional calls are needed. If the function is called after module load, the stats entry must be synchronized with the client via the `SyncStat` call. 

**Notes:**
 - For enumerations, the function accepts both the enumeration label (a string value, eg. `Corrosive`) and the enumeration index (an integer value, eg, `7`). See `ModifierLists.txt` or `Enumerations.xml` for a list of enumerations and enumeration labels.
 - Be aware that a lot of number-like attributes are in fact enums. eg. the `Strength`, `Finesse`, `Intelligence`, etc. attributes of `Potion` are enumerations and setting them by passing an integer value to this function will yield unexpected results. For example, calling `StatSetAttribute("SomePotion", "Strength", 5)` will set the `Strength` value to `-9.6`! The proper way to set these values is by passing the enumeration label as string, eg. `StatSetAttribute("SomePotion", "Strength", "5")`

Example:
```lua
-- Swap DamageType from Poison to Air on all skills
for i,name in pairs(Ext.GetStatEntries("SkillData")) do
    local damageType = Ext.StatGetAttribute(name, "DamageType")
    if damageType == "Poison" then
        Ext.StatSetAttribute(name, "DamageType", "Air")
    end
end
```

When modifying stat attributes that contain tables (i.e. `Requirements`, `TargetConditions`, `SkillProperties` etc.) it is not sufficient to just modify the table, the modified table must be reassigned to the property:
```lua
local requirements = Ext.StatGetAttribute(name, "MemorizationRequirements")
table.insert(requirements, {Name = "Intelligence", Param = 10, Not = false})
Ext.StatSetAttribute(name, "Requirements", requirements)
```

Stat entries that are modified on the fly (i.e. after `ModuleLoading`/`StatsLoaded`) must be synchronized via `SyncStat()`. Neglecting to do this will cause the stat entry to be different on the client and the server.
```lua
local stat = Ext.GetStat(name)
stat.DamageType = "Air"
stat.Damage = 10
Ext.SyncStat(name)
```

### GetStat(stat, [level])

Returns the specified stats entry as an object for easier manipulation.
If the `level` argument is not specified or is `nil`, the table will contain stat values as specified in the stat entry.
If the `level` argument is not `nil`, the table will contain level-scaled values for the specified level. A `level` value of `-1` will use the level specified in the stat entry.

The behavior of getting a table entry is identical to that of `StatGetAttribute` and setting a table entry is identical to `StatSetAttribute`.

The `StatSetAttribute` example rewritten using `GetStat`:
```lua
-- Swap DamageType from Poison to Air on all skills
for i,name in pairs(Ext.GetStatEntries("SkillData")) do
    local stat = Ext.GetStat(name)
    if stat.DamageType == "Poison" then
        stat.DamageType = "Air"
    end
end
```

### StatAddCustomDescription(stat, attribute, description) <sup>R</sup>

Adds a custom property description to the specified stat entry. (The blue text in the skill description tooltip).
This function can only be called from a `ModuleLoading` listener.

Example:
```lua
Ext.StatAddCustomDescription("Dome_CircleOfProtection", "SkillProperties", "Custom desc one")
```
### StatSetLevelScaling(statType, attribute, func) <sup>R</sup>

Replaces level scaling formula for the specified stat.
This function can only be called from a `ModuleLoading` listener.

`statType` is the type of stat to override (`Character`, `SkillData`, `Potion`, etc). `attribute` is the stat attribute to override (`Strength`, `Constitution`, ...).

`func` must satisfy the following requirements:
 - Must be a Lua function that receives two arguments `(attributeValue, level)` and returns the integer level scaled value.
 - Must have no side effects (i.e. can't set external variables, call external functions, etc)
 - Must always returns the same result when given the same argument values
 - Since the function is called very frequently (up to 50,000 calls during a level load), it should execute as quickly as possible

### GetSkillSet(name)

Returns a table with the names of skills contained within the specified SkillSet.

### GetEquipmentSet(name)

Returns a table with the names of equipment entries contained within the specified EquipmentSet.

<a id="stats-custom-skill-properties"></a>
## Custom Skill Properties

It is possible to create custom `SkillProperties` actions that will fire when the game executes the `SkillProperties` of a skill. A handler should be registered for the action on both the client and the server.

The client side needs to implement the `GetDescription` function that'll return the description of the skill property (used in tooltips). If the function returns `nil`, no description is displayed.

Example:
```lua
Ext.RegisterSkillProperty("MY_CUSTOM_SKILL", {
    GetDescription = function (property)
        return "Test SkillProperty description"
    end
})
```

The server side needs to implement the `ExecuteOnTarget` method for executing the property on a target character and the `ExecuteOnPosition` for executing the property on the ground.

Example:
```lua
Ext.RegisterSkillProperty("MY_CUSTOM_SKILL", {
    ExecuteOnTarget = function (property, attacker, target, position, isFromItem, skill, hit)
        Ext.PrintWarning("SKILLPROPERTY ExecuteOnTarget!")
        Ext.PrintWarning(property, attacker, target, position, isFromItem, skill, hit)
    end,
    ExecuteOnPosition = function (property, attacker, position, areaRadius, isFromItem, skill, hit)
        Ext.PrintWarning("SKILLPROPERTY ExecuteOnPosition!")
        Ext.PrintWarning(property, attacker, position, areaRadius, isFromItem, skill, hit)
    end
})
```

The stats entry uses the same basic format as statuses, but the action should be prefixed with `EXT:`. You can pass up to 5 (optional) parameters in `EXT:name,int,int,string,int,int` format.
Property contexts are supported (i.e. `SELF:`, `TARGET:`, `AOE:`, etc.).
Conditions (i.e. `IF(whatever):`) are not _yet_ supported.

Examples:
`data "SkillProperties" "SELF:EXT:MY_CUSTOM_SKILL"`
`data "SkillProperties" "EXT:MY_CUSTOM_SKILL,100,1,TEST TEST TEST,1,2"`

### Executing Skill Properties

It is possible to execute the `SkillProperties` of a skill manually (without actually using the skill) using the following functions:

```lua
Ext.ExecuteSkillPropertiesOnPosition(skillId: string, attacker: ObjectHandle|int|string, target: ObjectHandle|int|string, position: number[], propertyContext: string, isFromItem: boolean)
Ext.ExecuteSkillPropertiesOnTarget(skillId: string, attacker: ObjectHandle|int|string, position: number[], radius: number, propertyContext: string, isFromItem: boolean)
```

 - `skillId` is the stats entry name of the skill to use
 - `attacker` and `target` are the server-side object handles, GUIDs or NetID-s of the attacker and target character
 - `position` is the position of the hit
 - `radius` is the radius of the effect (when targeting the ground)
 - `propertyContext` contains the type of properties to execute; it should be one of `Target`, `AoE`, `Self`, `SelfOnHit`, `SelfOnEquip`
 - `isFromItem` determines whether the skill was granted by an item

Example:
```lua
local char = CharacterGetHostCharacter()
local position = Ext.GetCharacter(char).WorldPos

-- Exec on ground
Ext.ExecuteSkillPropertiesOnPosition("Projectile_AcidSpores", char, position, 3.0, "AoE", false)
-- Exec on a target character
Ext.ExecuteSkillPropertiesOnTarget("Shout_CatSwapPlaces", char, ch2, position, "Target", false)
```

### ExtraData

`Ext.ExtraData` is an object containing all entries from `Data.txt`.

*Note*: It is possible to add custom `ExtraData` keys by adding a new `Data.txt` to the mod and then retrieve them using Lua.

Example:
```lua
Ext.Print(Ext.ExtraData.DamageBoostFromAttribute)
```

<a id="stats-additional-types"></a>
## Additional Types

The following functions can be used to edit stats that don't use the object format (described above).

Getter functions can be used to retrieve the stats entry by name. They return a table that contains every property of the specified stats entry. 
Setter functions expect the same table as a parameter, and will update the stats entry. If the stats entry passed to the setter function doesn't exist, a new entry will be created.

| Stat File | Getter Function | Setter Function |
|--|--|--|
| `CraftingStationsItemComboPreviewData.txt` | `Ext.GetItemComboPreviewData(name)`| `Ext.UpdateItemComboPreviewData(previewData)` |
| `DeltaModifier.txt` | `Ext.GetDeltaMod(name, modifierType)`| `Ext.UpdateDeltaMod(deltaMod)` |
| `Equipment.txt` | `Ext.GetEquipmentSet(name)`| `Ext.UpdateEquipmentSet(equipmentSet)` |
| `ItemComboProperties.txt` | `Ext.GetItemComboProperty(name)`| `Ext.UpdateItemComboProperty(itemComboProperty)` |
| `ItemCombos.txt` | `Ext.GetItemCombo(name)`| `Ext.UpdateItemCombo(itemCombo)` |
| `ObjectCategoriesItemComboPreviewData.txt` | `Ext.GetItemComboPreviewData(name)`| `Ext.UpdateItemComboPreviewData(previewData)` |
| `SkillSet.txt` | `Ext.GetSkillSet(name)`| `Ext.UpdateSkillSet(skillSet)` |
| `TreasureGroups.txt` | `Ext.GetTreasureCategory(name)`| `Ext.UpdateTreasureCategory(name, treasureCategory)` |
| `TreasureTable.txt` | `Ext.GetTreasureTable(name)`| `Ext.UpdateTreasureTable(treasureTable)` |

Example usage:
```lua
-- Adding a new boost to an existing DeltaMod
local deltaMod = Ext.GetDeltaMod("Boost_Armor_Gloves_Primary_Strength_Medium", "Armor")
table.insert(deltaMod.Boosts, {"Boost" : "_Boost_Armor_Gloves_Primary_Wits", "Count": 1})
Ext.UpdateDeltaMod(deltaMod)

-- Creating a new DeltaMod from scratch
local deltaMod = {
    "Name" : "Boost_Armor_Gloves_Primary_Strength_Medium",
    "BoostType" : "ItemCombo",
    "ModifierType" : "Armor",
    "SlotType" : "Gloves",
    "Boosts" : [{
        "Boost" : "_Boost_Armor_Gloves_Primary_Strength_Medium",
        "Count" : 1
    }]
}
Ext.UpdateDeltaMod(deltaMod)
```



<a id="character-stats"></a>
## Character Stats

Represents all stats of a character (both players and non-players). Unlike Character objects which are different on the server and the client, the same Character Stats objects are present on both ends.

| Name | Type | Notes |
|--|--|--|
| DynamicStats | table | A table containing all dynamic stat entries for the character. See [CharacterDynamicStats](#character-dynamic-stats) for details on what these are. |
| MainWeapon | userdata | Currently equipped main hand weapon (or `nil` if none is equipped). See [ItemStats](#item-stats) |
| OffHandWeapon | userdata | Currently equipped off-hand weapon (or `nil` if none is equipped). See [ItemStats](#item-stats) |

To access items in other slots, use the `character:GetItemBySlot(slot)` method. The `slot` name must be one of `Helmet`, `Breast`, `Leggings`, `Weapon`, `Shield`, `Ring`, `Belt`, `Boots`, `Gloves`, `Amulet`, `Ring2` `Wings`, `Horns`, `Overhead`.

It is possible to fetch the base and computed values of the following stats. To get the base value (returns base points + permanent boosts + talent bonuses), add a `Base` prefix to the field (i.e. `BasePhysicalResistance` instead of `PhysicalResistance`); to get the computed value, just use the name (i.e. `PhysicalResistance`).

| Name | Type | Notes |
|--|--|--|
| MaxMp | integer | Max Source points |
| APStart | integer | |
| APRecovery | integer | |
| APMaximum | integer | |
| Strength | integer | |
| Finesse | integer | |
| Intelligence | integer | |
| Constitution | integer | |
| Memory | integer | |
| Wits | integer | |
| Accuracy | integer | |
| Dodge | integer | |
| CriticalChance | integer | |
| FireResistance | integer | |
| EarthResistance | integer | |
| WaterResistance | integer | |
| AirResistance | integer | |
| PoisonResistance | integer | |
| ShadowResistance | integer | |
| CustomResistance | integer | |
| PhysicalResistance | integer | |
| PiercingResistance | integer | |
| CorrosiveResistance | integer | |
| MagicResistance | integer | |
| LifeSteal | integer | |
| Sight | integer | |
| Hearing | integer | |
| Movement | integer | |
| Initiative | integer | |
| BlockChance | integer | |
| ChanceToHitBoost | integer | |
| DamageBoost | integer | |

Talents can be queried using the field name `TALENT_` + the talent name (e.g. `character.TALENT_Bully`). For a list of talents, see [https://docs.larian.game/Talent_list](https://docs.larian.game/Talent_list)

Abilities can be queried using their name (e.g. `character.WarriorLore`). For a list of ability names see the `Ability` enumeration in `Enumerations.xml`.


<a id="character-dynamic-stats"></a>
## Character Dynamic Stats

Character stats are calculated from multiple different sources (base stats, potions, statuses, etc.). Each of these sources is stored as a dynamic stat.

Dynamic stat index `1` always contains character base stats, index `2` contains permanent boosts.

| Name | Type | Notes |
|--|--|--|
| SummonLifelinkModifier | integer | |
| Strength | integer | |
| Memory | integer | |
| Intelligence | integer | |
| Movement | integer | |
| MovementSpeedBoost | integer | |
| Finesse | integer | |
| Wits | integer | |
| Constitution | integer | |
| Willpower | integer | |
| Bodybuilding | integer | |
| FireResistance  | integer | |
| EarthResistance | integer | |
| WaterResistance | integer | |
| AirResistance | integer | |
| PoisonResistance | integer | |
| ShadowResistance | integer | |
| PiercingResistance | integer | |
| PhysicalResistance | integer | |
| CorrosiveResistance | integer | |
| MagicResistance | integer | |
| CustomResistance | integer | |
| Sight | integer | |
| Hearing | integer | |
| FOV | integer | |
| APMaximum | integer | |
| APStart | integer | |
| APRecovery | integer | |
| CriticalChance | integer | |
| Initiative | integer | |
| CriticalChance | integer | |
| Vitality | integer | |
| VitalityBoost | integer | |
| MagicPoints | integer | |
| Level | integer | |
| Gain | integer | |
| Armor | integer | |
| MagicArmor | integer | |
| ArmorBoost | integer | |
| MagicArmorBoost | integer | |
| ArmorBoostGrowthPerLevel | integer | |
| MagicArmorBoostGrowthPerLevel | integer | |
| DamageBoost | integer | |
| DamageBoostGrowthPerLevel | integer | |
| Accuracy | integer | |
| Dodge | integer | |
| MaxResistance | integer | |
| LifeSteal | integer | |
| Weight | integer | |
| ChanceToHitBoost | integer | |
| RangeBoost | integer | |
| APCostBoost | integer | |
| SPCostBoost | integer | |
| MaxSummons | integer | |
| BonusWeaponDamageMultiplier | integer | |
| TranslationKey | string | |
| BonusWeapon | string | |


<a id="item-stats"></a>
## Item Stats

Represents all stats of an item. Unlike Item objects which are different on the server and the client, the same Item Stats objects are present on both ends.

| Name | Type | Notes |
|--|--|--|
| DynamicStats | table | A table containing all dynamic stat entries for the item. See [ItemDynamicStats](#item-dynamic-stats) for details on what these are. |
| ItemType | string | `EquipmentStatType` value |
| ItemSlot | string | `ItemSlot` value |
| WeaponType | string | `WeaponType` value |
| AnimType | integer | |
| WeaponRange | number | |
| IsIdentified | boolean | |
| IsTwoHanded | boolean | |
| ShouldSyncStats | boolean | |
| HasModifiedSkills | boolean | |
| Skills | string | |
| DamageTypeOverwrite | string | `DamageType` value |
| Durability | integer | |
| DurabilityCounter | integer | |
| ItemTypeReal | string | |
| MaxCharges | integer | |
| Charges | integer | |

Immunity/attribute flags from the [AttributeFlags enumeration](#attributeflags) can be retrieved using their name (i.e. `stats.KnockdownImmunity`).


<a id="item-dynamic-stats"></a>
## Item Dynamic Stats

Item stats are calculated from multiple different sources (base stats, permanent boosts, runes, deltamods, etc.). Each of these sources is stored as a dynamic stat.

Dynamic stat index `1` always contains item base stats, index `2` contains permanent boosts, indices `3` to `5` are rune slots.

| Name | Type | Notes |
|--|--|--|
| Durability| integer | |
| DurabilityDegradeSpeed | integer | |
| StrengthBoost | integer | |
| FinesseBoost | integer | |
| IntelligenceBoost | integer | |
| ConstitutionBoost | integer | |
| MemoryBoost | integer | |
| WitsBoost | integer | |
| SightBoost | integer | |
| HearingBoost | integer | |
| VitalityBoost | integer | |
| SourcePointsBoost | integer | |
| MaxAP | integer | |
| StartAP | integer | |
| APRecovery | integer | |
| AccuracyBoost | integer | |
| DodgeBoost | integer | |
| LifeSteal | integer | |
| CriticalChance | integer | |
| ChanceToHitBoost | integer | |
| MovementSpeedBoost | integer | |
| RuneSlots | integer | |
| RuneSlots_V1 | integer | |
| FireResistance  | integer | |
| EarthResistance | integer | |
| WaterResistance | integer | |
| AirResistance | integer | |
| PoisonResistance | integer | |
| ShadowResistance | integer | |
| PiercingResistance | integer | |
| PhysicalResistance | integer | |
| CorrosiveResistance | integer | |
| MagicResistance | integer | |
| CustomResistance | integer | |
| Movement | integer | |
| Initiative | integer | |
| Willpower | integer | |
| Bodybuilding | integer | |
| MaxSummons | integer | |
| Value | integer | |
| Weight | integer | |
| Skills | string | |
| ItemColor | string | |
| ObjectInstanceName | string | |
| BoostName | string | |
| StatsType | string | (See `EquipmentStatsType` enumeration) 

Weapon-only properties:
| Name | Type | Notes |
|--|--|--|
| DamageType | string | (See `DamageType` enumeration) |
| MinDamage | integer | |
| MaxDamage | integer | |
| DamageBoost | integer | |
| DamageFromBase | integer | |
| CriticalDamage | integer | |
| WeaponRange | integer | |
| CleaveAngle | integer | |
| CleavePercentage | integer | |
| AttackAPCost | integer | |

Shield-only properties:
| Name | Type | Notes |
|--|--|--|
| ArmorValue | integer | |
| ArmorBoost | integer | |
| MagicArmorValue | integer | |
| MagicArmorBoost | integer | |
| Blocking | integer | |

Armor-only properties:
| Name | Type | Notes |
|--|--|--|
| ArmorValue | integer | |
| ArmorBoost | integer | |
| MagicArmorValue | integer | |
| MagicArmorBoost | integer | |


# Mod Info

### IsModLoaded(modGuid)

Returns whether the module with the specified GUID is loaded.
This is equivalent to Osiris `NRD_IsModLoaded`, but is callable when the Osiris scripting runtime is not yet available (i.e. `ModuleLoading˙, etc events).

Example:
```lua
if (Ext.IsModLoaded("5cc23efe-f451-c414-117d-b68fbc53d32d"))
    Ext.Print("Mod loaded")
end
```

### GetModLoadOrder()

Returns the list of loaded module UUIDs in the order they're loaded in.

### GetModInfo(modGuid)

Returns detailed information about the specified (loaded) module.
Example:
```lua
local loadOrder = Ext.GetModLoadOrder()
for k,uuid in pairs(loadOrder) do
    local mod = Ext.GetModInfo(uuid)
    Ext.Print(Ext.JsonStringify(mod))
end
```

Output:
```json
{
    "Author" : "Larian Studios",
    "Dependencies" :
    [
        "2bd9bdbe-22ae-4aa2-9c93-205880fc6564",
        "eedf7638-36ff-4f26-a50a-076b87d53ba0"
    ],
    "Description" : "",
    "Directory" : "DivinityOrigins_1301db3d-1f54-4e98-9be5-5094030916e4",
    "ModuleType" : "Adventure",
    "Name" : "Divinity: Original Sin 2",
    "PublishVersion" : 905969667,
    "UUID" : "1301db3d-1f54-4e98-9be5-5094030916e4",
    "Version" : 372645092
}
```

# Server Objects

<a id="server-characters"></a>
## Server Characters <sup>S</sup>

Characters in server contexts can be retrieved using the `Ext.GetCharacter(ref)` call. The function accepts a character GUID, a NetID or an ObjectHandle. If the character cannot be found, the return value is `nil`; otherwise a Character object is returned.

Player objects have the following properties:

| Name | Type | Notes |
|--|--|--|
| Stats | userdata | See [CharacterStats](#character-stats) |
| PlayerCustomData | userdata | See [PlayerCustomData](#player-custom-data) |
| NetID | integer | Network ID of the character |
| MyGuid | string | GUID of the character |
| WorldPos | number[3] | Position of the character |
| CurrentLevel | String | Name of level (map) the character is currently on |
| Scale | number |  |
| AnimationOverride | string |  |
| WalkSpeedOverride | integer |  |
| RunSpeedOverride | integer |  |
| NeedsUpdateCount | integer |  |
| ScriptForceUpdateCount | integer |  |
| ForceSynchCount | integer |  |
| SkillBeingPrepared | string |  |
| LifeTime | number | Used for summons to indicate lifetime |
| PartialAP | number | Movement AP |
| AnimType | integer |  |
| DelayDeathCount | integer |  |
| AnimationSetOverride | string |  |
| CustomTradeTreasure | string |  |
| Archetype | string |  |
| EquipmentColor | string |  |
| IsPlayer | boolean |  |
| Multiplayer | boolean |  |
| InParty | boolean |  |
| HostControl | boolean |  |
| Activated | boolean |  |
| OffStage | boolean |  |
| Dead | boolean |  |
| HasOwner | boolean  |  |
| InDialog | boolean |  |
| Summon | boolean  |  |
| CharacterControl | boolean |  |
| Loaded | boolean  |  |
| InArena | boolean |  |
| CharacterCreationFinished | boolean |  |
| Floating | boolean |  |
| SpotSneakers | boolean |  |
| WalkThrough | boolean |  |
| CoverAmount | boolean |  |
| CanShootThrough | boolean |  |
| PartyFollower | boolean |  |
| Totem | boolean  |  |
| NoRotate | boolean  |  |
| IsHuge | boolean  |  |
| Global | boolean |  |
| HasOsirisDialog | boolean |  |
| HasDefaultDialog | boolean |  |
| TreasureGeneratedForTrader | boolean |  |
| Trader | boolean |  |
| Resurrected | boolean |  |
| IsPet | boolean |  |
| IsSpectating | boolean |  |
| NoReptuationEffects | boolean |  |
| HasWalkSpeedOverride | boolean |  |
| HasRunSpeedOverride | boolean |  |
| IsGameMaster | boolean  |  |
| IsPossessed | boolean |  |


<a id="player-custom-data"></a>
## Player Custom Data

Contains player customization info. Properties:

| Name | Type | 
|--|--|
| CustomLookEnabled | boolean |
| Name | string |
| ClassType | string |
| SkinColor | integer |
| HairColor | integer |
| ClothColor1 | integer |
| ClothColor2 | integer |
| ClothColor3 | integer |
| IsMale | boolean |
| Race | string |
| OriginName | string |
| Icon | string |
| MusicInstrument | string |
| OwnerProfileID | string |
| ReservedProfileID | string |
| AiPersonality | string |
| Speaker | string |


<a id="server-items"></a>
## Server Items <sup>S</sup>

Items on the server can be retrieved using the `Ext.GetItem(ref)` call. The function accepts an item GUID or an ObjectHandle. If the item cannot be found, the return value is `nil`; otherwise an Item object is returned.

Items have the following properties:

| Name | Type | Notes |
|--|--|--|
| Stats | userdata | See [ItemStats](#item-stats) |
| PlayerCustomData | userdata | See [PlayerCustomData](#player-custom-data) |
| NetID | integer | Network ID of the item |
| MyGuid | string | GUID of the item |
| WorldPos | vec3 | Position of the item |
| CurrentLevel | String | Name of level (map) the item is currently on |
| Scale | number |  |
| CustomDisplayName | string | |
| CustomDescription | string | |
| CustomBookContent | string | |
| StatsId | string | Stats entry (eg. `WPN_Dagger`) |
| Slot | integer | |
| Amount | integer | |
| Vitality | integer | |
| Armor | integer | |
| InUseByCharacterHandle | integer | Character currently using the item |
| Key | string | Key used to open the container |
| LockLevel | integer | |
| OwnerHandle | integer | ObjectHandle to the owner of this item |
| ComputedVitality | integer | |
| ItemType | integer | |
| GoldValueOverwrite | integer | |
| WeightValueOverwrite | integer | |
| TreasureLevel | integer | |
| LevelOverride | integer | |
| ForceSynch | boolean | |


<a id="server-projectiles"></a>
## Server Projectiles <sup>S</sup>

Currently projectiles are only available when passed as parameters to event listeners (`GetSkillDamage`, `ComputeCharacterHit`, etc.), and are not retrievable otherwise.

They have the following properties:

| Name | Type | Notes |
|--|--|--|
| NetID | integer | Network ID of the projectile |
| MyGuid | string | GUID of the projectile |
| CasterHandle | integer | |
| SourceHandle | integer | |
| TargetObjectHandle | integer | |
| HitObjectHandle | integer | |
| SourcePosition | vec3 | |
| TargetPosition | vec3 | |
| DamageType | string | (See `DamageType` enumeration) |
| DamageSourceType | string | (See `CauseType` enumeration) |
| LifeTime | number |  |
| HitInterpolation | number |  |
| ExplodeRadius0 | number |  |
| ExplodeRadius1 | number |  |
| DeathType | string | (See `DeathType` enumeration) |
| SkillId | string |  |
| WeaponHandle | integer | |
| MovingEffectHandle | integer |  |
| SpawnEffect | string |  |
| SpawnFXOverridesImpactFX | boolean |  |
| EffectHandle | integer |  |
| RequestDelete | boolean |  |
| Launched | boolean |  |
| IsTrap | boolean |  |
| UseCharacterStats | boolean |  |
| ReduceDurability | boolean |  |
| AlwaysDamage | boolean |  |
| ForceTarget | boolean  |  |
| IsFromItem | boolean |  |
| DivideDamage | boolean  |  |
| IgnoreRoof | boolean |  |
| CanDeflect | boolean  |  |
| IgnoreObjects | boolean |  |
| CleanseStatuses | string |  |
| StatusClearChance | number |  |
| Position | vec3 |  |
| PrevPosition | vec3 |  |
| Velocity | vec3 |  |
| Scale | number|  |
| CurrentLevel | string |  |


<a id="server-surfaces"></a>
## Server Surfaces <sup>S</sup>

**WIP**

| Property | Type | Writeable | Notes |
|--|--|--|--|
| NetId | integer | | |
| MyHandle | ObjectHandle | | |
| SurfaceType | string | | |
| RootTemplate | [SurfaceTemplate](#surface-templates) | | |
| Flags | integer | | |
| TeamId | integer | | Combat team ID (see [Combat Team](#server-combat-team)) |
| OwnerHandle | ObjectHandle| | Character/item that created this surface |
| LifeTime | number | ✓ | Surface lifetime in seconds |
| LifeTimeFromTemplate | boolean | | |
| StatusChance | number | ✓ | |


<a id="server-statuses"></a>
## Server Statuses <sup>S</sup>

Properties available on all statuses:

| Property | Type | Notes |
|--|--|--|
| NetID | integer | Network ID of the status. Since status have no GUID, only the NetID can be used for sending status references between the client and the server. |
| StatusId | string | Name of the associated stat entry |
| StatusHandle | integer | Handle of this status |
| TargetHandle | integer | Character or item that the status was applied to |
| StatusSourceHandle | integer | Character or item that caused the status |
| StartTimer | number |  |
| LifeTime | number | Total lifetime of the status, in seconds. -1 if the status does not expire. |
| CurrentLifeTime | number | Remaining lifetime of the status, in seconds. |
| TurnTimer | number | Elapsed time in the current turn (0..6) |
| Strength | number |  |
| StatsMultiplier | number |  |
| CanEnterChance | integer | Chance of entering status (between 0 and 100) |
| DamageSourceType | string | Cause of status (See `DamageSourceType` enum) |
| KeepAlive | boolean |  |
| IsOnSourceSurface | boolean |  |
| IsFromItem | boolean |  |
| Channeled | boolean |  |
| IsLifeTimeSet | boolean | Does the status have a lifetime or is it infinite? |
| InitiateCombat | boolean |  |
| Influence | boolean |  |
| BringIntoCombat | boolean |  |
| IsHostileAct | boolean |  |
| IsInvulnerable | boolean | The status turns the character invulnerable |
| IsResistingDeath | boolean | The character can't die until the status expires |
| ForceStatus | boolean | Bypass immunity and status enter chance checks. |
| ForceFailStatus | boolean | Forces prerequisite checks to fail. |
| RequestDelete | boolean | The status is being deleted (i.e. it's not active anymore) |
| RequestDeleteAtTurnEnd | boolean | The status will be deleted at the end of the current turn |
| Started | boolean |  |

### `CONSUME` status properties

| Property | Type | Notes |
|--|--|--|
| ResetAllCooldowns | boolean |  |
| ResetOncePerCombat | boolean |  |
| ScaleWithVitality | boolean |  |
| LoseControl | boolean |  |
| ApplyStatusOnTick | boolean |  |
| EffectTime | number |  |
| StatsId | string |  |
| StackId | string |  |
| OriginalWeaponStatsId | string |  |
| OverrideWeaponStatsId | string |  |
| OverrideWeaponHandle | integer |  |
| SavingThrow | string |  |
| SourceDirection | vec3 |  |
| Turn | integer | |
| HealEffectOverride | string | See `HealEffect` enumeration |

### `HIT` status properties

| Property | Type | Notes |
|--|--|--|
| SkillId | string | Stats ID of the skill (`SkillData`) that was used for the attack |
| HitByHandle | integer |  |
| HitWithHandle | integer |  |
| WeaponHandle | integer |  |
| HitReason | integer |  |
| Interruption | boolean |  |
| AllowInterruptAction | boolean |  |
| ForceInterrupt | boolean |  |
| DecDelayDeathCount | boolean |  |
| ImpactPosition | vec3 |  |
| ImpactOrigin | vec3 |  |
| ImpactDirection | vec3 |  |
| Equipment | integer | |
| TotalDamage | integer | Sum of all damages |
| DamageDealt | integer | Damage dealt after ApplyDamage |
| DeathType | string | See `Death Type` enumeration |
| DamageType | string | See `Damage Type` enumeration |
| AttackDirection | string | See `AttackDirection` enumeration. |
| ArmorAbsorption | integer | Armor points consumed during attack |
| LifeSteal | integer |  |
| HitWithWeapon | boolean |  |
| Hit | boolean | The attack hit |
| Blocked | boolean | The attack was blocked |
| Dodged | boolean | The attack was dodged |
| Missed | boolean | The attack missed |
| CriticalHit | boolean |  |
| AlwaysBackstab | boolean | Equivalent to the `AlwaysBackstab` skill property |
| FromSetHP | boolean | Indicates that the hit was called from `CharacterSetHitpointsPercentage` (or similar) |
| DontCreateBloodSurface | boolean | Avoids creating a blood surface when the character is hit |
| Reflection | boolean |  |
| NoDamageOnOwner | boolean |  |
| FromShacklesOfPain | boolean |  |
| DamagedMagicArmor | boolean | Indicates that the hit damaged magic armor |
| DamagedPhysicalArmor | boolean | Indicates that the hit damaged physical armor |
| DamagedVitality | boolean | Indicates that the hit damaged the characters vitality |
| Flanking | boolean | |
| PropagatedFromOwner | boolean |  |
| Surface | boolean | The hit is from a surface (`HitType` was `Surface`) |
| DoT | boolean | The hit is from a DoT attack (`HitType` was `DoT`) |
| ProcWindWalker | boolean | Hit should proc the Wind Walker talent |
| CounterAttack | boolean | Counterattack triggered by Gladiator talent |
| Poisoned | boolean | Character was poisoned when hit |
| Bleeding | boolean | Character was bleeding when hit |
| Burning | boolean | Character was burning when hit |
| NoEvents | boolean | Don't throw `OnHit`/`OnPrepareHit` events for this hit |

### `DAMAGE` status properties

| Property | Type | Notes |
|--|--|--|
| DamageEvent | integer |  |
| HitTimer | number |  |
| TimeElapsed | number | |
| DamageLevel | integer | |
| DamageStats | string |  |
| SpawnBlood | boolean | |

### `DAMAGE_ON_MOVE` status properties

| Property | Type | Notes |
|--|--|--|
| DistancePerDamage | number | |
| DistanceTraveled | number | |

### `HEAL` status properties

| Property | Type | Notes |
|--|--|--|
| EffectTime | number |  |
| HealAmount | integer |  |
| HealEffect | string |  |
| HealEffectId | string | Default `RS3_FX_GP_ScriptedEvent_Regenerate_01` |
| HealType | string | See `StatusHealType` enumeration |
| AbsorbSurfaceRange | integer |  |
| TargetDependentHeal | boolean |  |


### `HEALING` status properties

| Property | Type | Notes |
|--|--|--|
| HealAmount | integer |  |
| TimeElapsed | number |  |
| HealEffect | string | See `HealEffect` enumeration |
| HealEffectId | string | Default `RS3_FX_GP_ScriptedEvent_Regenerate_01` |
| SkipInitialEffect | boolean | |
| HealingEvent | integer |  |
| HealStat | string | See `HealType` enumeration |
| AbsorbSurfaceRange | integer |  |


<a id="server-combat"></a>
## Combat <sup>S</sup>

Each combat in-game is represented by a Combat object in Lua. 

Properties:

| Name | Type | Notes |
|--|--|--|
| CombatId | integer | A number identifying the combat instance |
| LevelName | string | Level where the combat is taking place |
| IsActive | boolean |  |


Methods:

#### GetAllTeams() <sup>S</sup>
Retrieves all participants of the combat. The return value is a table of `Team` objects.

#### GetCurrentTurnOrder() <sup>S</sup>
Retrieves the turn order of the current round. The return value is a table of `Team` objects.

#### GetNextTurnOrder() <sup>S</sup>
Retrieves the turn order of the next round. The return value is a table of `Team` objects.

#### UpdateCurrentTurnOrder(turnOrder) <sup>S</sup>
Updates the turn order of the current round. The `turnOrder` argument should be a reordered version of the table returned by `GetCurrentTurnOrder()`. 

Notes:
 - It is possible to remove or add characters to the current turn by adding/removing their `Team` object from the table. 
 - It is possible to add a character to the current turn more than once, the character will only appear once in the UI however.
 - The character whose turn is currently active (the very first item) should not be removed or reordered. This only applies for `GetCurrentTurnOrder`, the first item can be freely reordered in `GetNextTurnOrder`.
 - Changed performed using this function are synchronized to the client at the end of the current server tick.

#### UpdateNextTurnOrder(turnOrder) <sup>S</sup>
Updates the turn order of the next round. The `turnOrder` argument should be a reordered version of the table returned by `GetNextTurnOrder()`. 

Notes:
 - It is possible to remove or add characters to the next turn by adding/removing their `Team` object from the table. 
 - It is possible to add a character to the next turn more than once, the character will only appear once in the UI however.
 - Changed performed using this function are synchronized to the client at the end of the current server tick.

### CalculateTurnOrder Event <sup>S R</sup>

When the turn order of the next round of a combat is being updated for some reason (new round, character entered combat, etc.) the `CalculateTurnOrder` Ext event is thrown. 
The event receives two parameters:
 - `combat`: the Combat object
 - `order`: the turn order determined by the game (equivalent to calling `combat:GetNextTurnOrder()`)

To change the turn order, reorder the `order` table and return it from the event handler. To leave the turn order untouched, return `nil` (or nothing).

```lua
-- Example for calculating an initiative-based turn order
local function CalcInitiativeTurnOrder(combat, order)
    table.sort(order, function (a, b)
        return a.Initiative > b.Initiative
    end)
    return order
end

Ext.RegisterListener("CalculateTurnOrder", CalcInitiativeTurnOrder)
```

<a id="server-combat-team"></a>
### Combat Team <sup>S</sup>

A `Team` is a combat participant (either a character or an item).

Properties:

| Name | Type | Notes |
|--|--|--|
| TeamId | integer | A number identifying the team instance |
| CombatId | integer | Identifies which combat the team is a participant of |
| Initiative | integer | Computed initiative value of the team |
| StillInCombat | boolean | Can the team currently fight, or is it temporarily out of combat? |
| Character | esv::Character | Character object if the team is a character; `nil` otherwise |
| Item | esv::Item | Item object if the team is an item; `nil` otherwise |


<a id="surface-actions"></a>
# Surface Actions <sup>S</sup>

The surface action API allows creation and manipulation of surfaces.
To perform an action, the following steps must be performed:
 - Create the appropriate action using `Ext.CreateSurfaceAction(type)`
 - Set the required properties on the action object (see below for what settings are available for each action)
 - Launch the action using `Ext.ExecuteSurfaceAction(action)`

To cancel an action before its completed, `Ext.CancelSurfaceAction(handle)` can be used.

Shared properties on all surface actions:

| Property | Type | Notes |
|--|--|--|
| OwnerHandle | ObjectHandle| Character/item that created the surface |
| Duration | number | Surface duration in seconds |
| StatusChance | number | |
| Position | vec3 | |
| SurfaceType | string | Surface type name (eg. `Fire`, `FireBlessed`, etc.) |
| StatusChance | number | |


## ChangeSurfaceOnPath

Transforms surfaces in a path that follows the specified character or item.

Example:
```lua
local objectToFollow = Ext.GetCharacter(CharacterGetHostCharacter())
local surf = Ext.CreateSurfaceAction("ChangeSurfaceOnPathAction")
surf.SurfaceType = "Water"
surf.FollowObject = objectToFollow.Handle
surf.Radius = 3.0
local handle = surf.MyHandle
Ext.ExecuteSurfaceAction(surf)
```

Unlike other actions, `ChangeSurfaceOnPathAction` never expires and must be canceled by script to stop its effects:
```lua
Ext.CancelSurfaceAction(handle)
```

Properties:

| Property | Type | Notes |
|--|--|--|
| FollowObject | ObjectHandle | Character/item the transform will follow |
| Radius | number | Radius around the follow object that will be transformed |
| IgnoreIrreplacableSurfaces | boolean | |
| CheckExistingSurfaces | boolean | |
| SurfaceCollisionFlags | integer | When specified, AI grid cells that don't have any of the flags in `SurfaceCollisionFlags` will be ignored |
| SurfaceCollisionNotOnFlags | integer | When specified, AI grid cells that have any of the flags in `SurfaceCollisionNotOnFlags` will be ignored |
| IgnoreOwnerCells | boolean | Don't transform surfaces that are owned by the owner of this action |


## CreatePuddle

Creates a circular surface.
Example:
```lua
local pos = Ext.GetCharacter(CharacterGetHostCharacter()).WorldPos
local surf = Ext.CreateSurfaceAction("CreatePuddleAction")
surf.Duration = 6.0
surf.SurfaceType = "Fire"
surf.Position = {pos[1] + 10.0, pos[2], pos[3]}
surf.SurfaceCells = 100
surf.GrowSpeed = 0.1
surf.Step = 100
Ext.ExecuteSurfaceAction(surf)
```

Properties:

| Property | Type | Notes |
|--|--|--|
| SurfaceCells | integer | Number of AI grid cells to cover with the surface |
| Step | number | How many cells the surface grows on each tick |
| GrowSpeed | number | Determines the time between grow steps during surface creation |
| IgnoreIrreplacableSurfaces | boolean | |
| GrowTimer | number| |


## ExtinguishFire

Extinguishes fire surfaces in a radius around the target.
Example:
```lua
local surf = Ext.CreateSurfaceAction("ExtinguishFireAction")
surf.Position = {pos[1] + 2.0, pos[2], pos[3]}
surf.Radius = 2.0
surf.Percentage = 1.0
surf.GrowTimer = 0.1
surf.Step = 100
Ext.ExecuteSurfaceAction(surf)
```

Properties:

| Property | Type | Notes |
|--|--|--|
| DamageList | DamageList | Damage that gets applied when a character enters the surface |
| Position | vec3 | Position of surface to extinguish |
| Radius | number | Extinguish radius |
| Percentage | number | % of surface cells to remove (0..1) |
| GrowTimer | number | Time between grow steps |
| Step | number | Number of cells the action processes on each tick |


## RectangleSurface

Creates a rectangular surface.
Example:
```lua
local pos = Ext.GetCharacter(CharacterGetHostCharacter()).WorldPos
local surf = Ext.CreateSurfaceAction("RectangleSurfaceAction")
surf.Duration = 6.0
surf.SurfaceType = "Fire"
surf.Position = {pos[1] + 1.0, pos[2], pos[3]}
surf.Target = {pos[1] + 10.0, pos[2], pos[3]}
surf.Width = 2
surf.Length = 10
surf.GrowTimer = 0.1
surf.GrowStep = 100
Ext.ExecuteSurfaceAction(surf)
```

Properties:

| Property | Type | Notes |
|--|--|--|
| DamageList | DamageList | Damage that gets applied when a character enters the surface |
| Target | vec3 | Position that the surface grows towards |
| SurfaceArea | number | |
| Width | number | Width of rectangle |
| Length | number | Length of rectangle |
| GrowTimer | number | Determines the time between grow steps during surface creation |
| GrowStep | number | How many cells the surface grows on each tick |
| MaxHeight | number | |
| AiFlags | integer | |
| DeathType | string | |
| LineCheckBlock | integer | |


## PolygonSurface

Creates a surface defined by a polygon.

Example:
```lua
local pos = Ext.GetCharacter(CharacterGetHostCharacter()).WorldPos
local surf = Ext.CreateSurfaceAction("PolygonSurfaceAction")
surf.Duration = 12.0
surf.SurfaceType = "Oil"
surf.Vertices = { -- Triangle shape
    {pos[1], pos[3]},
    {pos[1] + 10.0, pos[3]},
    {pos[1] + 10.0, pos[3] + 10.0}
}
Ext.ExecuteSurfaceAction(surf)
```

Properties:

| Property | Type | Notes |
|--|--|--|
| DamageList | DamageList | Damage that gets applied when a character enters the surface |
| Vertices | vec2[] | List of 2D (X,Z) vertices of the polygon |
| GrowTimer | number | Determines the time between grow steps during surface creation |
| GrowStep | number | How many cells the surface grows on each tick |


## SwapSurface

Swaps surfaces between the specified positions.

Example:
```lua
local pos = Ext.GetCharacter(CharacterGetHostCharacter()).WorldPos
local surf = Ext.CreateSurfaceAction("SwapSurfaceAction")
surf.Position = {pos[1] + 10.0, pos[2], pos[3]}
surf.Target = {pos[1] + 5.0, pos[2], pos[3]}
surf.Radius = 5.0
Ext.ExecuteSurfaceAction(surf)
```

Properties:

| Property | Type | Notes |
|--|--|--|
| Radius | number | |
| ExcludeRadius | number | |
| MaxHeight | number | |
| Target | vec3 | Position of second surface that'll be swapped (first position is set using the `Position` property) |
| IgnoreIrreplacableSurfaces | boolean | |
| CheckExistingSurfaces | boolean | |
| SurfaceCollisionFlags | integer | When specified, AI grid cells that don't have any of the flags in `SurfaceCollisionFlags` will be ignored |
| SurfaceCollisionNotOnFlags | integer | When specified, AI grid cells that have any of the flags in `SurfaceCollisionNotOnFlags` will be ignored |
| LineCheckBlock | integer | |
| GrowTimer | number | Determines the time between grow steps during surface swap |
| GrowStep | number | How many cells to swap on each tick |


## Zone

Creates a cone-shaped surface.
Example:

```lua
local pos = Ext.GetCharacter(CharacterGetHostCharacter()).WorldPos
local surf = Ext.CreateSurfaceAction("ZoneAction")
surf.Duration = 6.0
surf.SurfaceType = "Oil"
surf.Position = pos
surf.Target = {pos[1] + 5.0, pos[2], pos[3]}
surf.GrowTimer = 0.02
surf.GrowStep = 10
surf.Shape = 0 -- 0=Cone, 1=Square
surf.Radius = 7.0
surf.AngleOrBase = 60.0
surf.MaxHeight = 2.4
Ext.ExecuteSurfaceAction(surf)
```

Properties:

| Property | Type | Notes |
|--|--|--|
| SkillId | string | Skill to use for `TargetConditions` checks and for executing `SkillProperties` when a target is hit by the surface |
| DamageList | DamageList | |
| Target | vec3 | Direction towards which the cone will be shot |
| Shape | integer | Surface shape (0 = Cone, 1 = Square) |
| Radius | number| Cone range |
| AngleOrBase | number | Cone angle or square base (depending on surface shape) |
| BackStart | number | |
| FrontOffset | number | |
| MaxHeight | number | |
| GrowTimer | number | Determines the time between grow steps during surface creation |
| GrowStep | integer | How many cells to add on each tick |
| AiFlags | integer | |
| DeathType | string | |


<a id="root-templates"></a>
# Root Templates

**Note:** When editing root templates from Lua make sure that the templates are updated on both the server and all clients; otherwise what the client sees may be out of sync with what's actually happening on the server.

<a id="surface-templates"></a>
## Surface Templates

Surface templates can be retrieved using the `Ext.GetSurfaceTemplate(type)` function. The `type` parameter is the name of the surface to fetch (eg. `Fire`, `OilCursed`). If the specified surface type exists, a surface template object is returned with the properties described below.

The properties of surface templates can be changed during module load as well as in runtime.
Example to replace `BURNING` status with `WET` when fire is applied:
```lua
Ext.GetSurfaceTemplate("Fire").Statuses = {{
    Duration = 12.0,
    KeepAlive = true,
    StatusId = "WET"
}}
```

| Property | Type | Writeable | Notes |
|--|--|--|--|
| SurfaceTypeId | integer | | Engine type ID of the surface |
| SurfaceType | string | | Surface name (eg. `Fire`, `FireBlessed`, etc.) |
| DisplayName | string | ✓ | |
| Description | string | ✓ | |
| DecalMaterial | string | ✓ | |
| CanEnterCombat | boolean | ✓ | |
| AlwaysUseDefaultLifeTime | boolean | ✓ | |
| DefaultLifeTime | number | ✓ | |
| SurfaceGrowTimer | number | ✓ | |
| FadeInSpeed | number | ✓ | |
| FadeOutSpeed | number | ✓ | |
| Seed | integer | ✓ | |
| Statuses | [SurfaceTemplateStatus](#surface-template-status)[] | ✓ | List of statuses that the surface applies when it comes in contact with a character |
| DamageWeapon | string | ✓ | |
| Summon | string | ✓ | |
| DamageCharacters | boolean | ✓ | |
| DamageItems | boolean | ✓ | |
| DamageTorches | boolean | ✓ | |
| RemoveDestroyedItems | boolean | ✓ | |
| CanSeeThrough | boolean | ✓ | |
| CanShootThrough | boolean | ✓ | |


<a id="surface-template-status"></a>
### Surface Template Status

This type describes a status that is either applied or removed when a character comes in contact with the surface. Each surface type may apply/remove multiple statuses.

| Property | Type | Writeable | Notes |
|--|--|--|--|
| StatusId | string | ✓ | Stats ID of the status that the surface applies |
| Chance | number | ✓ | Status apply chance (0..1) |
| Duration | string | ✓ | Duration of status in seconds |
| RemoveStatus | boolean | ✓ | Should the surface remove this status instead of apply? |
| OnlyWhileMoving | boolean | ✓ | |
| ApplyToCharacters | boolean | ✓ | |
| ApplyToItems | boolean | ✓ | |
| KeepAlive | boolean  | ✓ | Characters lose the status if they're no longer in the surface |
| VanishOnReapply | boolean | ✓ | Decay the surface when the status is already applied to the character |
| ForceStatus | boolean | ✓ | Equivalent to the `_Force` parameter to `ApplyStatus()` |


## Damage Lists

A damage list is an object that stores the amount of damage dealt for each damage type (`Physical`, `Poison`, etc.).
It is currently used by the `GetSkillDamage` and `ComputeCharacterHit` events to fetch damage information.

Damage lists can be created using the `Ext.NewDamageList()` function.

Methods:

#### Add(damageType, amount)
Increase/decrease the amount of damage in the list. Positive values add, negative values remove damage.
```lua
list:Add("Physical", 10) -- Add 10 points of physical damage
list:Add("Physical", -5) -- Subtract 5 points of physical damage
```

#### Clear([damageType])
Clears the damage list. If `damageType` is specified, only damages with the specified type will be removed.
```lua
list:Clear() -- Remove all
list:Clear("Poison") -- Remove only poison damage
```

#### Multiply(amount)
Multiplies the amount of damage with the specified value.
```lua
list:Add("Physical", 10)
list:Multiply(2.5) -- Physical damage amount is now 25
```

#### Merge(list)
Merge the damage values in the second list into the first one.
```lua
list:Add("Physical", 15)
local list2 = Ext.NewDamageList()
list2:Add("Physical", 25)
list:Merge(list2) -- Physical damage amount is now 40
```

#### ConvertDamageType(damageType)
Converts the damage type of every item in the damage list to the specified value.
```lua
list:Add("Physical", 15)
list:Add("Piercing", 15)
list:ConvertDamageType("Poison") -- Poison damage amount is now 30
```

#### ToTable()
Returns a table containing every item in the list.
```lua
for i,damage in pairs(list:ToTable()) do
    Ext.Print(damage.DamageType .. ": " .. damage.Amount)
end
```

<a id="ext-utility"></a>
## Utility functions

#### Ext.Require(path) <sup>R</sup>

The `Ext.Require` function is the extender's version of the Lua built-in `require` function. 
The function checks if the file at `Mods/<ModuleUUID>/Story/RawFiles/Lua/<path>` was already loaded; if not, it'll load the file, store the return value of the main chunk and return it to the caller. If the file was already loaded, it'll return the stored return value.
**Note:** `Ext.Require` should only be called during module startup (i.e. when loading `BootstrapClient.lua` or `BoostrapServer.lua`). Loading Lua files after module startup is deprecated.

#### Ext.Print(...)

Prints the specified value(s) to the debug console. Works similarly to the built-in Lua `print()`, except that it also logs the printed messages to the editor messages pane.

#### Ext.GetTranslatedString(key[, fallback])

Returns the text associated with the specified translated string key. If the key doesn't exist, the value of `fallback` is returned. If no fallback value is specified, an empty string (`""`) is returned.

```lua
local str = Ext.GetTranslatedString("h17edbbb2g9444g4c79g9409gdb8eb5731c7c", "[1] cast [2] on the ground")
```

#### Ext.AddPathOverride(originalPath, newPath)

Redirects file access from `originalPath` to `newPath`. This is useful for overriding built-in files or resources that are otherwise not moddable, eg. UI Flash files.
Make sure that the override is added as early as possible (preferably in `ModuleLoading`), as adding path overrides after the game has already loaded the resource has no effect.

Example:
```lua
Ext.AddPathOverride("Public/Game/GUI/enemyHealthBar.swf", "Public/YourMod/GUI/enemyHealthBar.swf")
```

#### Ext.PlayerHasExtender(playerGuid)

Returns whether the player that controls the character `playerGuid` has a compatible Script Extender version installed.
Example:
```lua
for i,player in  ipairs(Osi.DB_IsPlayer:Get(nil)) do
    if  not Ext.PlayerHasExtender(player[1]) then
        OpenMessageBox(player[1], "Install the extender!!!")
    end
end
```

#### Ext.MonotonicTime()

Returns a monotonic value representing the current system time in milliseconds. Useful for performance measurements / measuring real world time.
(Note: This value is not synchronized between peers and different clients may report different time values!)

Example:
```lua
local startTime = Ext.MonotonicTime()
DoLongTask()
local endTime = Ext.MonotonicTime()
Ext.Print("Took: " .. tostring(endTime - startTime) .. " ms")
```

## JSON Support

Two functions are provided for parsing and building JSON documents, `Ext.JsonParse` and `Ext.JsonStringify`.

Lua types are encoded in JSON (and vice versa) using the following table:

| Lua Type | JS Type |
|--|--|
| `nil` | `null` |
| `boolean` | `boolean` |
| `number` (integer) | `number` |
| `number` (float) | `number` |
| `string` | `string` |
| `table` (sequential keys) | `array` |
| `table` (non-sequential) | `object` |

It is not possible to stringify/parse `lightuserdata`, `userdata`, `function` and `thread` values.

Since JSON only supports string object keys, Lua `number` (integer/float) keys are saved as `string`.

Usage example:
```lua
local tab = {
    asd = 1234,
    arr = {
        "ab", "bc", 44
    }
}

local json = Ext.JsonStringify(tab)
Ext.Print(json)

local decoded = Ext.JsonParse(json)
Ext.Print(decoded.arr[1])
```

Expected output:
```
{
    "arr": [
        "ab",
        "bc",
        44
    ],
    "asd" : 1234
}

ab
```


<a id="engine-events"></a>
# Engine Events

<a id="event-load-events"></a>
## Load Events

### ModuleLoadStarted

The `ModuleLoadStarted` event is thrown when the engine has started loading mods. Mod data (stats, localization, root templates, etc.) is not yet loaded when this listener is called, so most mod editing functionality (eg. `Ext.StatSetAttribute`) is inaccessible.
The purpose of this event is to allow adding filesystem-level hooks using `Ext.AddPathOverride` before mod data is loaded.

### StatsLoaded

`StatsLoaded` is thrown after stats entries (weapons, skills, etc.) were cleared and subsequently reloaded. Stat modifications that are valid for every game session should be applied here.

### ModuleLoading

`ModuleLoading` is thrown after the stats manager has finished loading; this callback is deprecated and `StatsLoaded` should be used instead.

### SessionLoading

`SessionLoading` is thrown when the the engine has started setting up a game session (i.e. new game, loading a savegame or joining a multiplayer game). Stat overrides that use Lua callbacks (`Ext.StatSetLevelScaling`) and custom UI (`Ext.CreateUI`, `Ext.RegisterUICall`, `Ext.RegisterUIInvokeListener`, etc.) should be set up here.

### SessionLoaded

`SessionLoaded` is thrown when the game session was set up.


<a id="event-skillgetdescriptionparam"></a>
## SkillGetDescriptionParam <sup>C</sup>

`SkillGetDescriptionParam` is called when the substitution value for a skill description parameter is being calculated. The function should return the parameter value if it wishes to override the built-in value, or return `nil` if the engine substitution logic should be used. Each part of the colon-separated description param is passed as a separate parameter; i.e. `"Stats:Stats_Slowed:MovementSpeedBoost"` is passed as `(skill, character, isFromItem, "Stats", "Stats_Slowed", "MovementSpeedBoost")`. 
This event must only be registered on the client (since the server has no UI).

Example:
```lua
local skillGetDescriptionParam = function (skill, character, isFromItem, param)
    if skill.Name == "MyCustomSkill" and param == "Constitution" then
        return tostring(character.Constitution)
    end
end

Ext.RegisterListener("SkillGetDescriptionParam", skillGetDescriptionParam)
```

<a id="event-statusgetdescriptionparam"></a>
## StatusGetDescriptionParam <sup>C</sup>

`StatusGetDescriptionParam`  is called when engine requests the value of a status description parameter. The function should return the parameter value if it wishes to override the built-in value, or return `nil` if the engine substitution logic should be used. Each part of the colon-separated description param is passed as a separate parameter; i.e. `"Stats:Stats_Slowed:MovementSpeedBoost"` is passed as `(status, statusSource, character, "Stats", "Stats_Slowed", "MovementSpeedBoost")`.
This event must only be registered on the client (since the server has no UI).

Example:
```lua
local statusGetDescriptionParam = function (status, statusSource, character, param)
    if status.Name == "MyCustomStatus" and param == "HealAmount" then
        if character.Vitality < character.MaxVitality*0.5 then
            return tostring(status.HealAmount)
        else
            return "0"
        end
    end
end

Ext.RegisterListener("StatusGetDescriptionParam", statusGetDescriptionParam)
```

<a id="event-getskilldamage"></a>
## GetSkillDamage

`GetSkillDamage` is called when the engine is calculating the amount of damage dealt by a specific skill.
Note that this function only calculates the base damage amount without taking into account the stats of the target character/item.

Signature:
```lua
local function GetSkillDamage(skill, attacker, isFromItem, stealthed, attackerPos, targetPos, level, noRandomization)
    if skill.Name == "MySkill" then
        local damageList = Ext.NewDamageList()
        damageList:Add("Fire", attacker.Constitution)
        return damageList, "Explode"
    end
end

Ext.RegisterListener("GetSkillDamage", GetSkillDamage)
```

Parameters:
 - `skill` is a stats object representing the skill being invoked (`StatEntrySkillData` type in IDE helpers)
 - `attacker` is the stats object of the attacker character (`StatCharacter` type in IDE helpers)
 - `isFromItem` indicates if the skill was used from an item or not
 - `stealthed` indicates whether the attacker character is in stealth
 - `attackerPos` is a 3-element (XYZ) vector containing the position of the attacker
 - `targetPos` is a 3-element (XYZ) vector containing the position of the target
 - `level` is the skill level being used
 - `noRandomization` determines whether an RNG roll should be used for damage or the damage range should be ignored entirely

The function should return a DamageList object and a DamageType string if it wishes to override the built-in damage calculation formula, or return nothing if the engine formula should be used. 

For a reference implementation that replicates the ingame skill damage calculation logic check out the [Game.Math](https://github.com/Norbyte/ositools/blob/master/OsiInterface/Game.Math.lua) library.


<a id="event-computecharacterhit"></a>
## ComputeCharacterHit <sup>S</sup>

`ComputeCharacterHit` is called when the engine is applying a hit on a specific character.
Since hit logic is run entirely on the server this callback cannot be registered on the client.

Signature:
```lua
local function ComputeCharacterHit(target, attacker, weapon, damageList, hitType, noHitRoll, forceReduceDurability, hit, alwaysBackstab, highGroundFlag, criticalRoll)
    [...]
end

Ext.RegisterListener("ComputeCharacterHit", ComputeCharacterHit)
```

Parameters:
 - `target` is the stats object of the target character (`StatCharacter` type in IDE helpers)
 - `attacker` is the stats object of the attacker character (`StatCharacter` type in IDE helpers)
 - `weapon` is the stats object of the attackers weapon or `nil` if no weapon was used (`StatItem` type in IDE helpers)
 - `damageList` contains the damage amounts being applied (usually the output of `GetSkillDamage`); `DamageList` type in IDE helpers
 - `hitType` contains the type of hit (`DoT`, `Magic`, etc.) from the `HitType` enumeration
 - `noHitRoll` indicates that no RNG roll should be made to determine if the attack hits (i.e. guaranteed hit)
 - `forceReduceDurability` indicates that the durability of the attackers weapon should be reduced
 - `hit` is a table containing parameters of the hit (`HitRequest` type in IDE helpers)
 - `alwaysBackstab` indicates that the used skill has the `AlwaysBackstab` property
 - `highGroundFlag` is a `HighGroundFlag` enumeration value indicating the type of high/low ground bonus that applies
 - `criticalRoll` is a `CriticalRoll` enumeration value indicating how critical hit damage should be applied

The function should update and return the `hit` table if it wishes to override the built-in hit simulation formula, or return nothing if the engine formula should be used. 

For a reference implementation that replicates the ingame hit logic check out the [Game.Math](https://github.com/Norbyte/ositools/blob/master/OsiInterface/Game.Math.lua) library.


<a id="event-beforecharacterapplydamage"></a>
## BeforeCharacterApplyDamage <sup>S</sup>

The `BeforeCharacterApplyDamage` is called before a hit is applied on the character. The damage values passed to this function are final values (include resistances, bonuses, etc.).

Changes made to damage types and amounts are not visible in client pop-ups and combat log messages, but are applied to the health/armor pools. Because of this behavior,  `BeforeCharacterApplyDamage` is ideal for applying systemic changes to damage types (i.e. converting `Fire` damage to behave as `Piercing`, but still show up as `Fire` on the client).

Since hit logic is run entirely on the server this callback cannot be registered on the client.

Example:
```lua
local function BeforeCharacterApplyDamage(target, attacker, hit, causeType, impactDirection)
    hit.DamageList:ConvertDamageType("Corrosive")
end

Ext.RegisterListener("BeforeCharacterApplyDamage", BeforeCharacterApplyDamage)
```

Parameters:
 - `target` is the stats object of the target character (`StatCharacter` type in IDE helpers)
 - `attacker` is the stats object of the attacker character (`StatCharacter` type in IDE helpers)
 - `hit` is a table containing parameters of the hit (`HitRequest` type in IDE helpers)
 - `causeType` is a `CauseType` enumeration value containing the reason for the hit
 - `impactDirection` is a 3D vector containing the direction of the hit

The function should update fields in the `hit` table directly if it wishes to override the built-in hit simulation formula. Please note that unlike other callbacks, changes performed in this function are always applied as it has no return value!


<a id="event-statusgetenterchance"></a>
## StatusGetEnterChance <sup>S</sup>

The `StatusGetEnterChance` listener is called to fetch the chance of entering/ticking a status.
When a status is first applied to a character or item, `StatusGetEnterChance` is called with `isEnterCheck = true`. If the status apply succeeds, `StatusGetEnterChance` is called at the end of each subsequent turn with `isEnterCheck = false`. If the status tick check fails, the status is removed.

Since status ticking is done by the server this callback cannot be registered on the client.

Example:
```lua
local function StatusGetEnterChance(status, isEnterCheck)
    return 100
end

Ext.RegisterListener("StatusGetEnterChance", StatusGetEnterChance)
```

Parameters:
 - `status` is the status being applied/ticked (`EsvStatus` type in IDE helpers)
 - `isEnterCheck` is `true` when the status is being applied and `false` when it is ticked

The function should return an integer value between 0 and 100 representing the enter/tick chance percentage or return `nil` if the engine formula should be used. 

For a reference implementation that replicates the ingame status enter chance logic check out the [Game.Math](https://github.com/Norbyte/ositools/blob/master/OsiInterface/Game.Math.lua) library.


<a id="event-gethitchance"></a>
## GetHitChance

Each time the game calculates hit chance, the `GetHitChance` listener is called. If a Lua script listens to this event and returns a non-`nil` value from the listener function, the game will use the return value of the custom function as the hit chance. If the function returns `nil` or the function call fails, the game's own hit chance calculation is used.
The function is used by the server to perform hit checks and by the client to display the hit chance tooltip.
 
The following example makes it so the overall hit chance is the attacker's Accuracy minus the target's Dodge, no multiplicative operations involved:
```lua
local function YourHitChanceFunction(attacker, target)
    local hitChance = attacker.Accuracy - target.Dodge
    -- Make sure that we return a value in the range (0% .. 100%)
    hitChance = math.max(math.min(hitChance, 100), 0)
    return hitChance
end

Ext.RegisterListener("GetHitChance", YourHitChanceFunction)
```

Parameters:
 - `target` is the stats object of the target character (`StatCharacter` type in IDE helpers)
 - `attacker` is the stats object of the attacker character (`StatCharacter` type in IDE helpers)

The function should return an integer value between 0 and 100 representing the hit chance percentage or return `nil` if the engine formula should be used. 

Be aware that the Hit Chance Calculation considers a lot of variables, including checking if the target is incapacitated. To better approximate vanilla behavior, it is recommended to replicate the majority of the features present on the vanilla's code, changing only what you want to change.

For a reference implementation that replicates the ingame hit chance logic check out the [Game.Math](https://github.com/Norbyte/ositools/blob/master/OsiInterface/Game.Math.lua) library.


## Upgrading

### Migrating from v51 to v52

Object handles were converted from `integer` to `lightuserdata` in v52. This means that APIs that returned or accepted object handles now expect `lightuserdata` parameters. The usage of these APIs and the way parameters should be passed remains unchanged, so this change should not affect most users.

Additional notes:
 - Since both `NetID`s and `ObjectHandle`s are integers, it was previously impossible or very hard to distinguish between the two in APIs that accept both (eg. `Ext.GetItem`, `Ext.GetCharacter`, etc.). `Ext.GetItem` and `Ext.GetCharacter` still accept integer parameters, but will always treat an int parameter as a `NetID` from now on.
 - It is no longer possible to pass handles between the client and the server. Since client handles are useless on the server (and vice versa), this should only be an issue if handles were passed accidentally in network messages.

### Migrating from v44 to v45

It was discovered that there are situations when `ModuleLoading` is not triggered even though the stats were reloaded (this occurs during certain client-server sync situations and when stats were reloaded from the editor console); this causes stats to revert to their original values. A new event (`StatsLoaded`) was added to fix these shortcomings. The `StatsLoaded` event is meant to replace the use of `ModuleLoading` for stat override purposes, i.e. all stats should be edited when `StatsLoaded` is triggered. (The `ModuleLoading` event will be kept for backwards compatibility.)
```lua
local function statsLoaded()
    -- Edit stats here!
end

Ext.RegisterListener("StatsLoaded", statsLoaded)
``` 

### Migrating from v43 to v44

There are no backwards incompatible changes in v44.

### Migrating from v42 to v43

The following changes must be observed when migrating to v43:
- Since it is very easy to accidentally pollute the global table, the global Lua table is changed for mods targeting v43 (i.e. `RequiredExtensionVersion` >= 43). A new config option in `OsiToolsConfig.json`, `ModTable` specifies the name of the mod in the global mod table. Each mod has an isolated mod-specific global table at `Mods[ModTable]`. This table contains all of the symbols from the original global table (`Ext`, `Osi`, ...) and built-in libraries (`math`, `table`, ...), but is otherwise separate. (i.e. declaring `function xyz()` is equivalent to setting `Mods[ModTable].xyz = function () ...`).
- `Ext.Require` is only callable during bootstrap to avoid lag when loading files / doing startup stuff from the server loop
- `Ext.Require` got a one-argument form; i.e. it is sufficient to call `Ext.Require("wtf.lua")` instead of `Ext.Require("SomeUUID_1111-2222-...", "wtf.lua")` (old version will still work) 


### Migrating from v41 to v42

The client and server Lua contexts were previously unified, i.e. both the server and the client used the same Lua state. After v42 server and client contexts are separated.

The following changes must be observed when migrating to v42:
 - `Bootstrap.lua` is deprecated. The server will load `BootstrapServer.lua`, the client will try to load `BootstrapClient.lua`. If these files cannot be found, `Bootstrap.lua` will be loaded as a fallback. In addition, care must be taken to ensure that `BootstrapServer.lua` only contains code that should be run on the server and `BootstrapClient.lua` only contains code for the client.
 - `Ext.GetHitChance` and `Ext.StatusGetEnterChance` are now events; instead of assigning a single function to them as before (i.e. `Ext.GetHitChance = func`), a listener should be registered (`Ext.RegisterListener("GetHitChance", func)`). For backwards compatibility, during the deprecation period assigning a function to `Ext.GetHitChance` is equivalent to registering that function as a listener.
 - Server-side scripts now only allow registering server-side listeners and vice versa
     - Listeners allowed on the server: 	`SessionLoading`, `ModuleLoading`, `ModuleResume`, `GetSkillDamage`, `ComputeCharacterHit`, `CalculateTurnOrder`, `GetHitChance`, `StatusGetEnterChance`
     - Listeners allowed on the client: 	`SessionLoading`, `ModuleLoading`, `ModuleResume`, `GetSkillDamage`, `SkillGetDescriptionParam`, `StatusGetDescriptionParam`, `GetHitChance`
 - Calling `Ext.EnableStatOverride` is no longer necessary; any calls to this function should be removed
 - The following functions are deleted in client contexts: `Ext.NewCall`, `Ext.NewQuery`, `Ext.NewEvent`, `Ext.GetCharacter`, `Ext.GetItem`, `Ext.GetStatus`, `Ext.GetCombat`, `Ext.GenerateIdeHelpers`



### TODO
 - Game.Tooltip usage
 - GameObject calls - HasTag, GetTags, GetStatus, GetStatusByType, GetStatuses + GetInventoryItems
 - GetCharacter, GetItem, GetProjectile, GetGameObject, GetStatus (client/server) + updated property maps
 - RegisterNetListener + networking concepts
 - NetID, ObjectHandle, GUID concepts
 - File IO
 - Reloading Lua and changing exports
