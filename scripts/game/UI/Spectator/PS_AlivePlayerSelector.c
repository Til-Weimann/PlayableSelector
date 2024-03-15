class PS_AlivePlayerSelector : SCR_ButtonBaseComponent
{
	// Const
	protected const ResourceName m_sImageSet = "{D17288006833490F}UI/Textures/Icons/icons_wrapperUI-32.imageset";
	
	// Cache global
	protected PS_PlayableManager m_PlayableManager;
	protected PlayerController m_PlayerController;
	protected SCR_FactionManager m_FactionManager;
	
	// Parameters
	protected PS_AlivePlayerGroup m_AliveGroup;
	protected PS_SpectatorMenu m_mSpectatorMenu;
	protected RplId m_iPlayableId;
	protected ResourceName m_sPlayableIcon;
	
	// Cache parameters
	protected PS_AlivePlayerList m_AlivePlayerList;
	protected PS_PlayableComponent m_PlayableComponent;
	protected SCR_CharacterDamageManagerComponent m_CharacterDamageManagerComponent;
	
	// Widgets
	protected ImageWidget m_wPlayerFactionColor;
	protected ImageWidget m_wUnitIcon;
	protected TextWidget m_wPlayerName;
	
	// Init
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Widgets
		m_wUnitIcon = ImageWidget.Cast(w.FindAnyWidget("UnitIcon"));
		m_wPlayerName = TextWidget.Cast(w.FindAnyWidget("PlayerName"));
		m_wPlayerFactionColor = ImageWidget.Cast(w.FindAnyWidget("PlayerFactionColor"));
		
		// Cache global
		m_PlayableManager = PS_PlayableManager.GetInstance();
		m_PlayerController = GetGame().GetPlayerController();
		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		GetGame().GetCallqueue().CallLater(AddOnClick, 0);
	}
	
	void AddOnClick()
	{
		m_OnClicked.Insert(AlivePlayerButtonClicked);
	}
	
	// Set parameters
	void SetPlayable(int playableId)
	{
		m_iPlayableId = playableId;
		
		// Cache parameters
		m_PlayableComponent = m_PlayableManager.GetPlayableById(playableId);
		m_CharacterDamageManagerComponent = m_PlayableComponent.GetCharacterDamageManagerComponent();
		
		// Temp
		FactionAffiliationComponent factionAffiliationComponent = m_PlayableComponent.GetFactionAffiliationComponent();
		Faction faction = factionAffiliationComponent.GetDefaultAffiliatedFaction();
		SCR_EditableCharacterComponent editableCharacterComponent = m_PlayableComponent.GetEditableCharacterComponent();
		SCR_UIInfo uiInfo = editableCharacterComponent.GetInfo();
		
		// Initial setup
		m_sPlayableIcon = uiInfo.GetIconPath();
		m_wPlayerFactionColor.SetColor(faction.GetFactionColor());
		EDamageState damageState = m_CharacterDamageManagerComponent.GetState();
		UpdateDammage(damageState);
		UpdatePlayer(m_PlayableManager.GetPlayerByPlayable(m_iPlayableId));
		
		// Events
		m_PlayableComponent.GetOnPlayerChange().Insert(UpdatePlayer);
		m_PlayableComponent.GetOnDamageStateChanged().Insert(UpdateDammage);
		m_PlayableComponent.GetOnUnregister().Insert(RemoveSelf);
	}
	
	void SetSpectatorMenu(PS_SpectatorMenu spectatorMenu)
	{
		m_mSpectatorMenu = spectatorMenu;
	}
	
	void SetAlivePlayerList(PS_AlivePlayerList alivePlayerList)
	{
		m_AlivePlayerList = alivePlayerList;
	}
	
	void SetAliveGroup(PS_AlivePlayerGroup alivePlayerGroup)
	{
		m_AliveGroup = alivePlayerGroup;
	}
	
	// Updates
	void UpdateDammage(EDamageState state)
	{
		if (state == EDamageState.DESTROYED)
		{
			m_wUnitIcon.LoadImageFromSet(0, m_sImageSet, "death");
			m_wPlayerName.SetColor(Color.Gray);
		}
		else
		{
			m_wUnitIcon.LoadImageTexture(0, m_sPlayableIcon);
			m_wPlayerName.SetColor(Color.White);
		}
	}
	
	void UpdatePlayer(int playerId)
	{
		string playerName = m_PlayableManager.GetPlayerName(playerId);
		if (playerName == "") // No player
			playerName = m_PlayableComponent.GetName();
		m_wPlayerName.SetText(playerName);
	}
	
	void RemoveSelf()
	{
		m_wRoot.RemoveFromHierarchy();
		m_AliveGroup.OnAliveRemoved(m_PlayableComponent);
		m_AlivePlayerList.OnAliveRemoved(m_PlayableComponent);
	}
	
	// -------------------- Buttons events --------------------
	void AlivePlayerButtonClicked(SCR_ButtonBaseComponent playerButton)
	{
		m_mSpectatorMenu.SetCameraCharacter(m_PlayableComponent.GetOwner());
	}
	
}