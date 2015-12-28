#pragma once

#if !defined(_RELEASE) && (defined(WIN32) || defined(WIN64))
# define DEBUG_MODULAR_BEHAVIOR_TREE
# define USING_BEHAVIOR_TREE_EDITOR
#endif

#ifdef USING_BEHAVIOR_TREE_EDITOR
# define USING_BEHAVIOR_TREE_SERIALIZATION
# define USING_BEHAVIOR_TREE_XML_DESCRIPTION_CREATION
#endif

#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
#  define USING_BEHAVIOR_TREE_DEBUG_EVENTS
#  define USING_BEHAVIOR_TREE_LOG
#  define USING_BEHAVIOR_TREE_VISUALIZER
#  define USING_BEHAVIOR_TREE_NODE_CUSTOM_DEBUG_TEXT
#  define USING_BEHAVIOR_TREE_DEBUG_TIMESTAMPS
#  define STORE_BLACKBOARD_VARIABLE_NAMES
//#  define USING_BEHAVIOR_TREE_DEBUG_MEMORY_USAGE
#endif

#include <VectorMap.h>
#include <VariableCollection.h>
#include <BehaviorTree/TimestampCollection.h>
#include <Serialization/TypeID.h>

#ifdef USING_BEHAVIOR_TREE_SERIALIZATION

#include <Serialization/BoostSharedPtr.h>
#include <Serialization/IClassFactory.h>

namespace Serialization 
{ 
	class IArchive;
	template<class T> class ClassFactory; 
}

namespace BehaviorTree 
{ 
	struct INode;
	typedef Serialization::ClassFactory<INode> NodeSerializationClassFactory; 
}

#endif

struct IAISignalExtraData;

namespace BehaviorTree
{
	struct INodeFactory;
	class EventDispatcher;

	enum Status
	{
		Invalid,
		Success,
		Failure,
		Running
	};

	enum LoadResult
	{
		LoadFailure,
		LoadSuccess,
	};

#if defined( USING_BEHAVIOR_TREE_DEBUG_EVENTS ) || defined( USING_BEHAVIOR_TREE_SERIALIZATION )
#define STORE_EVENT_NAME
#endif

	class Event
	{
	public:
		Event()
			: m_nameCRC32(0)
			, m_pData(0)
#ifdef STORE_EVENT_NAME
			, m_name()
#endif
		{
		}

		Event(uint32 nameCRC32, const void* pData = 0)
			: m_nameCRC32(nameCRC32)
			, m_pData(pData)
#ifdef STORE_EVENT_NAME
			, m_name()
#endif
		{
		}

#ifdef STORE_EVENT_NAME
		Event(uint32 nameCRC32, const char* name, const void* pData = 0)
			: m_nameCRC32(nameCRC32)
			, m_name(name)
			, m_pData(pData)
		{
		}
#endif

		Event(const char* name, const void* pData = 0)
			: m_pData(pData)			
#ifdef STORE_EVENT_NAME
			, m_name(name)
#endif
		{
			m_nameCRC32 = gEnv->pSystem->GetCrc32Gen()->GetCRC32(name);
		}

		bool operator == (const Event& rhs) const
		{
			return m_nameCRC32 == rhs.m_nameCRC32;
		}

		bool operator < (const Event& rhs) const
		{
			return m_nameCRC32 < rhs.m_nameCRC32;
		}

#ifdef STORE_EVENT_NAME
		const char* GetName() const
		{
			return m_name.c_str();
		}
#endif

		uint32 GetCRC() const
		{
			return m_nameCRC32;
		}

		const void* GetData() const
		{
			return m_pData;
		}

#ifdef USING_BEHAVIOR_TREE_SERIALIZATION
		virtual void Serialize( Serialization::IArchive& archive )
		{
			archive( m_name, "name", "^Name" );
		}
#endif

	private:
		uint32 m_nameCRC32;
		const void* m_pData;

#ifdef STORE_EVENT_NAME
		string m_name;
#endif
	};

	struct LoadContext
	{
		LoadContext(
			INodeFactory& _nodeFactory,
			const char* _treeName,
			const Variables::Declarations& _variableDeclarations
			)
			: nodeFactory(_nodeFactory)
			, treeName(_treeName)
			, variableDeclarations(_variableDeclarations)
		{
		}

		INodeFactory& nodeFactory;
		const char* treeName;
		const Variables::Declarations& variableDeclarations;

