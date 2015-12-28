/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2009.
-------------------------------------------------------------------------

Mock for ICrySizer interface objects

-------------------------------------------------------------------------
History:
- 13:05:2009   Created by Federico Rebora

*************************************************************************/

#include "StdAfx.h"
#include "MockCrySizer.h"

CMockCrySizer::CMockCrySizer()
: m_totalSize(0)
{

}

void CMockCrySizer::Release()
{

}

size_t CMockCrySizer::GetTotalSize()
{
    return m_totalSize;
}

bool CMockCrySizer::AddObject(const void* identifier, size_t sizeInBytes, int count)
{
    m_totalSize += sizeInBytes;

    return true;
}

IResourceCollector& CMockCrySizer::GetResourceCollector()
{
    CRY_TODO(12, 5, 2009, "We might eventually implement this as a Mock");

    IResourceCollector* nullCollector = 0;

    return *nullCollector;
}

void CMockCrySizer::Push(const char* componentName)
{

}

void CMockCrySizer::PushSubcomponent(const char* subcomponentName)
{

}

void CMockCrySizer::Pop()
{

}

void CMockCrySizer::Reset()
{

}
