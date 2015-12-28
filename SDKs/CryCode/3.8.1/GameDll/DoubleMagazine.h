#pragma once

#ifndef _DOUBLE_MAGAZINE_H_
#define _DOUBLE_MAGAZINE_H_


#include "Accessory.h"


class CDoubleMagazine : public CAccessory
{
public:
	CDoubleMagazine();

	virtual void OnAttach(bool attach);
	virtual void OnParentReloaded();
	virtual void SetAccessoryReloadTags(CTagState& fragTags);

private:
	bool m_reloadFaster;
};



#endif