		// Warning! If you're thinking about adding the entity id to the
		// LoadContext you need to keep one thing in mind:
		// The modular behavior tree will be loaded up and constructed before
		// the connection between the entity and the ai object has been made.
	};

	struct INode;
	struct DebugNode;

	typedef boost::shared_ptr<DebugNode> DebugNodePtr;

	struct DebugNode
	{
		typedef DynArray<DebugNodePtr> Children;

		const INode* node;
		Children children;

		DebugNode(const INode* _node) { node = _node; }
	};

#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
	class DebugTree
	{
	public:
		void Push(const INode* node)
		{
			FUNCTION_PROFILER(gEnv->pSystem, PROFILE_AI);
			DebugNodePtr debugNode(new DebugNode(node));

			if (!m_firstDebugNode)
				m_firstDebugNode = debugNode;

			if (!m_debugNodeStack.empty())
				m_debugNodeStack.back()->children.push_back(debugNode);

			m_debugNodeStack.push_back(debugNode);
		}

		void Pop(Status s)
		{
			FUNCTION_PROFILER(gEnv->pSystem, PROFILE_AI);
			IF_UNLIKELY (s == Failure || s == Success)
			{
				m_succeededAndFailedNodes.push_back(m_debugNodeStack.back());
			}

			m_debugNodeStack.pop_back();

			if (s != Running)
			{
				if (!m_debugNodeStack.empty())
					m_debugNodeStack.back()->children.pop_back();
				else
					m_firstDebugNode.reset();
			}
		}

		DebugNodePtr GetFirstNode() const
		{
			return m_firstDebugNode;
		}

		const DynArray<DebugNodePtr>& GetSucceededAndFailedNodes() const
		{
			return m_succeededAndFailedNodes;
		}

	private:
		DebugNodePtr m_firstDebugNode;
		DynArray<DebugNodePtr> m_debugNodeStack;
		DynArray<DebugNodePtr> m_succeededAndFailedNodes;
	};
#endif // DEBUG_MODULAR_BEHAVIOR_TREE

	typedef uint32 NodeID;

	struct BehaviorVariablesContext
	{
		BehaviorVariablesContext( Variables::Collection& _collection, const Variables::Declarations& _declarations, bool _variablesWereChangedBeforeCurrentTick )
			: collection( _collection )
			, declarations( _declarations )
			, variablesWereChangedBeforeCurrentTick( _variablesWereChangedBeforeCurrentTick )
		{
		}

		bool VariablesChangedSinceLastTick() const
		{
			return variablesWereChangedBeforeCurrentTick || collection.Changed();
		}

		Variables::Collection& collection;
		const Variables::Declarations& declarations;
		const bool variablesWereChangedBeforeCurrentTick;
	};

#ifdef USING_BEHAVIOR_TREE_LOG
	class MessageQueue
	{
	public:
		typedef std::deque<string> Messages;

		void AddMessage( const char* message )
		{
			if( m_messages.size() + 1 > 20 )
				m_messages.pop_front();

			m_messages.push_back( message );
		}

		const Messages& GetMessages() const
		{
			return m_messages;
		}

		void Clear()
		{
			m_messages.clear();
		}

	private:
		Messages m_messages;
	};
#endif // USING_BEHAVIOR_TREE_LOG

	//////////////////////////////////////////////////////////////////////////

	struct IBlackboardVariable
	{
	public:
		virtual ~IBlackboardVariable() {}

		template <typename Type>
		bool SetData( const Type& data )
		{
			if( Serialization::TypeID::get<Type>() != GetDataTypeId() )
				return false;

			*static_cast<Type*>( GetDataVoidPointer() ) = data;
			return true;
		}

		template <typename Type>
		bool GetData( Type& data ) const
		{
			if( Serialization::TypeID::get<Type>() != GetDataTypeId() )
				return false;

			data = *static_cast<const Type*>( GetDataVoidPointer() );
			return true;
		}

		virtual Serialization::TypeID GetDataTypeId() const	= 0;

	private:
		virtual void* GetDataVoidPointer() = 0;
		virtual const void* GetDataVoidPointer() const = 0;
	};

	DECLARE_BOOST_POINTERS( IBlackboardVariable );

	template < class Type = void >
	class BlackboardVariable : public IBlackboardVariable
	{
	public:
		BlackboardVariable( Type value )
		{
			m_value = value;
			m_typeId = Serialization::TypeID::get<Type>();
		}

		virtual Serialization::TypeID GetDataTypeId() const	OVERRIDE { return m_typeId; }

	private:
		BlackboardVariable() {};

