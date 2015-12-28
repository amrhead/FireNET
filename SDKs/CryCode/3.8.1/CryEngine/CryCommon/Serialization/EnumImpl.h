// Copyright (c) 2012 Crytek GmbH
// Authors: Evgeny Andreeshchev, Alexander Kotliar
// Based on: Yasli - the serialization library.

#pragma once
#include "IArchive.h"
#include "STL.h"
#include "Enum.h"
#include "StringList.h"
#ifndef SERIALIZATION_STANDALONE
#include <ISystem.h>
#endif

namespace Serialization{

inline void CEnumDescription::add(int value, const char* name, const char *label)
{
	YASLI_ESCAPE( name && label, return );
	nameToValue_[name] = value;
	labelToValue_[label] = value;
	valueToName_[value] = name;
	valueToLabel_[value] = label;
	valueToIndex_[value] = int(names_.size());
	names_.push_back(name);
	labels_.push_back(label);
	values_.push_back(value);
}

inline bool CEnumDescription::Serialize(IArchive& ar, int& value, const char* name, const char* label) const
{
	if (!ar.IsInPlace())
	{
		if(count() == 0){
#ifdef SERIALIZATION_STANDALONE
			assert(0 && "Attempt to serialize enum type that is not registered with SERIALIZATION_ENUM macro");
#else
			CryFatalError("Attempt to serialize enum type that is not registered with SERIALIZATION_ENUM macro: %s", type().name());
#endif
			return false;
		}

		int index = StringListStatic::npos;
		if(ar.IsOutput()){
			index =  indexByValue(value);
		}
		StringListStaticValue stringListValue(ar.IsEdit() ? labels() : names(), index, &value, type());
		ar(stringListValue, name, label);
		if(ar.IsInput()){
			if(stringListValue.index() == StringListStatic::npos)
				return false;
			value = ar.IsEdit() ? valueByLabel(stringListValue.c_str()) : this->value(stringListValue.c_str());
		}
		else if(index == StringListStatic::npos){
			ar.Error(&value, type(), "Unregistered or uninitialized enumeration value.");
		}
	}
	else
	{
		return ar(value, name, label);
	}
	return true;
}

inline bool CEnumDescription::serializeBitVector(IArchive& ar, int& value, const char* name, const char* label) const
{
    if(ar.IsOutput())
    {
        StringListStatic names = nameCombination(value);
		string str;
        joinStringList(&str, names, '|');
        return ar(str, name, label);
    }
    else
    {
		string str;
        if(!ar(str, name, label))
            return false;
        StringList values;
        splitStringList(&values, str.c_str(), '|');
        StringList::iterator it;
        value = 0;
        for(it = values.begin(); it != values.end(); ++it)
			if(!it->empty())
				value |= this->value(it->c_str());
		return true;
    }
}


inline const char* CEnumDescription::name(int value) const
{
    ValueToName::const_iterator it = valueToName_.find(value);
    YASLI_ESCAPE(it != valueToName_.end(), return "");
    return it->second;
}
inline const char* CEnumDescription::label(int value) const
{
    ValueToLabel::const_iterator it = valueToLabel_.find(value);
    YASLI_ESCAPE(it != valueToLabel_.end(), return "");
    return it->second;
}

inline StringListStatic CEnumDescription::nameCombination(int bitVector) const 
{
    StringListStatic strings;
    for(ValueToName::const_iterator i = valueToName_.begin(); i != valueToName_.end(); ++i)
        if((bitVector & i->first) == i->first){
            bitVector &= ~i->first;
            strings.push_back(i->second);
        }
	YASLI_ASSERT(!bitVector && "Unregistered enum value");
    return strings;
}

inline StringListStatic CEnumDescription::labelCombination(int bitVector) const 
{
    StringListStatic strings;
    for(ValueToLabel::const_iterator i = valueToLabel_.begin(); i != valueToLabel_.end(); ++i)
        if(i->second && (bitVector & i->first) == i->first){
            bitVector &= ~i->first;
            strings.push_back(i->second);
        }
	YASLI_ASSERT(!bitVector && "Unregistered enum value");
	return strings;
}


inline int CEnumDescription::indexByValue(int value) const
{
    ValueToIndex::const_iterator it = valueToIndex_.find(value);
    if(it == valueToIndex_.end())
        return -1;
    else
        return it->second;
}

inline int CEnumDescription::valueByIndex(int index) const
{
	if (size_t(index) < values_.size())
		return values_[index];
	return 0;
}

inline const char* CEnumDescription::nameByIndex(int index) const
{
	if (size_t(index) < size_t(names_.size()))
		return names_[size_t(index)];
	return 0;
}

inline const char* CEnumDescription::labelByIndex(int index) const
{
	if (size_t(index) < size_t(labels_.size()))
		return labels_[size_t(index)];
	return 0;
}

inline int CEnumDescription::value(const char* name) const
{
    NameToValue::const_iterator it = nameToValue_.find(name);
    YASLI_ESCAPE(it != nameToValue_.end(), return 0);
    return it->second;
}
inline int CEnumDescription::valueByLabel(const char* label) const
{
    LabelToValue::const_iterator it = labelToValue_.find(label);
    YASLI_ESCAPE(it != labelToValue_.end(), return 0);
    return it->second;
}

}
// vim:ts=4 sw=4:
