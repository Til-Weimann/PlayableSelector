modded enum ChimeraMenuPreset : ScriptMenuPresetEnum
{
	CoopLobby,
	PlayableSelector
}

//------------------------------------------------------------------------------------------------
class SCR_GameModeCoopClass: SCR_BaseGameModeClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_GameModeCoop : SCR_BaseGameMode
{
	IEntity CameraEntity;
	
	
	static ref map<int, PlayableControllerState> playersStates = new map<int, PlayableControllerState>; // Controllers server only. (╯°□°）╯︵ ┻━┻
	static ref map<int, string> playableGroups = new map<int, string>;
	static ref map<int, int> playersPlayable = new map<int, int>;
	
	void SetPlayerPlayableServer(int playerId, int playableId)
	{
		Print("SetPlayerPlayableServer: playerId " + playerId.ToString() + " playableId " + playableId.ToString());
		Rpc_SetPlayerPlayableClient(playerId, playableId);
		Rpc(Rpc_SetPlayerPlayableClient, playerId, playableId);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_SetPlayerPlayableClient(int playerId, int playableId)
	{
		Print("Rpc_SetPlayerPlayableClient: playerId " + playerId.ToString() + " playableId " + playableId.ToString());
		playersPlayable[playerId] = playableId;
	}
	int GetPlayerPlayable(int playerId)
	{
		Print("GetPlayerPlayable: playerId " + playerId.ToString() + " playableId " + playersPlayable[playerId].ToString());
		return playersPlayable[playerId];
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_TryReconnectServer(int playerId)
	{
		Print("Rpc_TryReconnectServer: playerId " + playerId.ToString());
		if (playersPlayable.Contains(playerId))
			Rpc(Rpc_TryReconnectClient, playerId, playersPlayable[playerId])
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Rpc_TryReconnectClient(int playerId, int playableId)
	{
		Print("Rpc_TryReconnectClient: playerId " + playerId.ToString() + " playableId " + playableId.ToString());
		PlayerController playerController = GetGame().GetPlayerController();
		if (playerController.GetPlayerId() == playerId) 
		{
			GetGame().GetCallqueue().CallLater(ReconnectForcePossess, 0, false, playableId);
		}
	}
	void ReconnectForcePossess(int playableId)
	{
		Print("ReconnectForcePossess: playableId " + playableId.ToString());
		PlayerController playerController = GetGame().GetPlayerController();
		SCR_PlayableControllerComponent playableController = SCR_PlayableControllerComponent.Cast(playerController.FindComponent(SCR_PlayableControllerComponent));
		playableController.TakePossession(playerController.GetPlayerId(), playableId);
		MenuManager menuManager = GetGame().GetMenuManager();
		SCR_CoopLobby lobby = SCR_CoopLobby.Cast(menuManager.FindMenuByPreset(ChimeraMenuPreset.CoopLobby));
		if (lobby) lobby.Close();
	}
	
	string GetPlayableGroupName(int playableId)
	{
		return playableGroups[playableId];
	}
	void SetPlayableGroupName(int playableId, string groupName)
	{
		Rpc_SetPlayableGroupNameClient(playableId, groupName);
		Rpc(Rpc_SetPlayableGroupNameClient, playableId, groupName);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Rpc_SetPlayableGroupNameClient(int playableId, string groupName)
	{
		playableGroups.Set(playableId, groupName);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_SyncPlayableGroupNameServer()
	{
		for (int i = 0; i < playableGroups.Count(); i++)
		{
			int playableId = playableGroups.GetKey(i);
			string groupName = playableGroups.GetElement(i);
			SetPlayableGroupName(playableId, groupName);
		}
	}
	
	void SetPlayerState(int playerId, PlayableControllerState state)
	{
		Rpc_SetPlayerStateClient(playerId, state);
		Rpc(Rpc_SetPlayerStateClient, playerId, state);
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_SyncPlayerStateServer(int SyncPlayerId)
	{
		GetGame().GetCallqueue().CallLater(SyncPlayerState, 0, false, SyncPlayerId);
	}
	
	void SyncPlayerState(int SyncPlayerId)
	{
		array<int> playerIds = new array<int>();
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);
		foreach (int playerId: playerIds)
		{
			SetPlayerState(playerId, GetPlayerState(playerId));
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Rpc_SetPlayerStateClient(int playerId, PlayableControllerState state)
	{
		playersStates.Set(playerId, state);
		SCR_GameModeCoop.Cast(GetGame().GetGameMode()).UpdateMenu();
	}
	
	static PlayableControllerState GetPlayerState(int playerId)
	{
		if (!playersStates.Contains(playerId)) return PlayableControllerState.NotReady;
		return playersStates[playerId];
	}
	
	protected override void OnPlayerConnected(int playerId)
	{
		GetGame().GetCallqueue().CallLater(SyncState, 0, false, playerId);
	}
	
	void SyncState(int playerId)
	{
		Rpc(Rpc_SyncPlayerStateServer, playerId);
		Rpc(Rpc_SyncPlayableGroupNameServer);
		Rpc(Rpc_TryReconnectServer, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	/*
	override void HandleOnCharacterDeath(notnull CharacterControllerComponent characterController, IEntity instigator)
	{
		super.HandleOnCharacterDeath(characterController, instigator);
		if (!instigator) return;
			
		UpdateMenu()
	}
	*/
	
	void UpdateMenu()
	{
		Rpc(RPC_UpdateMenu);
		RPC_UpdateMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameStart()
	{
		super.OnGameStart();
		GetGame().GetInputManager().AddActionListener("PlayableSelector", EActionTrigger.DOWN, OpenPlayableMenu);
		
		/*
		EntitySpawnParams params();
		vector mat[4];
		GetTransform(mat);
		params.Transform = mat;
		//if (CameraEntity == null)
		//	CameraEntity = GetGame().SpawnEntityPrefab(Resource.Load("{C8FDE42491F955CB}Prefabs/ManualCameraInitialPlayer.et"), GetGame().GetWorld(), params);
		*/
		
		if (Replication.IsClient()) {
			GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CoopLobby);
		}
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_UpdateMenu()
	{
		if (!Replication.IsClient()) return;
		GetGame().GetCallqueue().CallLater(UpdateMenuClient, 1);
	}
	
	void UpdateMenuClient()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		SCR_PlayableSelectorMenu menu = SCR_PlayableSelectorMenu.Cast(menuManager.FindMenuByPreset(ChimeraMenuPreset.PlayableSelector));
		if (menu) menu.UpdateList();
		SCR_CoopLobby lobby = SCR_CoopLobby.Cast(menuManager.FindMenuByPreset(ChimeraMenuPreset.CoopLobby));
		if (lobby) lobby.UpdateMenu();
	}
	
	void OpenPlayableMenu()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PlayableSelector);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
	}
	
	
	override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{	
		vector mat[4];
		player.GetTransform(mat);
		Rpc(RPC_ForceCamera, playerId, mat[0], mat[1], mat[2], mat[3]); 
		RPC_ForceCamera(playerId, mat[0], mat[1], mat[2], mat[3]) 
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_ForceCamera(int playerId, vector m0, vector m1, vector m2, vector m3) // Literally garbage
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (playerController.GetPlayerId() == playerId) 
		{
			EntitySpawnParams params();
			vector mat[4];
			mat[0] = m0;
			mat[1] = m1;
			mat[2] = m2;
			mat[3] = m3;
			params.Transform = mat;
			if (CameraEntity == null)
				CameraEntity = GetGame().SpawnEntityPrefab(Resource.Load("{C8FDE42491F955CB}Prefabs/ManualCameraInitialPlayer.et"), GetGame().GetWorld(), params);
		}
	}
	
	void RemoveCamera() 
	{
		if (CameraEntity) {
			RplComponent.DeleteRplEntity(CameraEntity, false);
			CameraEntity = null;
		}
	}
	
	protected override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		
	}
};