		virtual void* GetDataVoidPointer() OVERRIDE { return &m_value; }
		virtual const void* GetDataVoidPointer() const OVERRIDE { return &m_value; }

		Serialization::TypeID m_typeId;
		Type m_value;
	};

	struct BlackboardVariableId
	{
		BlackboardVariableId()
			: id( 0 ) 
		{};

#ifdef STORE_BLACKBOARD_VARIABLE_NAMES
		BlackboardVariableId( uint32 _id, const char* _name )
			: id( _id )
			, name( _name ) 
		{};
#else
		BlackboardVariableId( uint32 id, const char* name )
			: id( id ) 
		{};
#endif

		operator uint32() const	{	return id; };

		uint32 id;
#ifdef STORE_BLACKBOARD_VARIABLE_NAMES
		string name;
#endif
	};

	class Blackboard
	{
	public:
		static inline BlackboardVariableId GetBlackboardVariableId( const char* name )
		{
			return BlackboardVariableId( gEnv->pSystem->GetCrc32Gen()->GetCRC32( name ), name );
		}

		template <typename Type>
		bool SetVariable( BlackboardVariableId id, const Type& value )
		{
			BlackboardVariableMap::iterator blackboardVariableIt = m_blackboardVariableMap.find( id );

			if( blackboardVariableIt != m_blackboardVariableMap.end() )
			{
				return blackboardVariableIt->second->SetData( value );
			}

			IBlackboardVariablePtr blackboardVariablePrt;
			blackboardVariablePrt.reset( new BlackboardVariable<Type>( value ) );
			m_blackboardVariableMap.insert( std::make_pair( id, blackboardVariablePrt ) );
			return true;
		}

		template <typename Type>
		bool GetVariable( BlackboardVariableId id, Type& variable ) const
		{
			BlackboardVariableMap::const_iterator blackboardVariableIt = m_blackboardVariableMap.find( id );

			if( blackboardVariableIt != m_blackboardVariableMap.end() )
			{
				return blackboardVariableIt->second->GetData( variable );
			}

			return false;
		}

		typedef std::map<BlackboardVariableId, IBlackboardVariablePtr> BlackboardVariableMap;

		const BlackboardVariableMap& GetBlackboardVariableMap() const { return m_blackboardVariableMap; }		

	private:

		BlackboardVariableMap m_blackboardVariableMap; 
	};

	//////////////////////////////////////////////////////////////////////////

	struct UpdateContext
	{
		UpdateContext(
			const EntityId _id
			, IEntity& _entity
			, BehaviorVariablesContext& _variables
			, TimestampCollection& _timestamps
			, Blackboard& _blackboard
#ifdef USING_BEHAVIOR_TREE_LOG
			, MessageQueue& _behaviorLog
#endif // USING_BEHAVIOR_TREE_LOG
#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
			, DebugTree* _debugTree = NULL
#endif // DEBUG_MODULAR_BEHAVIOR_TREE

		)
			: entityId(_id)
			, entity(_entity)
			, runtimeData(NULL)
			, variables(_variables)
			, timestamps(_timestamps)
			, blackboard(_blackboard)
#ifdef USING_BEHAVIOR_TREE_LOG
			, behaviorLog(_behaviorLog)
#endif // USING_BEHAVIOR_TREE_LOG
#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
			, debugTree(_debugTree)
#endif // DEBUG_MODULAR_BEHAVIOR_TREE
		{
		}

		EntityId entityId;
		IEntity& entity;
		void* runtimeData;
		BehaviorVariablesContext& variables;
		TimestampCollection& timestamps;
		Blackboard& blackboard;

#ifdef USING_BEHAVIOR_TREE_LOG
		MessageQueue& behaviorLog;
#endif // USING_BEHAVIOR_TREE_LOG

#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
		DebugTree* debugTree;
#endif // DEBUG_MODULAR_BEHAVIOR_TREE

	};

	struct EventContext
	{
		EventContext( const EntityId _id )
			: entityId( _id )
			, runtimeData( NULL )
		{
		}

		EntityId entityId;
		void* runtimeData;
	};

	struct INode
	{
		virtual ~INode() {}

		// When you "tick" a node the following rules apply:
		// 1. If the node was not previously running the node will first be
		//    initialized and then immediately updated.
		// 2. If the node was previously running, but no longer is, it will be
		//    terminated.
		virtual Status Tick(const UpdateContext& context) = 0;

