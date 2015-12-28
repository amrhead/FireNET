#pragma once

#include "BehaviorTree/IBehaviorTree.h"

namespace BehaviorTree
{
	class XmlLoader
	{
	public:
		XmlNodeRef LoadBehaviorTreeXmlFile( const char* path, const char* name ) 
		{
			stack_string file;
			file.Format("%s%s.xml", path, name);
			XmlNodeRef behaviorTreeNode = GetISystem()->LoadXmlFromFile(file);

			IF_UNLIKELY(!behaviorTreeNode)
				gEnv->pLog->LogError("Failed to load behavior tree '%s'. Could not load the '%s' file.", name, file.c_str());

			return behaviorTreeNode;
		}

		INodePtr CreateBehaviorTreeRootNodeFromBehaviorTreeXml( const XmlNodeRef& behaviorTreeXmlNode, const LoadContext& context ) const
		{
			XmlNodeRef rootXmlNode = behaviorTreeXmlNode->findChild("Root");
			IF_UNLIKELY(!rootXmlNode)
			{
				gEnv->pLog->LogError("Failed to load behavior tree '%s'. The 'Root' node is missing.", context.treeName);
				return INodePtr();
			}
				
			return CreateBehaviorTreeNodeFromXml(rootXmlNode, context);
		}

		INodePtr CreateBehaviorTreeNodeFromXml( const XmlNodeRef& xmlNode, const LoadContext& context ) const
		{
			if (xmlNode->getChildCount() != 1)
			{
				gEnv->pLog->LogWarning("Failed to load behavior tree '%s'. The node '%s' must contain exactly one child node.", context.treeName, xmlNode->getTag());
				return INodePtr();
			}

			return context.nodeFactory.CreateNodeFromXml(xmlNode->getChild(0), context);
		}
		
	};
}
