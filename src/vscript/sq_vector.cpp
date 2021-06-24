#include "mathlib/vector.h"
#include "tier1/fmtstr.h"
#include "tier1/strtools.h"

#include "squirrel.h"
#include "sqobject.h"
#include "sq_vector.h"


#define sq_checkvector(vm, vector) \
	if ( vector == nullptr ) { return sq_throwerror( vm, "Null vector" ); }

#define sq_pushvector(vm, vector) \
	sq_getclass( vm, -1 ); \
	sq_createinstance( vm, -1 ); \
	sq_setinstanceup( vm, -1, vector ); \
	sq_setreleasehook( vm, -1, &VectorRelease ); \
	sq_remove( vm, -2 );

Vector GetVectorByValue( HSQUIRRELVM pVM, int nIndex )
{
	// support vector = vector + 15
	if ( sq_gettype( pVM, nIndex ) & SQOBJECT_NUMERIC )
	{
		SQFloat flValue = 0;
		sq_getfloat( pVM, nIndex, &flValue );
		return Vector( flValue );
	}

	SQUserPointer up = NULL, tag = NULL;
	sq_getinstanceup( pVM, nIndex, &up, &tag );
	Vector *pVector = (Vector *)up;
	if ( pVector == nullptr )
	{
		sq_throwerror( pVM, "Null vector" );
		return Vector();
	}

	return *pVector;
}

SQInteger VectorConstruct( HSQUIRRELVM pVM )
{
	Vector *pVector = new Vector;

	int i, _top = sq_gettop( pVM );
	for ( i=0; i < _top - 1 && i < 3; ++i )
	{
		sq_getfloat( pVM, i + 2, &( *pVector )[i] );
	}

	if ( i < 3 )
	{
		for( ; i<3; ++i )
			(*pVector)[i] = 0;
	}

	sq_setinstanceup( pVM, 1, pVector );
	sq_setreleasehook( pVM, 1, &VectorRelease );

	return SQ_OK;
}

SQInteger VectorRelease( SQUserPointer up, SQInteger size )
{
	delete (Vector *)up;
	return SQ_OK;
}

SQInteger VectorGet( HSQUIRRELVM pVM )
{
	SQUserPointer up = NULL, tag = NULL;
	sq_getinstanceup( pVM, 1, &up, &tag );
	Vector *pVector = (Vector *)up;
	sq_checkvector( pVM, pVector );

	const SQChar *pString = NULL;
	sq_getstring( pVM, 2, &pString );
	// Are we using the table accessor correctly?
	if ( pString == NULL || *pString == '\0' )
		return SQ_ERROR;

	// Error on using additional characters
	if ( pString[1] != '\0' )
		return SQ_ERROR;

	// Accessing x, y or z
	if ( pString[0] - 'x' < 3 )
	{
		sq_pushfloat( pVM, ( *pVector )[pString[0] - 'x'] );
		return 1;
	}

	return SQ_ERROR;
}

SQInteger VectorSet( HSQUIRRELVM pVM )
{
	SQUserPointer up = NULL, tag = NULL;
	sq_getinstanceup( pVM, 1, &up, &tag );
	Vector *pVector = (Vector *)up;
	sq_checkvector( pVM, pVector );

	const SQChar *pString = NULL;
	sq_getstring( pVM, 2, &pString );
	// Are we using the table accessor correctly?
	if ( pString == NULL || *pString == '\0' )
		return SQ_ERROR;

	// Error on using additional characters
	if ( pString[1] != '\0' )
		return SQ_ERROR;

	// Accessing x, y or z
	if ( pString[0] - 'x' < 3 )
	{
		SQFloat flValue = 0;
		sq_getfloat( pVM, 3, &flValue );

		(*pVector)[ pString[0] - 'x' ] = flValue;
		sq_pushfloat( pVM, flValue );
		return 1;
	}

	return SQ_ERROR;
}

SQInteger VectorToString( HSQUIRRELVM pVM )
{
	SQUserPointer up = NULL, tag = NULL;
	sq_getinstanceup( pVM, 1, &up, &tag );
	Vector *pVector = (Vector *)up;
	sq_checkvector( pVM, pVector );

	sq_pushstring( pVM, CFmtStr("(vector : (%f, %f, %f))", VectorExpand( *pVector ) ), -1 );
	return 1;
}

SQInteger VectorTypeInfo( HSQUIRRELVM pVM )
{
	sq_pushstring( pVM, "Vector", -1 );
	return 1;
}

