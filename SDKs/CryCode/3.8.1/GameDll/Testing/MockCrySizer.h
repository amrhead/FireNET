/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2009.
-------------------------------------------------------------------------

Mock for ICrySizer interface objects

-------------------------------------------------------------------------
History:
- 13:05:2009   Created by Federico Rebora

*************************************************************************/

#ifndef MOCK_CRY_SIZER_H_INCLUDED
#define MOCK_CRY_SIZER_H_INCLUDED

class CMockCrySizer : public ICrySizer
{
public:
    CMockCrySizer();

    virtual void Release();

    virtual size_t GetTotalSize();

    virtual bool AddObject(const void* identifier, size_t sizeInBytes, int count);

    virtual IResourceCollector& GetResourceCollector();

    virtual void Push(const char* componentName);

    virtual void PushSubcomponent(const char* subcomponentName);

    virtual void Pop();

		virtual void Reset();

private:
    size_t m_totalSize;
};

#endif //MOCK_CRY_SIZER_H_INCLUDED