		// Call this to explicitly terminate a node. The node will itself take
		// care of cleaning things up.
		// It's safe to call Terminate on an already terminated node,
		// although it's of course redundant.
		virtual void Terminate(const UpdateContext& context) = 0;

		// Load up a behavior tree node with information from an xml node.
		virtual LoadResult LoadFromXml(const XmlNodeRef& xml, const struct LoadContext& context) = 0;

#ifdef USING_BEHAVIOR_TREE_XML_DESCRIPTION_CREATION
		virtual XmlNodeRef CreateXmlDescription() = 0;
#endif

		// Send an event to the node.
		// The event will be dispatched to the correct HandleEvent method
		// with validated runtime data.
		// Never override!
		virtual void SendEvent(const EventContext& context, const Event& event) = 0;

#ifdef USING_BEHAVIOR_TREE_SERIALIZATION
		virtual void Serialize( Serialization::IArchive& archive ) = 0;
#endif
	};

	DECLARE_BOOST_POINTERS(INode);

	// This is the recipe for a behavior tree.
	// The information in this template should be considered to be
	// immutable (read-only). You can create a behavior tree instance
	// from a template.
	struct BehaviorTreeTemplate
	{
		BehaviorTreeTemplate() {}

		BehaviorTreeTemplate(
			INodePtr& _rootNode,
			TimestampCollection& _timestampCollection,
			Variables::Declarations& _variableDeclarations,
			Variables::SignalHandler& _signals )
			: rootNode( _rootNode )
			, defaultTimestampCollection( _timestampCollection )
			, variableDeclarations( _variableDeclarations )
			, signalHandler( _signals )
		{
		}

		INodePtr rootNode;
		TimestampCollection defaultTimestampCollection;
		Variables::Declarations variableDeclarations;
		Variables::SignalHandler signalHandler;

#if defined( DEBUG_MODULAR_BEHAVIOR_TREE )
		CryFixedStringT<64> mbtFilename;
#endif
	};

	DECLARE_BOOST_POINTERS(BehaviorTreeTemplate);

	// This contains all the data for a behavior tree that can be modified
	// during runtime. The behavior tree instance is created from a
	// behavior tree template.
	struct BehaviorTreeInstance
	{
		BehaviorTreeInstance() {}

		BehaviorTreeInstance(
			const TimestampCollection& _timestampCollection,
			const Variables::Collection& _variables,
			BehaviorTreeTemplatePtr _behaviorTreeTemplate,
			INodeFactory& nodeFactory )
			: timestampCollection( _timestampCollection )
			, variables( _variables )
			, behaviorTreeTemplate( _behaviorTreeTemplate )
		{
		}

		TimestampCollection timestampCollection;
		Variables::Collection variables;
		const BehaviorTreeTemplatePtr behaviorTreeTemplate;
		Blackboard blackboard;

#ifdef USING_BEHAVIOR_TREE_LOG
		MessageQueue behaviorLog;
#endif // USING_BEHAVIOR_TREE_LOG
	};

	DECLARE_BOOST_POINTERS(BehaviorTreeInstance);

	struct IBehaviorTreeManager
	{
		virtual ~IBehaviorTreeManager() {}
		virtual struct INodeFactory& GetNodeFactory() = 0;
#ifdef USING_BEHAVIOR_TREE_SERIALIZATION
		virtual NodeSerializationClassFactory& GetNodeSerializationFactory() = 0;
#endif
		virtual void LoadFromDiskIntoCache( const char* behaviorTreeName ) = 0;
		virtual bool StartModularBehaviorTree( const EntityId entityId, const char* treeName ) = 0;
		virtual bool StartModularBehaviorTreeFromXml( const EntityId entityId, const char* treeName, XmlNodeRef treeXml ) = 0;
		virtual void StopModularBehaviorTree( const EntityId entityId ) = 0;

		virtual void HandleEvent( const EntityId entityId, Event& event ) = 0;

		// Todo: Remove these functions as the behavior variables are a internal concept of the behavior tree and should not
		// be directly accessed from the outside.
		// Instead of changing directly the internal state of the variable, the external systems should inform the tree of a 
		// particular event and let the behavior tree decide if and what variables should be changed.
		virtual Variables::Collection* GetBehaviorVariableCollection_Deprecated( const EntityId entityId ) const = 0;
		virtual const Variables::Declarations* GetBehaviorVariableDeclarations_Deprecated( const EntityId entityId ) const = 0;
	};

	typedef uint64 RuntimeDataID;

