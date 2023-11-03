// Widget displays info about alive players in game.
// Path: {18D3CF175C9AA974}UI/Spectator/AlivePlayersList.layout

class PS_AlivePlayerList : ScriptedWidgetComponent
{
	protected ResourceName m_sAlivePlayerSelectorPrefab = "{20DCB7288210C151}UI/Spectator/AlivePlayerSelector.layout";
	
	protected VerticalLayoutWidget m_wPlayersList;
	protected ref array<Widget> m_aPlayersListWidgets = {};
	protected ref array<PS_AlivePlayerSelector> m_aPlayersAliveList = {};
	
	override void HandlerAttached(Widget w)
	{
		m_wPlayersList = VerticalLayoutWidget.Cast(w.FindAnyWidget("AlivePlayersList"));
	}
	
	private int m_iOldPlayersCount = 0;
	
	// -------------------- Update content functions --------------------
	void HardUpdate()
	{
		array<int> alivePlayers = new array<int>();
		GetAlivePlayers(alivePlayers);
		
		if (m_iOldPlayersCount != alivePlayers.Count())
		{
			// Clear old widgets
			foreach (Widget widget: m_aPlayersListWidgets)
			{
				m_wPlayersList.RemoveChild(widget);
			}
			m_aPlayersAliveList.Clear();
			m_aPlayersListWidgets.Clear();
			
			PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();
			// Add new widgets
			foreach (int playerId: alivePlayers)
			{
				Widget alivePlayerWidget = GetGame().GetWorkspace().CreateWidgets(m_sAlivePlayerSelectorPrefab);
				PS_AlivePlayerSelector alivePlayerSelector = PS_AlivePlayerSelector.Cast(alivePlayerWidget.FindHandler(PS_AlivePlayerSelector));
				
				m_wPlayersList.AddChild(alivePlayerWidget);
				m_aPlayersListWidgets.Insert(alivePlayerWidget);
				m_aPlayersAliveList.Insert(alivePlayerSelector);
				
				alivePlayerSelector.SetPlayer(playerId);
			}
		}
		m_iOldPlayersCount = alivePlayers.Count();
		
		SoftUpdate();
	}
	
	void SoftUpdate()
	{
		foreach (PS_AlivePlayerSelector alivePlayerSelector: m_aPlayersAliveList)
		{
			alivePlayerSelector.UpdateInfo();
		}
	}
	
	void GetAlivePlayers(out array<int> outPlayersArray)
	{
		PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();
		array<PS_PlayableComponent> playablesSorted = playableManager.GetPlayablesSorted();
		for (int i = 0; i < playablesSorted.Count(); i++) {
			PS_PlayableComponent playable = playablesSorted[i];
			SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(playable.GetOwner());
			if (!character.GetDamageManager().IsDestroyed()) 
			{
				int playerId = playableManager.GetPlayerByPlayable(playable.GetId());
				if (playerId > 0 && !outPlayersArray.Contains(playerId))
					outPlayersArray.Insert(playerId);
			}
		}
	}
}