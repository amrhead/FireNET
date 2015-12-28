#ifndef IOFFMESHNAVIGATIONMANAGER_H__
#define IOFFMESHNAVIGATIONMANAGER_H__

#pragma once

#include "INavigationSystem.h"

namespace MNM
{
	struct OffMeshLink;

	enum EOffMeshOperationResult
	{
		eOffMeshOperationResult_SuccessToAddLink,
		eOffMeshOperationResult_FailedToAddLink,
		eOffMeshOperationResult_SuccessToRemoveLink,
		eOffMeshOperationResult_FailedToRemoveLink
	};

	typedef Functor1<MNM::OffMeshLinkID> OffMeshOperationCallback;

	enum EOffMeshOperationType
	{
		eOffMeshOperationType_Undefined,
		eOffMeshOperationType_Add,
		eOffMeshOperationType_Remove,
	};

	struct OffMeshOperationRequestBase
	{
		OffMeshOperationRequestBase() 
			: requestOwner( (EntityId)0 )
			, meshId( 0 )
			, linkId( MNM::Constants::eOffMeshLinks_InvalidOffMeshLinkID )
			, dataExists( false )
			, trimExcess( false )
			, callback( NULL )
			, operationType( eOffMeshOperationType_Undefined )
		{}

		OffMeshOperationRequestBase( const EOffMeshOperationType _operationType, const EntityId _requestOwnerId, const NavigationMeshID& _meshID, MNM::OffMeshLinkPtr _pLinkData, 
			const MNM::OffMeshLinkID& _linkID, const bool _dataExists = false, const bool _trimExcess = false )
			: requestOwner( _requestOwnerId )
			, meshId( _meshID )
			, linkId( _linkID )
			, dataExists( _dataExists )
			, trimExcess( _trimExcess )
			, operationType( _operationType )
		{
			assert( operationType == eOffMeshOperationType_Add );
			pLinkData = _pLinkData;
		}

		OffMeshOperationRequestBase( const EOffMeshOperationType _operationType, const EntityId _requestOwnerId, const MNM::OffMeshLinkID& _linkID )
			: requestOwner( _requestOwnerId )
			, meshId( 0 )
			, linkId( _linkID )
			, dataExists( false )
			, trimExcess( false )
			, callback( NULL )
			, operationType( _operationType )
		{
			assert( operationType == eOffMeshOperationType_Remove );
		}

		EntityId requestOwner;
		NavigationMeshID meshId;
		MNM::OffMeshLinkPtr pLinkData;
		MNM::OffMeshLinkID linkId;
		bool dataExists;
		bool trimExcess;
		OffMeshOperationCallback callback;
		EOffMeshOperationType operationType;
	};

	struct IsOffMeshAdditionOperationRequestRelatedToEntity
	{

		IsOffMeshAdditionOperationRequestRelatedToEntity(EntityId _requestOwnerId)
			: requestOwnerId(_requestOwnerId)
		{
		}

		bool operator()(OffMeshOperationRequestBase& other) { return ( other.operationType == eOffMeshOperationType_Add ) && ( other.requestOwner == requestOwnerId ); }

		EntityId requestOwnerId;
	};
	
	struct LinkAdditionRequest : public OffMeshOperationRequestBase
	{
		LinkAdditionRequest( const EntityId _requestOwnerId, const NavigationMeshID& _meshID, MNM::OffMeshLinkPtr _pLinkData, const MNM::OffMeshLinkID& _linkID, 
			const bool _dataExists = false, const bool _trimExcess = false )
			: OffMeshOperationRequestBase( eOffMeshOperationType_Add, _requestOwnerId, _meshID, _pLinkData, _linkID, _dataExists, _trimExcess )
		{
		}

		LinkAdditionRequest( const LinkAdditionRequest& other )
			: OffMeshOperationRequestBase ( other.operationType, other.requestOwner, other.meshId, other.pLinkData, other.linkId, other.dataExists, other.trimExcess )
		{
			callback = other.callback;
		}

		LinkAdditionRequest& operator=( const LinkAdditionRequest& other )
		{
			meshId = other.meshId;
			pLinkData = other.pLinkData;
			linkId = other.linkId;
			dataExists = other.dataExists;
			trimExcess = other.trimExcess;
			callback = other.callback;
			operationType = other.operationType;
			return *this;
		}

		void SetCallback( OffMeshOperationCallback _callback ) { callback = _callback; }
	};

	struct LinkRemovalRequest : OffMeshOperationRequestBase
	{
		LinkRemovalRequest( const EntityId _requestOwnerId, const MNM::OffMeshLinkID& _linkID )
			: OffMeshOperationRequestBase( eOffMeshOperationType_Remove, _requestOwnerId, _linkID )
		{
		}

		LinkRemovalRequest( const LinkRemovalRequest& other )
			: OffMeshOperationRequestBase( eOffMeshOperationType_Remove, other.requestOwner, other.linkId )
		{
		}
	};
}

struct IOffMeshNavigationListener
{
	virtual void OnOffMeshLinkGoingToBeRemoved(const MNM::OffMeshLinkID& linkID) = 0;
};

struct IOffMeshNavigationManager
{
	virtual ~IOffMeshNavigationManager() {}

	virtual void QueueCustomLinkAddition(const MNM::LinkAdditionRequest& request) = 0;
	virtual void QueueCustomLinkRemoval(const MNM::LinkRemovalRequest& request) = 0;
	virtual MNM::OffMeshLink* GetOffMeshLink(const MNM::OffMeshLinkID& linkID) = 0;
	virtual void RegisterListener(IOffMeshNavigationListener* pListener, const char* listenerName) = 0;
	virtual void UnregisterListener(IOffMeshNavigationListener* pListener) = 0;
	virtual void RemoveAllQueuedAdditionRequestForEntity(const EntityId requestOwner) = 0;
};

#endif // IOFFMESHNAVIGATIONMANAGER_H__