typedef uint32_t JPH_BodyID;
typedef struct JPH_Body JPH_Body;
typedef struct JPH_ContactManifold JPH_ContactManifold;
typedef struct JPH_ContactSettings JPH_ContactSettings;
typedef struct JPH_SubShapeIDPair JPH_SubShapeIDPair;

WRAP_EXPORT void WrInitMutex();

WRAP_EXPORT void WrDestroyMutex();

WRAP_EXPORT void WrOnContactAdded(
	void* userData,
	const JPH_Body* body1,
	const JPH_Body* body2,
	const JPH_ContactManifold* manifold,
	JPH_ContactSettings* settings
);

WRAP_EXPORT void WrOnContactRemoved(
	void* userData,
	const JPH_SubShapeIDPair* subShapePair
);

typedef struct WrResultStorage WrResultStorage;

typedef struct WrContactAddedResult
{
	const JPH_Body* Body1;
	const JPH_Body* Body2;
} WrContactAddedResult;

typedef struct WrContactRemovedResult
{
	JPH_BodyID BodyID1;
	JPH_BodyID BodyID2;
} WrContactRemovedResult;

WRAP_EXPORT WrResultStorage* WrGetStorage( void );

WRAP_EXPORT void WrResetStorage( void );

WRAP_EXPORT int WrGetAddedResultCount();

WRAP_EXPORT int WrGetRemovedResultCount();

WRAP_EXPORT WrContactAddedResult* WrGetAddedResult( int Index );

WRAP_EXPORT WrContactRemovedResult* WrGetRemovedResult( int Index );