SQInteger VectorEquals( HSQUIRRELVM pVM )
{
	SQUserPointer up = NULL, tag = NULL;

	sq_getinstanceup( pVM, 1, &up, &tag );
	Vector *pLHS = (Vector *)up;
	sq_checkvector( pVM, pLHS );

	sq_getinstanceup( pVM, 2, &up, &tag );
	Vector *pRHS = (Vector *)up;
	sq_checkvector( pVM, pRHS );

	sq_pushbool( pVM, VectorsAreEqual( *pLHS, *pRHS, 0.01 ) );
	return 1;
}

SQInteger VectorIterate( HSQUIRRELVM pVM )
{
	if ( sq_gettop( pVM ) < 2 )
		return SQ_ERROR;

	SQChar const *szAccessor = NULL;
	if ( sq_gettype( pVM, 2 ) == OT_NULL )
	{
		szAccessor = "w";
	}
	else
	{
		sq_getstring( pVM, 2, &szAccessor );
		if ( !szAccessor || !*szAccessor )
			return SQ_ERROR;
	}

	if ( szAccessor[1] != '\0' )
		return SQ_ERROR;

	static char const *const results[] ={
		"x",
		"y",
		"z"
	};

	// Accessing x, y or z
	if ( szAccessor[0] - 'x' < 3 )
		sq_pushstring( pVM, results[ szAccessor[0] - 'x' ], -1 );
	else
		sq_pushnull( pVM );

	return 1;
}

SQInteger VectorAdd( HSQUIRRELVM pVM )
{
	Vector LHS = GetVectorByValue( pVM, 1 );
	Vector RHS = GetVectorByValue( pVM, 2 );

	// Create a new vector so we can keep the values of the other
	Vector *pNewVector = new Vector;
	*pNewVector = LHS + RHS;

	sq_pushvector( pVM, pNewVector );

	return 1;
}

SQInteger VectorSubtract( HSQUIRRELVM pVM )
{
	Vector LHS = GetVectorByValue( pVM, 1 );
	Vector RHS = GetVectorByValue( pVM, 2 );

	// Create a new vector so we can keep the values of the other
	Vector *pNewVector = new Vector;
	*pNewVector = LHS - RHS;

	sq_pushvector( pVM, pNewVector );

	return 1;
}

SQInteger VectorMultiply( HSQUIRRELVM pVM )
{
	Vector LHS = GetVectorByValue( pVM, 1 );
	Vector RHS = GetVectorByValue( pVM, 2 );

	// Create a new vector so we can keep the values of the other
	Vector *pNewVector = new Vector;
	*pNewVector = LHS * RHS;

	sq_pushvector( pVM, pNewVector );

	return 1;
}

SQInteger VectorDivide( HSQUIRRELVM pVM )
{
	Vector LHS = GetVectorByValue( pVM, 1 );
	Vector RHS = GetVectorByValue( pVM, 2 );

	// Create a new vector so we can keep the values of the other
	Vector *pNewVector = new Vector;
	*pNewVector = LHS / RHS;

	sq_pushvector( pVM, pNewVector );

	return 1;
}

SQInteger VectorToKeyValue( HSQUIRRELVM pVM )
{
	SQUserPointer up = NULL, tag = NULL;
	sq_getinstanceup( pVM, 1, &up, &tag );
	Vector *pVector = (Vector *)up;
	sq_checkvector( pVM, pVector );

	sq_pushstring( pVM, CFmtStr( "(vector : (%f, %f, %f))", VectorExpand( *pVector ) ), -1 );
	return 1;
}

SQInteger VectorLength( HSQUIRRELVM pVM )
{
	SQUserPointer up = NULL, tag = NULL;
	sq_getinstanceup( pVM, 1, &up, &tag );
	Vector *pVector = (Vector *)up;
	sq_checkvector( pVM, pVector );

	sq_pushfloat( pVM, pVector->Length() );
	return 1;
}

SQInteger VectorLengthSqr( HSQUIRRELVM pVM )
{
	SQUserPointer up = NULL, tag = NULL;
	sq_getinstanceup( pVM, 1, &up, &tag );
	Vector *pVector = (Vector *)up;
	sq_checkvector( pVM, pVector );

	sq_pushfloat( pVM, pVector->LengthSqr() );
	return 1;
}

SQInteger VectorLength2D( HSQUIRRELVM pVM )
{
	SQUserPointer up = NULL, tag = NULL;
	sq_getinstanceup( pVM, 1, &up, &tag );
	Vector *pVector = (Vector *)up;
	sq_checkvector( pVM, pVector );

	sq_pushfloat( pVM, pVector->Length2D() );
	return 1;
}

SQInteger VectorLength2DSqr( HSQUIRRELVM pVM )
{
	SQUserPointer up = NULL, tag = NULL;
	sq_getinstanceup( pVM, 1, &up, &tag );
	Vector *pVector = (Vector *)up;
	sq_checkvector( pVM, pVector );

	sq_pushfloat( pVM, pVector->Length2DSqr() );
	return 1;
}

