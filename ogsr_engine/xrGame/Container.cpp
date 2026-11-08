// #+# Класс контейнеров

#include "stdafx.h"
#include "Container.h"
#include "physic_item.h"
#include "Level.h"
#include "entity_alive.h"
#include "EntityCondition.h"
#include "InventoryOwner.h"
#include "Artifact.h"
#include "PhysicsShell.h"
#include "ui/UIXmlInit.h"
#include "ui/UIDialogWnd.h"
#include "ui/UIScrollBar.h"
#include "ui/UIScrollView.h"

#include "Actor.h"
#include "ActorCondition.h"
#include "string_table.h"

#define ITEM_BIG_INFO "ui_sc_test_wnd.xml"


LPCSTR _imm_infection_name[] = {"radiation_infection", "biological_infection", "psi_infection", "temperature_infection", "cold_infection"};

LPCSTR _imm_char_names[] = {
    "char_jivuchest",           "char_temperature",   "char_gorenie",           "char_blast_defence", "char_power",          "char_weight",         "char_bio_v",
    "char_psi_defence",         "char_react_on_elec", "char_health_v",          "char_power_v",       "char_bullet_defence", "char_wound_v",        "char_wound_defence",
    "char_radiation_v",         "char_run_speed",     "char_move_speed",        "char_fire_defence",  "char_recoil",         "char_mutant_defence", "char_react_on_mutant",
    "char_electro_defence",     "char_heal_power",    "char_bio_defence",       "char_react_on_chem", "char_stoykost",       "char_perioud_heal",   "char_react_on_fire",
    "char_temperature_defence", "char_psi_defence",   "char_radiation_defence", "char_pokachivanie"};



CUIItemContainerInfo::CUIItemContainerInfo()
{
    m_need_reinit = false;
    m_need_update_scroll = true;
    
    Init();

    Hide();
};

CUIItemContainerInfo::~CUIItemContainerInfo()
{
    ClearCommonInfo();
    ClearContainerInfo();
    ClearInfectionInfo();
    ClearFinalCharInfo();

};


void CUIItemContainerInfo::Init()
{
    CUIXml uiXml;
    bool xml_result = uiXml.Init(CONFIG_PATH, UI_PATH, ITEM_BIG_INFO);
    R_ASSERT3(xml_result, " file parsing error ", uiXml.m_xml_file_name);

    CUIXmlInit xml_init;

    xml_init.InitWindow(uiXml, "main", 0, this);

    AttachChild(&UI_back);
    xml_init.InitStatic(uiXml, "back", 0, &UI_back);

    AttachChild(&UI_item_name);
    xml_init.InitStatic(uiXml, "item_name", 0, &UI_item_name);

    AttachChild(&m_stats_scroll_back);
    xml_init.InitStatic(uiXml, "stats_scroll_back", 0, &m_stats_scroll_back);


    m_stats_scroll = xr_new<CUIScrollView>();
    UI_back.AttachChild(m_stats_scroll);
    m_stats_scroll->SetAutoDelete(true);
    xml_init.InitScrollView(uiXml, "stats_scroll", 0, m_stats_scroll);


    //Общая информация об предмете
    auto* common_group = xr_new<CUIItemInfoCommonGroup>();
    common_group->Init(uiXml, "common_group", 0);
    common_group->SetAutoDelete(true);

    UI_main_group.AddGroup(EItemInfoCommonGroup, common_group);
    m_stats_scroll->AddWindow(common_group, true);
    
    common_group->AddRow(uiXml, "item_common_static", EItemInfoCommonRank, "rank", "newbie");
    common_group->AddRow(uiXml, "item_common_static", EItemInfoCommonClass, "class", "Bag");
    common_group->AddRow(uiXml, "item_common_static", EItemInfoCommonWeight, "weight", "1.0");
    common_group->AddRow(uiXml, "item_common_static", EItemInfoCommonOwner, "owner", "name");

    //Общая информация о контейнер
    auto* container_group = xr_new<CUIIteminfoContainerGroup>();
    container_group->Init(uiXml, "container_group", 0);
    container_group->SetAutoDelete(true);

    UI_main_group.AddGroup(EItemInfoContainerGroup, container_group);
    m_stats_scroll->AddWindow(container_group, true);

    container_group->AddRow(uiXml, "item_common_static", EItemInfoContainerType, "type", "bag");
    container_group->AddRow(uiXml, "item_common_static", EItemInfoContainerDefence, "defence", "100%");
    container_group->AddRow(uiXml, "item_common_static", EItemInfoContainerEffect, "effecty", "100%");
    container_group->AddRow(uiXml, "item_common_static", EItemInfoContainerSlots, "slots", "0");

    //Информация о заражениях, которые защищает контейнер. Так как он может и не иметь защиты, то на момент инита создадим только саму группу
    auto* infection_group = xr_new<CUIItemInfoInfectionGroup>();
    infection_group->Init(uiXml, "infection_group", 0);
    infection_group->SetAutoDelete(true);
    infection_group->Show(false);

    UI_main_group.AddGroup(EItemInfoContainerInfectionGroup, infection_group);
    m_stats_scroll->AddWindow(infection_group, true);
    
    // Информация о характеристиках контейнера. Так как он может и не иметь характеристик, то на момент инита создадим только саму группу
    auto* char_group = xr_new<CUIItemInfoCharGroup>();   
    char_group->Init(uiXml, "char_group", 0);
    char_group->SetAutoDelete(true);
    char_group->Show(false);

    UI_main_group.AddGroup(EItemInfoContainerCharGroup, char_group);
    m_stats_scroll->AddWindow(char_group, true);
};

