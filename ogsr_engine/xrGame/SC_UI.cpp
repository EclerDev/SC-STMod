#include "stdafx.h"
#include "SC_UI.h"
#include "ui/UIStatic.h"

LPCSTR _imm_infection_sections[] = {"radiation_infection", "biological_infection", "psi_infection", "temperature_infection", "cold_infection"};

LPCSTR _imm_char_sections[] = {
    "jivuchest_char",           "temperature_char",   "gorenie_char",           "blast_defence_char", "power_char",          "weight_char",         "bio_v_char",
    "psi_defence_char",         "react_on_elec_char", "health_v_char",          "power_v_char",       "bullet_defence_char", "wound_v_char",        "wound_defence_char",
    "radiation_v_char",         "run_speed_char",     "move_speed_char",        "fire_defence_char",  "recoil_char",         "mutant_defence_char", "react_on_mutant_char",
    "electro_defence_char",     "heal_power_char",    "bio_defence_char",       "react_on_chem_char", "stoykost_char",       "preioud_heal_char",   "react_on_fire_char",
    "temp_defence_char", "psi_defence_char",   "radiation_defence_char", "pokachivanie_char"};



CItemChar::CItemChar() { m_char_types.resize(EItemChar::EItemCharMax); }

void CItemChar::LoadChar(LPCSTR section, CInifile* ini)
{
    R_ASSERT2(ini->section_exist(section), section);

    for (u32 i = EJivuchest; i < EItemCharMax; ++i)
    {
        if (ini->line_exist(section, _imm_char_sections[i]))
        {
            auto& data = m_char_types[i];

            data.value = ini->r_float(section, _imm_char_sections[i]);
            data.exist = true;
        }
    }
}

float* CItemChar::GetChar(EItemChar type) 
{ 
    const auto index = static_cast<u32>(type); 
    VERIFY(index < m_char_types.size());

    auto& data = m_char_types[index];

    if (!data.exist)
        return nullptr;

    return &data.value;
}

bool CItemChar::IsCharExist()
{
    for (const auto& data : m_char_types)
    {
        if (data.exist)
            return true;

    }
    return false;
}

CItemInfection::CItemInfection() { m_infection_types.resize(EInfection::EInfectionMax); }

void CItemInfection::LoadInfection(LPCSTR section, CInifile* ini)
{
    R_ASSERT2(ini->section_exist(section), section);

    for (u32 i = EInfection::ERadiation_inf; i < EInfectionMax; ++i)
    {
        if (ini->line_exist(section, _imm_infection_sections[i]))
        {
            auto& data = m_infection_types[i];

            data.value = ini->r_float(section, _imm_infection_sections[i]);
            data.exist = true;
        }
    }
}

float* CItemInfection::GetInfection(EInfection type)
{ 
    const auto index = static_cast<u32>(type);
    VERIFY(index < m_infection_types.size());

    auto& data = m_infection_types[index];

    if (!data.exist)
        return nullptr;

    return &data.value;
}

bool CItemInfection::IsInfectionExist()
{
    for (const auto& data : m_infection_types)
    {
        if (data.exist)
            return true;
    }

    return false;
}

//Строка с названием параметра и самим параметром
////////////////////////////////////////////////
CUIItemInfoRow::CUIItemInfoRow()
{
    m_highlight = false;
    highlight_color = 0;
}

void CUIItemInfoRow::Init(CUIXml& xml, LPCSTR path, int index)
{
    CUIXmlInit xml_init;

    xml_init.InitWindow(xml, path, index, this);

    XML_NODE* stored_root = xml.GetLocalRoot();
    xml.SetLocalRoot(xml.NavigateToNode(path, index));

    m_name = xr_new<CUIStatic>();
    m_name->SetAutoDelete(true);
    AttachChild(m_name);
    xml_init.InitStatic(xml, "name", 0, m_name);

    m_value = xr_new<CUIStatic>();
    m_value->SetAutoDelete(true);
    AttachChild(m_value);
    xml_init.InitStatic(xml, "value", 0, m_value);

    xml.SetLocalRoot(stored_root);
}

void CUIItemInfoRow::SetName(LPCSTR name) { m_name->SetText(name); }
void CUIItemInfoRow::SetNameColor(u32 color) { m_name->SetTextColor(color); }
void CUIItemInfoRow::SetValue(LPCSTR value) { m_value->SetText(value); }
void CUIItemInfoRow::SetValueColor(u32 color) { m_value->SetTextColor(color); }
void CUIItemInfoRow::SetHightLightColor(u32 color) { highlight_color = color; }

