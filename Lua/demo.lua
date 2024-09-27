local FFI = require( "ffi" )

-------------------------------------
-- Load C definitions
-------------------------------------
local function LoadDefinitions()
	local Definitions = ""
	local Paths = {
		RootPath .. "/cdef_jolt_wrapper.c",
		RootPath .. "/cdef_jolt.c"
	}

	for i=1, #Paths do
		for Line in love.filesystem.lines( Paths[i] ) do
			Definitions = Definitions .. Line .. "\n"
		end

		Definitions = Definitions .. "\n"
	end

	if FFI.os == "Windows" then
		Definitions = Definitions:gsub( "JPH_API_CALL", "__cdecl" )
		Definitions = Definitions:gsub( "WRAP_EXPORT", "__cdecl" )
	else
		Definitions = Definitions:gsub( "JPH_API_CALL", "" )
		Definitions = Definitions:gsub( "WRAP_EXPORT", "" )
	end

	return Definitions
end

FFI.cdef( LoadDefinitions() )

-------------------------------------
-- Init libs
-------------------------------------
Path = assert( package.searchpath("libjolt_wrapper", package.cpath) )
PHYSICS.WRAPPER = FFI.load( Path )
PHYSICS.WRAPPER.WrInitMutex()

Path = assert( package.searchpath("libjoltc", package.cpath) )
PHYSICS.C = FFI.load( Path )
PHYSICS_C = PHYSICS.C

-------------------------------------
-- Init physics
-------------------------------------
PHYSICS.System = PHYSICS_C.JPH_PhysicsSystem_Create( Settings )

-------------------------------------
-- Event listener
-------------------------------------
PHYSICS.Listener = {
  ContactListener = PHYSICS_C.JPH_ContactListener_Create(),
  Functions = FFI.new( "JPH_ContactListener_Procs" )
}

PHYSICS.Listener.Functions.OnContactAdded = PHYSICS.WRAPPER.WrOnContactAdded
PHYSICS.Listener.Functions.OnContactRemoved = PHYSICS.WRAPPER.WrOnContactRemoved

PHYSICS_C.JPH_ContactListener_SetProcs(
  PHYSICS.Listener.ContactListener,
  PHYSICS.Listener.Functions,
  PHYSICS.WRAPPER.WrGetStorage()
)

PHYSICS_C.JPH_PhysicsSystem_SetContactListener(
  PHYSICS.System,
  PHYSICS.Listener.ContactListener
)


-------------------------------------
-- Update loop
-------------------------------------
-- Update physics
PHYSICS_C.JPH_PhysicsSystem_Step(
  PHYSICS.System,
  DeltaTime,
  2
)

-- Now retrieve events
local WRAPPER = PHYSICS.WRAPPER
local Counts = 0
local Result = nil
local ID = nil
local Target = nil

-- Handle added contacts
local CanAddToTrigger = false
Counts = WRAPPER.WrGetAddedResultCount()

for i=1, Counts do
  Result = WRAPPER.WrGetAddedResult( i - 1 )
  CanAddToTrigger = false

  if PHYSICS_C.JPH_Body_IsSensor( Result.Body1 ) then
    ID = PHYSICS_C.JPH_Body_GetID( Result.Body1 )
    Target = Result.Body2
    CanAddToTrigger = true
  elseif PHYSICS_C.JPH_Body_IsSensor( Result.Body2 ) then
    ID = PHYSICS_C.JPH_Body_GetID( Result.Body2 )
    Target = Result.Body1
    CanAddToTrigger = true
  end

  if PHYSICS.Triggers[ID] and CanAddToTrigger then
    PHYSICS.Triggers[ID].OverlappingBodies[Target] = true
  end
end

-- Handle removed contacts
local CanAddToTrigger = false
local BodyID1 = nil
local BodyID2 = nil
Counts = WRAPPER.WrGetRemovedResultCount()

for i=1, Counts do
  Result = WRAPPER.WrGetRemovedResult( i - 1 )
  BodyID1 = Result.BodyID1
  BodyID2 = Result.BodyID2

  if PHYSICS.Triggers[BodyID1] then
    Target = PHYSICS.BodiesInfo[BodyID2]
    PHYSICS.Triggers[BodyID1].OverlappingBodies[Target] = false
  elseif PHYSICS.Triggers[BodyID2] then
    Target = PHYSICS.BodiesInfo[BodyID1]
    PHYSICS.Triggers[BodyID2].OverlappingBodies[Target] = false
  end
end

-- Cleanup
WRAPPER.WrResetStorage()