void CUIItemContainerInfo::SetCommonInfo(CSCItemContainer* item)
{
    // Заместо созданных сейчас данных будут читаться данные контейнера, но его пока нет
    auto wnd = UI_main_group.GetGroup(EItemInfoCommonGroup);
    CUIItemInfoCommonGroup* group = smart_cast<CUIItemInfoCommonGroup*>(wnd);

    CUIXml uiXml;
    bool result = uiXml.Init(CONFIG_PATH, UI_PATH, ITEM_BIG_INFO);
    R_ASSERT3(result, "file parsing error", uiXml.m_xml_file_name);




    shared_str item_name = item->Name();
    shared_str rank = item->GetItemRank();
    shared_str cls = item->GetItemClass().c_str();
    float weight = item->Weight();
    shared_str owner_name = Actor()->Name();


    UI_item_name.SetText(item_name.c_str());


    LPCSTR rank_str = rank.c_str();
    if (rank_str)
    {
        auto* rank_row = group->GetRow(EItemInfoCommonRank);
        if (!rank_row)
        {
            rank_row = group->AddRow(uiXml, "item_common_static", EItemInfoCommonRank, "rank", rank_str);
        }

            // Позже здесь создадим строку, если ее нет, но пока ничегно не делаем
        rank_row->SetName(*CStringTable().translate("sc_st_rank_name"));
        rank_row->SetValue(rank_str);
    }

    LPCSTR cls_str = cls.c_str();
    if (cls_str)
    {
        auto* cls_row = group->GetRow(EItemInfoCommonClass);
        if (!cls_row)
        {
            cls_row = group->AddRow(uiXml, "item_common_static", EItemInfoCommonClass, "class", cls_str);
        }
            // Позже здесь создадим строку, если ее нет, но пока ничегно не делаем
        cls_row->SetName(*CStringTable().translate("sc_st_class_name"));
        cls_row->SetValue(cls_str);
    }

    if (weight)
    {
        shared_str str = ValueToString(weight);
        auto* weight_row = group->GetRow(EItemInfoCommonWeight);
        if (!weight_row)
        {
            weight_row = group->AddRow(uiXml, "item_common_static", EItemInfoCommonWeight, "weight", str.c_str());
        }
            // Позже здесь создадим строку, если ее нет, но пока ничегно не делаем
        weight_row->SetName(*CStringTable().translate("sc_st_weight_name"));
        weight_row->SetValue(str.c_str());
    }

    LPCSTR owner_str = owner_name.c_str(); 
    if (owner_str)
    {
        auto* owner_row = group->GetRow(EItemInfoCommonOwner);
        if (!owner_row)
        {
            owner_row = group->AddRow(uiXml, "item_common_static", EItemInfoCommonOwner, "owner", owner_str);
        }
            // Позже здесь создадим строку, если ее нет, но пока ничегно не делаем
        owner_row->SetName(*CStringTable().translate("sc_st_owner_name"));
        owner_row->SetValue(owner_str);
    }

    if (group->GetGroupSize() > 0)
    {
        group->Show(true);
        m_stats_scroll->ForceUpdate();
    }
}

