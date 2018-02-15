#include "pch.h"
#include "dataTypes.h"

/// <summary>
/// Creates an integer default type
/// </summary>
Ref<DefaultType> DefaultType::createInt()
{
	return refFromNew(new DefaultType(DT_INT));
}

/// <summary>
/// Creates a boolean default type
/// </summary>
Ref<DefaultType> DefaultType::createBool()
{
	return refFromNew(new DefaultType(DT_BOOL));
}

/// <summary>
/// Creates a void default type
/// </summary>
Ref<DefaultType> DefaultType::createVoid()
{
	return refFromNew(new DefaultType(DT_VOID));
}
