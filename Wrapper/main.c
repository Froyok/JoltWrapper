#if defined(_MSC_VER)
	#define WRAP_EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
	#define WRAP_EXPORT __attribute__((visibility("default")))
#else
	#define WRAP_EXPORT
#endif

#define MAX_RESULTS 2048

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

//--------------------------------------------
// Jolt
//--------------------------------------------
typedef struct Jolt_Body Jolt_Body;
typedef uint32_t JPH_BodyID;
typedef uint32_t JPH_SubShapeID;
typedef struct JPH_SubShapeIDPair
{
	JPH_BodyID Body1ID;
	JPH_SubShapeID subShapeID1;
	JPH_BodyID Body2ID;
	JPH_SubShapeID subShapeID2;
} JPH_SubShapeIDPair;

typedef struct WrContactAddedResult
{
	const Jolt_Body* Body1;
	const Jolt_Body* Body2;
} WrContactAddedResult;

typedef struct WrContactRemovedResult
{
	JPH_BodyID BodyID1;
	JPH_BodyID BodyID2;
} WrContactRemovedResult;


//--------------------------------------------
// Result storage and access
//--------------------------------------------
typedef struct WrResultStorage
{
	int ResultAddedCount;
	WrContactAddedResult ArrayAddedResults[MAX_RESULTS];

	int ResultRemovedCount;
	WrContactRemovedResult ArrayRemovedResults[MAX_RESULTS];

	pthread_mutex_t MutexAdded;
	pthread_mutex_t MutexRemoved;
} WrResultStorage;

WrResultStorage Storage;

WRAP_EXPORT void WrInitMutex( void )
{
	pthread_mutex_init( &Storage.MutexAdded, NULL );
	pthread_mutex_init( &Storage.MutexRemoved, NULL );
}

WRAP_EXPORT void WrDestroyMutex( void )
{
	pthread_mutex_destroy( &Storage.MutexAdded );
	pthread_mutex_destroy( &Storage.MutexRemoved );
}

WRAP_EXPORT WrResultStorage* WrGetStorage( void )
{
	return &Storage;
}

WRAP_EXPORT void WrResetStorage( void )
{
	Storage.ResultAddedCount = 0;
	Storage.ResultRemovedCount = 0;
}

WRAP_EXPORT int WrGetAddedResultCount( void )
{
	return Storage.ResultAddedCount;
}

WRAP_EXPORT int WrGetRemovedResultCount( void )
{
	return Storage.ResultRemovedCount;
}

WRAP_EXPORT WrContactAddedResult* WrGetAddedResult( int Index )
{
	return &Storage.ArrayAddedResults[Index];
}

WRAP_EXPORT WrContactRemovedResult* WrGetRemovedResult( int Index )
{
	return &Storage.ArrayRemovedResults[Index];
}


//--------------------------------------------
// Callback
//--------------------------------------------
// - Use mutex lock to prevent other threads to edit the
// array (they will stall until they can lock themselves).
// - Lock before we retrieve the results count since read
// access wouldn't be thread safe either.
//--------------------------------------------
typedef struct Jolt_ContactManifold Jolt_ContactManifold;
typedef struct Jolt_ContactSettings Jolt_ContactSettings;

WRAP_EXPORT void WrOnContactAdded(
	void* userData,
	const Jolt_Body* body1,
	const Jolt_Body* body2,
	const Jolt_ContactManifold* manifold,
	Jolt_ContactSettings* settings
)
{
	WrResultStorage *GlobalStorage = userData;

	pthread_mutex_lock( &GlobalStorage->MutexAdded );

	// Discard events that can't be stored
	if( GlobalStorage->ResultAddedCount < MAX_RESULTS )
	{
		// Increment counter for next call check
		int Index = GlobalStorage->ResultAddedCount;
		GlobalStorage->ResultAddedCount += 1;

		// Add new result
		GlobalStorage->ArrayAddedResults[ Index ].Body1 = body1;
		GlobalStorage->ArrayAddedResults[ Index ].Body2 = body2;
	}
	else
	{
		printf( "[JoltWrapper] Storage is full. Can't register OnContactAdded event.\n" );
	}

	pthread_mutex_unlock( &GlobalStorage->MutexAdded );
}

WRAP_EXPORT void WrOnContactRemoved(
	void* userData,
	const JPH_SubShapeIDPair* subShapePair
)
{
	WrResultStorage *GlobalStorage = userData;

	pthread_mutex_lock( &GlobalStorage->MutexRemoved );

	// Discard events that can't be stored
	if( GlobalStorage->ResultRemovedCount < MAX_RESULTS )
	{
		// Increment counter for next call check
		int Index = GlobalStorage->ResultRemovedCount;
		GlobalStorage->ResultRemovedCount += 1;

		// Add new result
		GlobalStorage->ArrayRemovedResults[ Index ].BodyID1 = subShapePair->Body1ID;
		GlobalStorage->ArrayRemovedResults[ Index ].BodyID2 = subShapePair->Body2ID;
	}
	else
	{
		printf( "[JoltWrapper] Storage is full. Can't register OnContactRemoved event.\n" );
	}

	pthread_mutex_unlock( &GlobalStorage->MutexRemoved );
}