void CUIItemContainerInfo::ClearCommonInfo()
{
    CUIWindow* wnd = UI_main_group.GetGroup(EItemInfoCommonGroup);
    if (!wnd)
    {
        Msg("CUIItemContainerInfo::ClearCommonInfo: common group is nullptr");
        return;
    }

    CUIItemInfoCommonGroup* group = smart_cast<CUIItemInfoCommonGroup*>(wnd);
    if (group)
        group->ClearAllRows();

    wnd->Show(false);
}

void CUIItemContainerInfo::SetContainerInfo(CSCItemContainer* item)
{
    // контейнера пока нет, так что пока просто создаем заглушку
    CUIWindow* wnd = UI_main_group.GetGroup(EItemInfoContainerGroup);
    if (!wnd)
        return;

    CUIIteminfoContainerGroup* group = smart_cast<CUIIteminfoContainerGroup*>(wnd);
    if (!group)
        return;
    
    shared_str subclass = item->GetItemSubClass();
    const auto& defence = item->GetContainerDefence();
    const auto& effect = item->GetContainerEffective();
    const auto& slots = item->GetContainerArtSlots();


    CUIXml uiXml;
    bool result = uiXml.Init(CONFIG_PATH, UI_PATH, ITEM_BIG_INFO);
    R_ASSERT3(result, "file parsing error", uiXml.m_xml_file_name);

    LPCSTR subclass_str = subclass.c_str();
    if (subclass_str)
    {
        auto* type_row = group->GetRow(EItemInfoContainerType);
        if (!type_row)
        {
            type_row = group->AddRow(uiXml, "item_common_static", EItemInfoContainerType, "type", subclass_str);
        }
        type_row->SetName(*CStringTable().translate("sc_st_subclass_name"));
        type_row->SetValue(subclass_str);
    }

    if (defence)
    {
        auto* defence_row = group->GetRow(EItemInfoContainerDefence);
        shared_str str = ValueToString(defence);
        if (!defence_row)
        {
            defence_row = group->AddRow(uiXml, "item_common_static", EItemInfoContainerDefence, "defence", str.c_str());
        }
        defence_row->SetName(*CStringTable().translate("sc_st_cont_defence"));
        defence_row->SetValue(str.c_str());
    }

    if (effect)
    {
        auto* effect_row = group->GetRow(EItemInfoContainerEffect);
        shared_str str = ValueToString(effect);
        if (!effect_row)
        {
            effect_row = group->AddRow(uiXml, "item_common_static", EItemInfoContainerEffect, "effect", str.c_str());
        }
        effect_row->SetName(*CStringTable().translate("sc_st_cont_effect"));
        effect_row->SetValue(str.c_str());
    }

    if (slots)
    {
        auto* slots_row = group->GetRow(EItemInfoContainerSlots);
        shared_str str = ValueToString(slots);
        if (!slots_row)
        {
            slots_row = group->AddRow(uiXml, "item_common_static", EItemInfoContainerSlots, "slots", str.c_str());
        }
        slots_row->SetName(*CStringTable().translate("sc_st_cont_slots"));
        slots_row->SetValue(str.c_str());
    }

    if (group->GetGroupSize() > 0)
    {
        group->Show(true);
        m_stats_scroll->ForceUpdate();
    }
}

void CUIItemContainerInfo::ClearContainerInfo()
{
    CUIWindow* wnd = UI_main_group.GetGroup(EItemInfoContainerGroup);
    if (!wnd)
    {
        Msg("CUIItemContainerInfo::ClearContainerInfo: container group is nullptr");
        return;
    }

    CUIIteminfoContainerGroup* group = smart_cast<CUIIteminfoContainerGroup*>(wnd);
    if (group)
        group->ClearAllRows();
    wnd->Show(false);
}

void CUIItemContainerInfo::SetInfectionInfo(CSCItemContainer* item)
{
    CUIWindow* wnd = UI_main_group.GetGroup(EItemInfoContainerInfectionGroup);
    if (!wnd)
    {
        Msg("CUIItemContainerInfo::SetInfectionDefence: infection group is nullptr");
        return;
    }

    CUIItemInfoInfectionGroup* group = smart_cast<CUIItemInfoInfectionGroup*>(wnd);
    if (!group)
        return;

    if (group)
        group->Show(true);

    CUIXml uiXml;
    bool result = uiXml.Init(CONFIG_PATH, UI_PATH, ITEM_BIG_INFO);
    R_ASSERT3(result, "file parsing error", uiXml.m_xml_file_name);

    for (int i = ERadiation_inf; i < EInfectionMax; ++i)
    {
        EInfection type = static_cast<EInfection>(i);
        const float* pValue = item->GetInfection(type);
        if (!pValue)
            continue;

        shared_str str = ValueToString(*pValue);

        auto* row = group->GetRow(type);
        if (!row)
        {
            row = group->AddRow(uiXml, "infection_static", type, _imm_infection_name[i], str.c_str());
        }
        row->SetName(*CStringTable().translate(_imm_infection_name[i]));
        row->SetValue(str.c_str());
    }

    if (group->GetGroupSize() > 0)
    {
        group->Show(true);
        m_stats_scroll->ForceUpdate();
    }
}

