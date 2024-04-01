//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "", color: "0 0 255 255", icon: HYBRID_COMPONENT_ICON)]
class PS_MissionDataManagerClass: ScriptComponentClass
{
	
};

[ComponentEditorProps(icon: HYBRID_COMPONENT_ICON)]
class PS_MissionDataManager : ScriptComponent
{
	// more singletons for singletons god, make our spagetie kingdom great
	static PS_MissionDataManager GetInstance() 
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return PS_MissionDataManager.Cast(gameMode.FindComponent(PS_MissionDataManager));
		else
			return null;
	}
	
	ref map<EntityID, RplId> m_EntityToRpl = new map<EntityID, RplId>();
	ref map<RplId, SCR_DamageManagerComponent> m_RplToDamageManager = new map<RplId, SCR_DamageManagerComponent>();
	ref map<int, bool> m_playerSaved = new map<int, bool>();
	PS_PlayableManager m_PlayableManager;
	PS_GameModeCoop m_GameModeCoop;
	PlayerManager m_PlayerManager;
	ref PS_MissionDataConfig m_Data = new PS_MissionDataConfig();
	int m_iInitTimer = 20;
	
	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		
		GetGame().GetCallqueue().CallLater(LateInit, 0, false);
		GetGame().GetCallqueue().CallLater(AwaitFullInit, 0, true);
	}
	
	void RegisterVehicle(Vehicle vehicle)
	{
		RplComponent rplComponent = RplComponent.Cast(vehicle.FindComponent(RplComponent));
		SCR_EditableVehicleComponent editableVehicleComponent = SCR_EditableVehicleComponent.Cast(vehicle.FindComponent(SCR_EditableVehicleComponent));
		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(vehicle.FindComponent(FactionAffiliationComponent));
		SCR_DamageManagerComponent damageManagerComponent = SCR_DamageManagerComponent.Cast(vehicle.FindComponent(SCR_DamageManagerComponent));
		RplId vehicleId = rplComponent.Id();
		
		PS_MissionDataVehicle vehicleData = new PS_MissionDataVehicle();
		vehicleData.EntityId = vehicleId;
		vehicleData.PrefabPath = vehicle.GetPrefabData().GetPrefabName();
		if (editableVehicleComponent)
		{
			SCR_UIInfo info = editableVehicleComponent.GetInfo();
			if (info)
				vehicleData.EditableName = info.GetName();
		}
		if (factionAffiliationComponent)
		{
			Faction faction = factionAffiliationComponent.GetDefaultAffiliatedFaction();
			if (faction)
			{
				vehicleData.VehicleFactionKey = faction.GetFactionKey();
			}
		}
		if (damageManagerComponent)
		{
			m_RplToDamageManager.Insert(vehicleId, damageManagerComponent);
			damageManagerComponent.GetOnDamage().Insert(OnDamaged);
		}
		
		m_Data.Vehicles.Insert(vehicleData);
		m_EntityToRpl.Insert(vehicle.GetID(), vehicleId);
	}
	
	void OnDamaged(BaseDamageContext damageContext)
	{
		IEntity target = damageContext.hitEntity;
		Instigator instigator = damageContext.instigator;
		if (target && instigator)
		{
			int playerId = instigator.GetInstigatorPlayerID();
			if (playerId == -1)
				return;
			
			EntityID entityID = target.GetID();
			if (!m_EntityToRpl.Contains(entityID))
				return;
			RplId rplId = m_EntityToRpl.Get(entityID);
			
			GetGame().GetCallqueue().Call(SaveDamageEvent, playerId, rplId, damageContext.damageValue);
		}
	}
	
	void SaveDamageEvent(int playerId, RplId targetId, float value)
	{
		SCR_DamageManagerComponent damageManagerComponent = m_RplToDamageManager.Get(targetId);
		EDamageState state = damageManagerComponent.GetState();
		
		PS_MissionDataDamageEvent missionDataDamageEvent = new PS_MissionDataDamageEvent();
		missionDataDamageEvent.PlayerId = playerId;
		missionDataDamageEvent.TargetId = targetId;
		missionDataDamageEvent.DamageValue = value;
		missionDataDamageEvent.TargetState = state;
		missionDataDamageEvent.Time = GetGame().GetWorld().GetWorldTime();
		m_Data.DamageEvents.Insert(missionDataDamageEvent);
	}
	
	void LateInit()
	{
		m_GameModeCoop = PS_GameModeCoop.Cast(GetOwner());
		m_PlayerManager = GetGame().GetPlayerManager();
		m_PlayableManager = PS_PlayableManager.GetInstance();
		
		m_GameModeCoop.GetOnGameStateChange().Insert(OnGameStateChanged);
		m_GameModeCoop.GetOnPlayerAuditSuccess().Insert(OnPlayerAuditSuccess);
		if (RplSession.Mode() != RplMode.Dedicated) 
			OnPlayerAuditSuccess(GetGame().GetPlayerController().GetPlayerId());
	}
	
	void AwaitFullInit()
	{
		m_iInitTimer--; // Wait 20 frames, I belive everything can init in 20 frames. maybe...
		if (m_iInitTimer <= 0)
		{
			GetGame().GetCallqueue().Remove(AwaitFullInit);
			InitData();
		}
	}
	
	void OnGameStateChanged(SCR_EGameModeState state)
	{
		PS_MissionDataStateChangeEvent missionDataStateChangeEvent = new PS_MissionDataStateChangeEvent();
		missionDataStateChangeEvent.State = state;
		missionDataStateChangeEvent.Time = GetGame().GetWorld().GetWorldTime();
		m_Data.StateEvents.Insert(missionDataStateChangeEvent);
		if (state == SCR_EGameModeState.GAME)
			SavePlayers();
	}
	
	void OnPlayerAuditSuccess(int playerId)
	{
		if (m_playerSaved.Contains(playerId))
			return;
		
		string GUID = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);
		string name = m_PlayerManager.GetPlayerName(playerId);
		
		PS_MissionDataPlayer player = new PS_MissionDataPlayer();
		player.PlayerId = playerId;
		player.GUID = GUID;
		player.Name = name;
		m_Data.Players.Insert(player);
		
		m_playerSaved.Insert(playerId, true);
	}
	
	// Save main mission data
	void InitData()
	{
		MissionHeader missionHeader = GetGame().GetMissionHeader();
		if (missionHeader) m_Data.MissionPath = missionHeader.GetHeaderResourcePath();
		
		PS_MissionDescriptionManager missionDescriptionManager = PS_MissionDescriptionManager.GetInstance();
		array<PS_MissionDescription> descriptions = new array<PS_MissionDescription>();
		missionDescriptionManager.GetDescriptions(descriptions);
		foreach (PS_MissionDescription description : descriptions)
		{
			PS_MissionDataDescription descriptionData = new PS_MissionDataDescription();
			m_Data.Descriptions.Insert(descriptionData);
			
			descriptionData.Title = description.m_sTitle;
			descriptionData.DescriptionLayout = description.m_sDescriptionLayout;
			descriptionData.TextData = description.m_sTextData;
			descriptionData.VisibleForFactions = description.m_aVisibleForFactions;
			descriptionData.EmptyFactionVisibility = description.m_bEmptyFactionVisibility;
		}
		
		
		PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();
		
		array<PS_PlayableComponent> playables = playableManager.GetPlayablesSorted();
		
		PS_MissionDataGroup groupData = new PS_MissionDataGroup();
		PS_MissionDataFaction factionData = new PS_MissionDataFaction();
		
		map<Faction, PS_MissionDataFaction> factionsMap = new map<Faction, PS_MissionDataFaction>();
		map<SCR_AIGroup, PS_MissionDataGroup> groupsMap = new map<SCR_AIGroup, PS_MissionDataGroup>();
		foreach (PS_PlayableComponent playable : playables)
		{
			IEntity character = playable.GetOwner();
			AIControlComponent aiComponent = AIControlComponent.Cast(character.FindComponent(AIControlComponent));
			AIAgent agent = aiComponent.GetAIAgent();
			SCR_AIGroup group = SCR_AIGroup.Cast(agent.GetParentGroup());
			Faction faction = group.GetFaction();
			if (!factionsMap.Contains(faction))
			{
				factionData = new PS_MissionDataFaction();
				m_Data.Factions.Insert(factionData);
				
				factionData.Name = WidgetManager.Translate("%1", faction.GetFactionName());
				factionData.Key = WidgetManager.Translate("%1", faction.GetFactionKey());
				
				factionsMap.Insert(faction, factionData);
			}
			factionData = factionsMap.Get(faction);
			if (!groupsMap.Contains(group))
			{
				groupData = new PS_MissionDataGroup();
				factionData.Groups.Insert(groupData);
				
				string customName = group.GetCustomName();
				string company, platoon, squad, t, format;
				group.GetCallsigns(company, platoon, squad, t, format);
				string callsign;
				callsign = WidgetManager.Translate(format, company, platoon, squad, "");
				
				groupData.Callsign = playableManager.GetGroupCallsignByPlayable(playable.GetId());
				groupData.CallsignName = callsign;
				groupData.Name = customName;
				
				groupsMap.Insert(group, groupData);
			}
			groupData = groupsMap.Get(group);
			
			array<AIAgent> outAgents = new array<AIAgent>();
			group.GetAgents(outAgents);
			SCR_EditableCharacterComponent editableCharacterComponent = SCR_EditableCharacterComponent.Cast(character.FindComponent(SCR_EditableCharacterComponent));
			SCR_UIInfo uiInfo = editableCharacterComponent.GetInfo();
			
			PS_MissionDataPlayable missionDataPlayable = new PS_MissionDataPlayable();
			
			SCR_CharacterDamageManagerComponent damageManagerComponent = playable.GetCharacterDamageManagerComponent();
			
			damageManagerComponent.GetOnDamage().Insert(OnDamaged);
			m_RplToDamageManager.Insert(playable.GetId(), damageManagerComponent);
			missionDataPlayable.EntityId = playable.GetId();
			missionDataPlayable.GroupOrder = outAgents.Find(agent);
			missionDataPlayable.Name = WidgetManager.Translate("%1", playable.GetName());
			missionDataPlayable.RoleName = WidgetManager.Translate("%1", uiInfo.GetName());
			m_EntityToRpl.Insert(character.GetID(), playable.GetId());
			
			groupData.Playables.Insert(missionDataPlayable);
		}
	}
	
	void SavePlayers()
	{
		array<PS_PlayableComponent> playables = m_PlayableManager.GetPlayablesSorted();
		
		foreach (PS_PlayableComponent playable : playables)
		{
			IEntity character = playable.GetOwner();
			RplId playableId = playable.GetId();
			int playerId = m_PlayableManager.GetPlayerByPlayable(playableId);
			
			PS_MissionDataPlayerToEntity playerToEntity = new PS_MissionDataPlayerToEntity();
			playerToEntity.PlayerId = playerId;
			playerToEntity.EntityId = playableId;
			
			m_Data.PlayersToPlayables.Insert(playerToEntity);
		}
	}
	
	void WriteToFile()
	{
		SCR_JsonSaveContext missionSaveContext = new SCR_JsonSaveContext();
		missionSaveContext.WriteValue("", m_Data);
		missionSaveContext.SaveToFile("$profile:PS_MissionData.json");
	}
}