	inline RuntimeDataID MakeRuntimeDataID(const EntityId entityID, const NodeID nodeID)
	{
		STATIC_ASSERT(sizeof(entityID) == 4, "Expected entity id to be 4 bytes");
		STATIC_ASSERT(sizeof(nodeID) == 4, "Expected node id to be 4 bytes");
		STATIC_ASSERT(sizeof(RuntimeDataID) == 8, "Expected runtime data id to be 8 bytes");

		const RuntimeDataID runtimeDataID = (uint64)entityID | (((uint64)nodeID) << 32);
		return runtimeDataID;
	}

	struct INodeCreator
	{
		virtual ~INodeCreator() {}
		virtual INodePtr Create() = 0;
		virtual void Trim() = 0;
		virtual const char* GetTypeName() const = 0;
		virtual size_t GetNodeClassSize() const = 0;
		virtual size_t GetSizeOfImmutableDataForAllAllocatedNodes() const = 0;
		virtual size_t GetSizeOfRuntimeDataForAllAllocatedNodes() const = 0;
		virtual void* AllocateRuntimeData(const RuntimeDataID runtimeDataID) = 0;
		virtual void* GetRuntimeData(const RuntimeDataID runtimeDataID) const = 0;
		virtual void FreeRuntimeData(const RuntimeDataID runtimeDataID) = 0;
		virtual void SetNodeFactory(INodeFactory* nodeFactory) = 0;
	};

	struct INodeFactory
	{
		virtual ~INodeFactory() {}
		virtual INodePtr CreateNodeOfType(const char* typeName) = 0;
		virtual INodePtr CreateNodeFromXml(const XmlNodeRef& xml, const LoadContext& context) = 0;
		virtual void RegisterNodeCreator(struct INodeCreator* nodeCreator) = 0;
		virtual size_t GetSizeOfImmutableDataForAllAllocatedNodes() const = 0;
		virtual size_t GetSizeOfRuntimeDataForAllAllocatedNodes() const = 0;
		virtual void* AllocateRuntimeDataMemory(const size_t size) = 0;
		virtual void FreeRuntimeDataMemory(void* pointer) = 0;
		virtual void* AllocateNodeMemory(const size_t size) = 0;
		virtual void FreeNodeMemory(void* pointer) = 0;
	};

	template <typename NodeType, typename NodeCreatorType>
	class NodeDeleter
	{
	public:
		NodeDeleter(INodeFactory& nodeFactory, NodeCreatorType& creator)
			: m_nodeFactory(nodeFactory), m_creator(creator)
		{
		}

		void operator () (NodeType* node)
		{
			node->~NodeType();
			m_nodeFactory.FreeNodeMemory(node);
			--m_creator.m_nodeCount;
		}

	private:
		INodeFactory& m_nodeFactory;
		NodeCreatorType& m_creator;
	};

	template <typename NodeType> class NodeCreator : public INodeCreator
	{
		typedef NodeCreator<NodeType> ThisNodeCreatorType;
		typedef typename NodeType::RuntimeData RuntimeDataType;
		typedef VectorMap<RuntimeDataID, void*> RuntimeDataCollection;

		friend class NodeDeleter<NodeType, ThisNodeCreatorType>;

	public:
		NodeCreator(const char* typeName)
			: m_typeName(typeName)
			, m_nodeCount(0)
		{
		}

		virtual INodePtr Create() OVERRIDE
		{
			MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Other, 0, "Modular Behavior Tree Node Factory: %s", m_typeName);

			assert(m_nodeFactory != NULL);

			void* const pointer = m_nodeFactory->AllocateNodeMemory(sizeof(NodeType));
			
			if (pointer == NULL)
			{
				CryFatalError("Could not allocate the memory needed for a '%s' node.", GetTypeName());
			}

			++m_nodeCount;

			return INodePtr(new (pointer) NodeType(), NodeDeleter<NodeType, ThisNodeCreatorType>(*m_nodeFactory, *this));
		}

		virtual void Trim()
		{
			assert(m_runtimeDataCollection.empty());
			if (m_runtimeDataCollection.empty())
				stl::free_container(m_runtimeDataCollection);
			else
				gEnv->pLog->LogError("Failed to free runtime data collection. There were still %" PRISIZE_T " runtime data elements in '%s' node creator.", m_runtimeDataCollection.size(), m_typeName);
		}

		virtual const char* GetTypeName() const
		{
			return m_typeName;
		}

		virtual size_t GetNodeClassSize() const
		{
			return sizeof(NodeType);
		}

