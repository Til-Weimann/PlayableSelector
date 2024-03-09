// Widget displays info about connected player.
// Path: {B55DD7054C5892AE}UI/Lobby/PlayerSelector.layout
// Part of Lobby menu PS_CoopLobby ({9DECCA625D345B35}UI/Lobby/CoopLobby.layout)

class PS_PlayerSelector : SCR_ButtonBaseComponent
{
	protected int m_iPlayer;
	
	protected ResourceName m_sImageSet = "{D17288006833490F}UI/Textures/Icons/icons_wrapperUI-32.imageset";
	
	ImageWidget m_wPlayerFactionColor;
	TextWidget m_wPlayerName;
	TextWidget m_wPlayerFactionName;
	ImageWidget m_wReadyImage;
	ButtonWidget m_wKickButton;
	ButtonWidget m_wPinButton;
	ImageWidget m_wPinImage;
	
	PS_VoiceButton m_wVoiceHideableButton;
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wPlayerFactionColor = ImageWidget.Cast(w.FindAnyWidget("PlayerFactionColor"));
		m_wPlayerName = TextWidget.Cast(w.FindAnyWidget("PlayerName"));
		m_wPlayerFactionName = TextWidget.Cast(w.FindAnyWidget("PlayerFactionName"));
		m_wReadyImage = ImageWidget.Cast(w.FindAnyWidget("ReadyImage"));
		m_wKickButton = ButtonWidget.Cast(w.FindAnyWidget("KickButton"));
		m_wPinButton = ButtonWidget.Cast(w.FindAnyWidget("PinButton"));
		m_wPinImage = ImageWidget.Cast(w.FindAnyWidget("PinImage"));
		Widget voiceHideableButtonWidget = Widget.Cast(w.FindAnyWidget("VoiceHideableButton"));
		m_wVoiceHideableButton = PS_VoiceButton.Cast(voiceHideableButtonWidget.FindHandler(PS_VoiceButton));
		
		GetGame().GetCallqueue().CallLater(AddOnClick, 0);
	}
	
	void AddOnClick()
	{
		SCR_ButtonBaseComponent kickButtonHandler = SCR_ButtonBaseComponent.Cast(m_wKickButton.FindHandler(SCR_ButtonBaseComponent));
		kickButtonHandler.m_OnClicked.Insert(KickButtonClicked);
		SCR_ButtonBaseComponent pinButtonHandler = SCR_ButtonBaseComponent.Cast(m_wPinButton.FindHandler(SCR_ButtonBaseComponent));
		pinButtonHandler.m_OnClicked.Insert(PinButtonClicked);
	}
	
	void SetPlayer(int playerId)
	{
		m_iPlayer = playerId;
		m_wVoiceHideableButton.SetPlayer(playerId);
		UpdatePlayerInfo();
	}
	
	int GetPlayerId()
	{
		return m_iPlayer;
	}
	
	void UpdatePlayerInfo()
	{
		m_wVoiceHideableButton.Update();
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();
		FactionKey factionKey = playableManager.GetPlayerFactionKey(m_iPlayer);
		
		if (factionKey != "") 
		{
			FactionManager factionManager = GetGame().GetFactionManager();
			SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByKey(factionKey));
			m_wPlayerFactionColor.SetColor(faction.GetOutlineFactionColor());
			m_wPlayerFactionName.SetText(faction.GetFactionName());
			
			int factionIndex = GetGame().GetFactionManager().GetFactionIndex(faction);
			m_wRoot.SetZOrder(factionIndex);
		}else{
			m_wPlayerFactionColor.SetColor(Color.FromInt(0xFF2c2c2c));
			m_wPlayerFactionName.SetText("-");
			
			m_wRoot.SetZOrder(-1);
		}
		
		// if admin set player color
		m_wPlayerName.SetText(playerManager.GetPlayerName(m_iPlayer));
		EPlayerRole playerRole = playerManager.GetPlayerRoles(m_iPlayer);
		if (PS_PlayersHelper.IsAdminOrServer()) m_wPlayerName.SetColor(Color.FromInt(0xfff2a34b));
		else m_wPlayerName.SetColor(Color.FromInt(0xffffffff));
		
		// If admin show kick button for non admins
		PlayerController currentPlayerController = GetGame().GetPlayerController();
		EPlayerRole currentPlayerRole = playerManager.GetPlayerRoles(currentPlayerController.GetPlayerId());
		m_wKickButton.SetVisible(PS_PlayersHelper.IsAdminOrServer() && playerRole != EPlayerRole.ADMINISTRATOR);
		
		PS_EPlayableControllerState state = PS_PlayableManager.GetInstance().GetPlayerState(m_iPlayer);
		
		if (state == PS_EPlayableControllerState.NotReady) m_wReadyImage.LoadImageFromSet(0, m_sImageSet, "dotsMenu");
		if (state == PS_EPlayableControllerState.Ready) m_wReadyImage.LoadImageFromSet(0, m_sImageSet, "check");
		if (state == PS_EPlayableControllerState.Disconected) m_wReadyImage.LoadImageFromSet(0, m_sImageSet, "disconnection");
		if (state == PS_EPlayableControllerState.Playing) m_wReadyImage.LoadImageFromSet(0, m_sImageSet, "characters");
		
		// If pinned show pinImage or pinButton for admins
		if (playableManager.GetPlayerPin(m_iPlayer))
		{
			m_wPinImage.SetVisible(!PS_PlayersHelper.IsAdminOrServer());
			m_wPinButton.SetVisible(PS_PlayersHelper.IsAdminOrServer());
		} else 
		{
			m_wPinImage.SetVisible(false);
			m_wPinButton.SetVisible(false);
		}
	}
	
	
	// -------------------- Buttons events --------------------
	// Admin may kick player
	void KickButtonClicked(SCR_ButtonBaseComponent kickButton)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		PS_PlayableControllerComponent playableController = PS_PlayableControllerComponent.Cast(playerController.FindComponent(PS_PlayableControllerComponent));
		playableController.KickPlayer(m_iPlayer);
	}
	
	// Admin may unpin player
	void PinButtonClicked(SCR_ButtonBaseComponent pinButton)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		PS_PlayableControllerComponent playableController = PS_PlayableControllerComponent.Cast(playerController.FindComponent(PS_PlayableControllerComponent));
		playableController.UnpinPlayer(m_iPlayer);
	}
	
}