[BaseContainerProps()]
class PS_CanOpenLobbyInGameEditorAttribute : SCR_BaseEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) return null;
		
		PS_GameModeCoop coopMode = PS_GameModeCoop.Cast(item);
		if (!coopMode) return null;
		
		bool value = coopMode.GetCanOpenLobbyInGame();
		return SCR_BaseEditorAttributeVar.CreateBool(value);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		PS_GameModeCoop coopMode = PS_GameModeCoop.Cast(item);
		if (!coopMode) return;
		
		int value = var.GetBool();
		coopMode.SetCanOpenLobbyInGame(value);
	}
};