void CUIItemContainerInfo::ClearInfectionInfo()
{
    CUIWindow* wnd = UI_main_group.GetGroup(EItemInfoContainerInfectionGroup);
    if (!wnd)
    {
        Msg("CUIItemContainerInfo::ClearInfectionDefence: infection group is nullptr");
        return;
    }
    CUIItemInfoInfectionGroup* group = smart_cast<CUIItemInfoInfectionGroup*>(wnd);
    if (group)
        group->ClearAllRows();
    wnd->Show(false);
}

void CUIItemContainerInfo::SetFinalCharInfo(CSCItemContainer* item)
{
    CUIWindow* wnd = UI_main_group.GetGroup(EItemInfoContainerCharGroup);
    if (!wnd)
    {
        Msg("CUIItemContainerInfo::SetFinalChar: char group is nullptr");
        return;
    }
    CUIItemInfoCharGroup* group = smart_cast<CUIItemInfoCharGroup*>(wnd);
    if (!group)
        return;

    if (group)
        group->Show(true);

    CUIXml uiXml;
    bool result = uiXml.Init(CONFIG_PATH, UI_PATH, ITEM_BIG_INFO);
    R_ASSERT3(result, "file parsing error", uiXml.m_xml_file_name);

    for (int i = EJivuchest; i < EItemCharMax; ++i)
    {
        EItemChar type = static_cast<EItemChar>(i);
        const float* value = item->GetChar(type);
        if (!value)
            continue;

        shared_str str = ValueToString(*value);

        auto* row = group->GetRow(type);
        if (!row)
        {
            row = group->AddRow(uiXml, "static_char", type, _imm_char_names[i], str.c_str());
        }
        row->SetName(*CStringTable().translate(_imm_char_names[i]));
        row->SetValue(str.c_str());
    }

    if (group->GetGroupSize() > 0)
    {
        group->Show(true);
        m_stats_scroll->ForceUpdate();
    }
}

void CUIItemContainerInfo::ClearFinalCharInfo()
{
    CUIWindow* wnd = UI_main_group.GetGroup(EItemInfoContainerCharGroup);
    if (!wnd)
    {
        Msg("CUIItemContainerInfo::ClearFinalChar: char group is nullptr");
        return;
    }
    CUIItemInfoCharGroup* group = smart_cast<CUIItemInfoCharGroup*>(wnd);
    if (group)
        group->ClearAllRows();
    wnd->Show(false);
}

//Временно добавил обьект на который нажали
void CUIItemContainerInfo::Show(CSCItemContainer* item)
{ 
    inherited::Show();

    if (!owner)
        owner = smart_cast<CActor*>(Level().CurrentEntity());

    if (!item)
    {
        Msg("CUIItemContainerInfo::Show item is not exist");
        return;
    }

    SetCommonInfo(item);
    SetContainerInfo(item);
    SetInfectionInfo(item);
    SetFinalCharInfo(item);

    if (m_need_reinit)
    {
        Init();
        m_need_reinit = false;
    }
    if (m_need_update_scroll)
        {
        m_stats_scroll->ForceUpdate();
        m_need_update_scroll = false;
    }


}

void CUIItemContainerInfo::Hide() 
{
    ClearCommonInfo();
    ClearContainerInfo();
    ClearInfectionInfo();
    ClearFinalCharInfo();
    inherited::Hide();
}

void CUIItemContainerInfo::Draw() 
{ 
    CUIWindow::Draw();
}



CSCItemContainer::CSCItemContainer()
{ 
    m_afSlots = 0; 
    m_fInsideDefence = 0.f;
    m_fEffective = 0.f;

    m_physic_item = 0;
}

DLL_Pure* CSCItemContainer::_construct()
{
    m_physic_item = smart_cast<CPhysicItem*>(this);
    return (inherited::_construct());
}

