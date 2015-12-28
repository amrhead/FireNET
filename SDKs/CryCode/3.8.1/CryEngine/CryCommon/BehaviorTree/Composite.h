#pragma once

#ifndef Composite_h
#define Composite_h

#include "Node.h"

#ifdef USING_BEHAVIOR_TREE_SERIALIZATION
#include <Serialization/BoostSharedPtr.h>
#endif

namespace BehaviorTree
{
	template <typename ChildType = INodePtr>
	class Composite : public Node
	{
		typedef Node BaseClass;

	public:
		void AddChild(const ChildType& child)
		{
			m_children.push_back(child);
		}

		virtual LoadResult LoadFromXml( const XmlNodeRef& xml, const LoadContext& context ) override
		{
			return BaseClass::LoadFromXml( xml, context );
		}

#ifdef USING_BEHAVIOR_TREE_SERIALIZATION
		virtual void Serialize( Serialization::IArchive& archive ) override
		{
			archive( m_children, "children", "^[+<>]" );

			if( archive.IsEdit() )
			{
				for( int i = 0; i < m_children.size(); ++i )
				{
					if( !m_children[i] )
						archive.Error( m_children, "Node must be specified." );
				}
			}

			BaseClass::Serialize( archive );
		}
#endif

#ifdef USING_BEHAVIOR_TREE_XML_DESCRIPTION_CREATION
		virtual XmlNodeRef CreateXmlDescription() override
		{
			XmlNodeRef xml = GetISystem()->CreateXmlNode( "Composite" );

			for( int i = 0; i < m_children.size(); ++i )
			{
				if( m_children[i] )
					xml->addChild( m_children[i]->CreateXmlDescription() );
			}

			return xml;
		}
#endif

	protected:
		typedef std::vector<ChildType> Children;
		typedef typename Children::iterator ChildIterator;
		Children m_children;
	};

	class CompositeWithChildLoader : public Composite<INodePtr>
	{
		typedef Composite<INodePtr> BaseClass;

	public:
		virtual LoadResult LoadFromXml(const XmlNodeRef& xml, const LoadContext& context) override
		{
			IF_UNLIKELY (BaseClass::LoadFromXml(xml, context) == LoadFailure)
				return LoadFailure;

			return ConstructChildNodesFromXml(xml, context);
		}

	protected:
		LoadResult ConstructChildNodesFromXml(const XmlNodeRef& xml, const LoadContext& context)
		{
			for (int i = 0; i < xml->getChildCount(); ++i)
			{
				INodePtr child = context.nodeFactory.CreateNodeFromXml(xml->getChild(i), context);

				if (child)
				{
					AddChild(child);
				}
				else
				{
					return LoadFailure;
				}
			}

			return !m_children.empty() ? LoadSuccess : LoadFailure;
		}
	};
}

#endif // Composite_h