// Линия разделитель 
////////////////////////////////////////////////
void CUIItemInfoSeparator::CentrilizeLine() 
{ 
    Fvector2 wnd_size = GetWndSize();
    Fvector2 line_size = m_line->GetWndSize();

    float x = (wnd_size.x * 0.5f) - (line_size.x * 0.5f);
    float y = (wnd_size.y * 0.5f) - (line_size.y * 0.5f);

    m_line->SetWndPos(Fvector2(x, y));
}




/*
CUIItemContainerArtCell::CUIItemContainerArtCell() {}

CUIItemContainerArtCell::~CUIItemContainerArtCell() {}

void CUIItemContainerArtCell::Init() 
{
    CUIXml uiXml;
    bool xml_result = uiXml.Init(CONFIG_PATH, UI_PATH, "ui_sc_test_wnd.xml");
    R_ASSERT3(xml_result, " file parsing error ", uiXml.m_xml_file_name);

    CUIXmlInit xml_init;

    xml_init.InitWindow(uiXml, "art_cell_wnd", 0, this);

    AttachChild(&UI_back);
    xml_init.InitStatic(uiXml, "art_cell_back", 0, &UI_back);

    AttachChild(&UI_color_back);
    xml_init.InitStatic(uiXml, "art_cell_color_back", 0, &UI_color_back);

    AttachChild(&UI_corner);
    xml_init.InitStatic(uiXml, "art_cell_corner", 0, &UI_corner);

    AttachChild(&UI_up_btn);
    xml_init.InitButton(uiXml, "art_cell_up_btn", 0, &UI_up_btn);
    UI_up_btn.Show(false);

    AttachChild(&UI_main_btn);
    xml_init.InitButton(uiXml, "art_cell_main_btn", 0, &UI_main_btn);

    //Позже тут еще будет статик заряда арта, он идет еще выше чем иконка
    AttachChild(&UI_item_icon);
    xml_init.InitStatic(uiXml, "art_cell_item_icon", 0, &UI_item_icon);
    UI_item_icon.Show(false);


}


CUIitemContainerCellOffer::CUIitemContainerCellOffer() {};
CUIitemContainerCellOffer::~CUIitemContainerCellOffer() {};

void CUIitemContainerCellOffer::Init()
{
    CUIXml uiXml;
    bool xml_result = uiXml.Init(CONFIG_PATH, UI_PATH, "ui_sc_test_wnd.xml");
    R_ASSERT3(xml_result, " file parsing error ", uiXml.m_xml_file_name);

    CUIXmlInit xml_init;

    xml_init.InitWindow(uiXml, "art_cell_wnd", 0, this);

    AttachChild(&UI_back);
    xml_init.InitStatic(uiXml, "art_cell_back", 0, &UI_back);

    AttachChild(&UI_color_back);
    xml_init.InitStatic(uiXml, "art_cell_color_back", 0, &UI_color_back);

    AttachChild(&UI_corner);
    xml_init.InitStatic(uiXml, "art_cell_corner", 0, &UI_corner);


    AttachChild(&UI_btn);
    xml_init.InitButton(uiXml, "art_cell_main_btn", 0, &UI_btn);

    // Позже тут еще будет статик заряда арта, он идет еще выше чем иконка
    AttachChild(&UI_item_icon);
    xml_init.InitStatic(uiXml, "art_cell_item_icon", 0, &UI_item_icon);
    UI_item_icon.Show(false);
}

void CUIitemContainerCellOffer::SetNullCell()
{ 
    UI_item_icon.Show(false); 
    UI_color_back.SetColor(1);
    UI_corner.SetColor(1);
}

void CUIitemContainerCellOffer::PrepareCell(CArtefact* art) 
{ 
    if (!art)
    {
        SetNullCell();
        return;
    }


    shared_str rank = art->GetItemRank();  
    LPCSTR tex_name = art->GetIconName().c_str();

    //Ранее мы должны были найти нужный цвет по рангу, но пока заготовки нет
    UI_color_back.SetColor(1);
    UI_corner.SetColor(1);

    UI_item_icon.InitTexture(tex_name);
    UI_item_icon.Show(true);

}


void CUIitemContainerCellStruct::AddElement(CArtefact* item)
{
    if (m_elements.size() >= 50)
      return;

    

}
void CUIitemContainerCellStruct::GetElement() {}
*/