SQInteger VectorDotProduct( HSQUIRRELVM pVM )
{
	SQUserPointer up = NULL, tag = NULL;

	sq_getinstanceup( pVM, 1, &up, &tag );
	Vector *pLHS = (Vector *)up;
	sq_checkvector( pVM, pLHS );

	sq_getinstanceup( pVM, 2, &up, &tag );
	Vector *pRHS = (Vector *)up;
	sq_checkvector( pVM, pRHS );

	sq_pushfloat( pVM, pLHS->Dot( *pRHS ) );
	return 1;
}

SQInteger VectorCrossProduct( HSQUIRRELVM pVM )
{
	SQUserPointer up = NULL, tag = NULL;

	sq_getinstanceup( pVM, 1, &up, &tag );
	Vector *pLHS = (Vector *)up;
	sq_checkvector( pVM, pLHS );

	sq_getinstanceup( pVM, 2, &up, &tag );
	Vector *pRHS = (Vector *)up;
	sq_checkvector( pVM, pRHS );

	// Create a new vector so we can keep the values of the other
	Vector *pNewVector = new Vector;
	*pNewVector = pLHS->Cross( *pRHS );

	sq_pushvector( pVM, pNewVector );

	return 1;
}

SQInteger VectorNormalize( HSQUIRRELVM pVM )
{
	SQUserPointer up = NULL, tag = NULL;
	sq_getinstanceup( pVM, 1, &up, &tag );
	Vector *pVector = (Vector *)up;
	sq_checkvector( pVM, pVector );

	const float flLength = VectorNormalize( *pVector );

	sq_pushfloat( pVM, flLength );
	return 1;
}


SQRegFunction g_VectorFuncs[] ={
	{_SC( "constructor" ),		VectorConstruct				},
	{MM_GET,					VectorGet					},
	{MM_SET,					VectorSet,			2,		_SC( ".." )},
	{MM_TOSTRING,				VectorToString,		3,		_SC( "..n" )},
	{MM_TYPEOF,					VectorTypeInfo				},
	{MM_CMP,					VectorEquals,		2,		0},
	{MM_NEXTI,					VectorIterate				},
	{MM_ADD, 					VectorAdd,			2,		0},
	{MM_SUB,					VectorSubtract,		2,		0},
	{MM_MUL,					VectorMultiply,		2,		0},
	{MM_DIV,					VectorDivide,		2,		0},
	{_SC( "Length" ),			VectorLength				},
	{_SC( "LengthSqr" ),		VectorLengthSqr				},
	{_SC( "Length2D" ),			VectorLength2D				},
	{_SC( "Length2DSqr" ),		VectorLength2DSqr			},
	{_SC( "Dot" ),				VectorDotProduct,	2,		0},
	{_SC( "Cross" ),			VectorCrossProduct,	2,		0},
	{_SC( "Norm" ),				VectorNormalize				},
	{_SC( "ToKVString" ),		VectorToKeyValue			}
};

SQRESULT RegisterVector( HSQUIRRELVM pVM )
{
	int nArgs = sq_gettop( pVM );

	// Register a new class of name Vector
	sq_pushroottable( pVM );
	sq_pushstring( pVM, "Vector", -1 );

	// Something went wrong, bail and reset
	if ( SQ_FAILED( sq_newclass( pVM, SQFalse ) ) )
	{
		sq_settop( pVM, nArgs );
		return SQ_ERROR;
	}

	HSQOBJECT pTable{};

	// Setup class table
	sq_getstackobj( pVM, -1, &pTable );
	sq_settypetag( pVM, -1, VECTOR_TYPE_TAG );

	// Add to VM
	sq_createslot( pVM, -3 );

	// Prepare table for insert
	sq_pushobject( pVM, pTable );

	for ( int i = 1; i < ARRAYSIZE( g_VectorFuncs ); ++i )
	{
		SQRegFunction *reg = &g_VectorFuncs[i];

		// Register function
		sq_pushstring( pVM, reg->name, -1 );
		sq_newclosure( pVM, reg->f, 0 );

		// Setup param enforcement if available
		if ( reg->nparamscheck != 0 )
			sq_setparamscheck( pVM, reg->nparamscheck, reg->typemask );

		// for debugging
		sq_setnativeclosurename( pVM, -1, reg->name );

		// Add to table
		sq_createslot( pVM, -3 );
	}

	// Reset vm
	sq_pop( pVM, 1 );
	sq_settop( pVM, nArgs );

	return SQ_OK;
}