		virtual size_t GetSizeOfImmutableDataForAllAllocatedNodes() const OVERRIDE
		{
			return m_nodeCount * (sizeof(NodeType) + sizeof(INodePtr));
		}

		virtual size_t GetSizeOfRuntimeDataForAllAllocatedNodes() const OVERRIDE
		{
			return m_runtimeDataCollection.size() * sizeof(RuntimeDataType);
		}

		virtual void* AllocateRuntimeData(const RuntimeDataID runtimeDataID) OVERRIDE
		{
			FUNCTION_PROFILER(gEnv->pSystem, PROFILE_AI);
			void* pointer = m_nodeFactory->AllocateRuntimeDataMemory(sizeof(RuntimeDataType));
			RuntimeDataType* runtimeData = new (pointer) RuntimeDataType;
			assert(runtimeData != NULL);
			m_runtimeDataCollection.insert(std::make_pair(runtimeDataID, reinterpret_cast<void*>(runtimeData)));
			return runtimeData;
		}

		virtual void* GetRuntimeData(const RuntimeDataID runtimeDataID) const OVERRIDE
		{
			FUNCTION_PROFILER(gEnv->pSystem, PROFILE_AI);
			typename RuntimeDataCollection::const_iterator it = m_runtimeDataCollection.find(runtimeDataID);
			if (it != m_runtimeDataCollection.end())
			{
				return it->second;
			}

			return NULL;
		}

		virtual void FreeRuntimeData(const RuntimeDataID runtimeDataID) OVERRIDE
		{
			FUNCTION_PROFILER(gEnv->pSystem, PROFILE_AI);
			typename RuntimeDataCollection::iterator it = m_runtimeDataCollection.find(runtimeDataID);
			assert(it != m_runtimeDataCollection.end());
			if (it != m_runtimeDataCollection.end())
			{
				RuntimeDataType* runtimeData = reinterpret_cast<RuntimeDataType*>(it->second);
				runtimeData->~RuntimeDataType();
				m_nodeFactory->FreeRuntimeDataMemory(runtimeData);
				m_runtimeDataCollection.erase(it);
			}
		}

		virtual void SetNodeFactory(INodeFactory* nodeFactory) OVERRIDE
		{
			m_nodeFactory = nodeFactory;
		}

	private:
		const char* m_typeName;
		INodeFactory* m_nodeFactory;
		RuntimeDataCollection m_runtimeDataCollection;
		size_t m_nodeCount;
	};

	// Register your node creator with the passed in manager. You need to
	// have made your creator available via GenerateBehaviorTreeNodeCreator.
	#define REGISTER_BEHAVIOR_TREE_NODE(manager, nodetype) \
	{ \
	 	static NodeCreator<nodetype> nodetype##NodeCreator(#nodetype); \
		manager.GetNodeFactory().RegisterNodeCreator(&nodetype##NodeCreator); \
	}

#ifdef USING_BEHAVIOR_TREE_SERIALIZATION
	#define REGISTER_BEHAVIOR_TREE_NODE_WITH_SERIALIZATION( manager, nodetype, label) \
		{ \
			REGISTER_BEHAVIOR_TREE_NODE( manager, nodetype ); \
			SERIALIZATION_CLASS_NAME_FOR_FACTORY(manager.GetNodeSerializationFactory(), INode, nodetype, #nodetype, label); \
		}
#else
	#define REGISTER_BEHAVIOR_TREE_NODE_WITH_SERIALIZATION( manager, nodetype, label) \
		{ \
			REGISTER_BEHAVIOR_TREE_NODE( manager, nodetype ); \
		}
#endif

}

#ifdef USING_BEHAVIOR_TREE_SERIALIZATION

namespace BehaviorTree
{
	struct NodePointerSerializer : BoostSharedPtrSerializer<INode>
	{
		NodePointerSerializer( boost::shared_ptr<INode>& ptr )
			: BoostSharedPtrSerializer( ptr )
		{
		}

		Serialization::ClassFactory<INode>& factoryOverride() const OVERRIDE
		{
			return gEnv->pAISystem->GetIBehaviorTreeManager()->GetNodeSerializationFactory();
		}
	};
}

namespace boost
{
	inline bool Serialize( Serialization::IArchive& ar, BehaviorTree::INodePtr& ptr, const char* name, const char* label )
	{
		BehaviorTree::NodePointerSerializer serializer( ptr );
		return ar( static_cast<Serialization::IPointer&>( serializer ), name, label );
	}
}

#endif
