//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "GameScripted/Character", description: "Set character playable", color: "0 0 255 255", icon: HYBRID_COMPONENT_ICON)]
class PS_PlayableComponentClass : ScriptComponentClass
{
}

[ComponentEditorProps(icon: HYBRID_COMPONENT_ICON)]
class PS_PlayableComponent : ScriptComponent
{
	[Attribute()]
	protected string m_name;
	[Attribute()]
	protected bool m_bIsPlayable;
	
	// Actually just RplId from RplComponent
	protected RplId m_id;
	
	// Cache components
	protected SCR_ChimeraCharacter m_Owner;
	SCR_ChimeraCharacter GetOwnerCharacter()
		return m_Owner;
	protected FactionAffiliationComponent m_FactionAffiliationComponent;
	FactionAffiliationComponent GetFactionAffiliationComponent()
		return m_FactionAffiliationComponent;
	protected SCR_EditableCharacterComponent m_EditableCharacterComponent;
	SCR_EditableCharacterComponent GetEditableCharacterComponent()
		return m_EditableCharacterComponent;
	protected SCR_CharacterDamageManagerComponent m_CharacterDamageManagerComponent;
	SCR_CharacterDamageManagerComponent GetCharacterDamageManagerComponent()
		return m_CharacterDamageManagerComponent;
	
	// Events
	protected ref ScriptInvokerInt m_eOnPlayerChange = new ScriptInvokerInt(); // int playerId
	ScriptInvokerInt GetOnPlayerChange()
		return m_eOnPlayerChange;
	void InvokeOnPlayerChanged(int playerId)
		m_eOnPlayerChange.Invoke(playerId);
	ScriptInvoker GetOnDamageStateChanged()
		return GetCharacterDamageManagerComponent().GetOnDamageStateChanged();
	protected ref ScriptInvokerVoid m_eOnUnregister = new ScriptInvokerVoid();
	ScriptInvokerVoid GetOnUnregister()
		return m_eOnUnregister;
	
	override void OnPostInit(IEntity owner)
	{
		m_Owner = SCR_ChimeraCharacter.Cast(owner);
		GetGame().GetCallqueue().CallLater(AddToList, 0, false, owner); // init delay

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		m_FactionAffiliationComponent = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));
		m_EditableCharacterComponent = SCR_EditableCharacterComponent.Cast(owner.FindComponent(SCR_EditableCharacterComponent));
		m_CharacterDamageManagerComponent = SCR_CharacterDamageManagerComponent.Cast(owner.FindComponent(SCR_CharacterDamageManagerComponent));
	}

	// Get/Set Broadcast
	bool GetPlayable()
	{
		return m_bIsPlayable;
	}
	void SetPlayable(bool isPlayable)
	{
		RPC_SetPlayable(isPlayable);
		Rpc(RPC_SetPlayable, isPlayable);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetPlayable(bool isPlayable)
	{
		m_bIsPlayable = isPlayable;
		if (m_bIsPlayable) GetGame().GetCallqueue().CallLater(AddToList, 0, false, m_Owner);
		else RemoveFromList();
	}

	void ResetRplStream()
	{
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		//rpl.EnableStreaming(true);
	}

	private void RemoveFromList()
	{
		GetGame().GetCallqueue().Remove(AddToList);
		GetGame().GetCallqueue().Remove(AddToListWrap);

		BaseGameMode gamemode = GetGame().GetGameMode();
		if (!gamemode)
			return;

		if (m_Owner)
		{
			AIControlComponent aiComponent = AIControlComponent.Cast(m_Owner.FindComponent(AIControlComponent));
			if (aiComponent)
			{
				AIAgent agent = aiComponent.GetAIAgent();
				if (agent) agent.ActivateAI();
			}

			RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
			rpl.EnableStreaming(true);
		}

		PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();
		playableManager.RemovePlayable(this);
		
		m_eOnUnregister.Invoke();
	}

	private void AddToList(IEntity owner)
	{
		AIControlComponent aiComponent = AIControlComponent.Cast(owner.FindComponent(AIControlComponent));
		if (aiComponent)
		{
			AIAgent agent = aiComponent.GetAIAgent();
			if (agent) agent.DeactivateAI();
		}

		GetGame().GetCallqueue().CallLater(AddToListWrap, 0, false, owner) // init delay
	}

	private void AddToListWrap(IEntity owner)
	{
		if (!m_bIsPlayable) return;

		AIControlComponent aiComponent = AIControlComponent.Cast(owner.FindComponent(AIControlComponent));
		if (aiComponent)
		{
			AIAgent agent = aiComponent.GetAIAgent();
			if (agent) agent.DeactivateAI();
		}

		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		rpl.EnableStreaming(false);

		PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();

		m_id = rpl.Id();
		playableManager.RegisterPlayable(this);
	}

	string GetName()
	{
		if (m_name != "") return m_name;
		SCR_EditableCharacterComponent editableCharacterComponent = SCR_EditableCharacterComponent.Cast(m_Owner.FindComponent(SCR_EditableCharacterComponent));
		SCR_UIInfo info = editableCharacterComponent.GetInfo();
		return info.GetName();
	}

	RplId GetId()
	{
		return m_id;
	}

	void PS_PlayableComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		
	}

	void ~PS_PlayableComponent()
	{
		RemoveFromList();
	}

	// Send our precision data, we need it on clients
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteString(m_name);
		writer.WriteBool(m_bIsPlayable);
		return true;
	}
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadString(m_name);
		reader.ReadBool(m_bIsPlayable);
		return true;
	}
}