void CSCItemContainer::Load(LPCSTR section) 
{ 
    inherited::Load(section);
  
    m_afSlots = pSettings->r_u32(section, "artefact_slots");
    m_fInsideDefence = pSettings->r_float(section, "container_defence");
    m_fEffective = pSettings->r_float(section, "container_effective");

    if (pSettings->r_string(section, "infection_defence"))
        item_infection.LoadInfection(section, pSettings);
}

BOOL CSCItemContainer::net_Spawn(CSE_Abstract* DC) 
{ 
    if (!inherited::net_Spawn(DC)) 
        return false;

    return TRUE;
}

bool CSCItemContainer::Useful() const
{
    if (!inherited::Useful())
        return false; 

    return true;
}

void CSCItemContainer::OnH_B_Independent(bool just_before_destroy)
{ 
    if (!Useful())
    {
        object().setVisible(false);
        object().setEnabled(false);
        if (m_physic_item)
            m_physic_item->m_ready_to_destroy = true;
    }

    inherited::OnH_B_Independent(just_before_destroy);
}


CContainerItemObject::CContainerItemObject() {}

CContainerItemObject::~CContainerItemObject() {}

DLL_Pure* CContainerItemObject::_construct()
{
    CSCItemContainer::_construct();
    CPhysicItem::_construct();
    return (this);
}

void CContainerItemObject::Load(LPCSTR section)
{
    CPhysicItem::Load(section);
    CSCItemContainer::Load(section);
}

void CContainerItemObject::Hit(SHit* pHDS)
{
    CPhysicItem::Hit(pHDS);
    CSCItemContainer::Hit(pHDS);
}

void CContainerItemObject::OnH_A_Independent()
{
    CSCItemContainer::OnH_A_Independent();
    CPhysicItem::OnH_A_Independent();
    // If we are dropping used item before removing - don't show it
    if (!Useful())
    {
        setVisible(false);
        setEnabled(false);
    }
}

void CContainerItemObject::OnH_B_Independent(bool just_before_destroy)
{
    CSCItemContainer::OnH_B_Independent(just_before_destroy);
    CPhysicItem::OnH_B_Independent(just_before_destroy);
}

void CContainerItemObject::OnH_B_Chield()
{
    CPhysicItem::OnH_B_Chield();
    CSCItemContainer::OnH_B_Chield();
}

void CContainerItemObject::OnH_A_Chield()
{
    CPhysicItem::OnH_A_Chield();
    CSCItemContainer::OnH_A_Chield();
}

void CContainerItemObject::UpdateCL()
{
    CPhysicItem::UpdateCL();
    CSCItemContainer::UpdateCL();
}

void CContainerItemObject::OnEvent(NET_Packet& P, u16 type)
{
    CPhysicItem::OnEvent(P, type);
    CSCItemContainer::OnEvent(P, type);
}

BOOL CContainerItemObject::net_Spawn(CSE_Abstract* DC)
{
    BOOL res = CPhysicItem::net_Spawn(DC);
    CSCItemContainer::net_Spawn(DC);
    return (res);
}

void CContainerItemObject::net_Destroy()
{
    CSCItemContainer::net_Destroy();
    CPhysicItem::net_Destroy();
}

void CContainerItemObject::net_Export(CSE_Abstract* E) {CSCItemContainer::net_Export(E);}

void CContainerItemObject::save(NET_Packet& packet)
{
    CPhysicItem::save(packet);
    CSCItemContainer::save(packet);
}

void CContainerItemObject::load(IReader& packet)
{
    CPhysicItem::load(packet);
    CSCItemContainer::load(packet);
}

void CContainerItemObject::renderable_Render(u32 context_id, IRenderable* root)
{
    CPhysicItem::renderable_Render(context_id, root);
    CSCItemContainer::renderable_Render(context_id, root);
}

void CContainerItemObject::reload(LPCSTR section)
{
    CPhysicItem::reload(section);
    CSCItemContainer::reload(section);
}

void CContainerItemObject::reinit()
{
    CSCItemContainer::reinit();
    CPhysicItem::reinit();
}

void CContainerItemObject::activate_physic_shell() { CSCItemContainer::activate_physic_shell(); }

void CContainerItemObject::on_activate_physic_shell() { CPhysicItem::activate_physic_shell(); }

u32 CContainerItemObject::ef_weapon_type() const { return (0); }

bool CContainerItemObject::Useful() const { return (CSCItemContainer::Useful()); }