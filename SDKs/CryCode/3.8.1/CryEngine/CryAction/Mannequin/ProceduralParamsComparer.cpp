#include "StdAfx.h"
#include <CryExtension/Impl/ClassWeaver.h>
#include <ICryMannequin.h>
#include <Mannequin/Serialization.h>
#include <Serialization/IArchiveHost.h>

class CProceduralParamsComparerDefault
: public IProceduralParamsComparer
{
public:
	CRYINTERFACE_BEGIN()
		CRYINTERFACE_ADD(IProceduralParamsComparer)
	CRYINTERFACE_END()

	CRYGENERATE_CLASS(CProceduralParamsComparerDefault, IProceduralParamsComparerDefaultName, 0xfc53bd9248534faa, 0xab0fb42b24e55b3e)

	virtual bool Equal(const IProceduralParams& lhs, const IProceduralParams& rhs) const
	{
		return Serialization::CompareBinary(lhs, rhs);
	}
};

CProceduralParamsComparerDefault::CProceduralParamsComparerDefault()
{
}

CProceduralParamsComparerDefault::~CProceduralParamsComparerDefault()
{
}

CRYREGISTER_CLASS( CProceduralParamsComparerDefault );

