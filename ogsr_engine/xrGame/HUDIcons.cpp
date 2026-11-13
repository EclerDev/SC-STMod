#include "stdafx.h"
#include "HUDIcons.h"
#include "Level.h"
#include "HUDManager.h"
#include "ui/UIStatic.h"
#include "ui/UIXmlInit.h"
#include "Actor.h"


LPCSTR _ui_cstm_static_class[] = {"ui_sc_none_cstm", "ui_sc_reward_cstm", "ui_sc_spot_cstm", "ui_sc_task_cstm"};
LPCSTR _ui_static_type_path[] = {"ui_sc_icon_none", "ui_sc_icon_spot_zombie", "ui_sc_icon_spot_mutant", "ui_sc_icon_spot_bossnpc"};


const u32 icons_update = 16;

HUDIconInfo* CHUDIcons::CreateCstmStatic(IconStaticClass static_class) 
{ 
    if (GetCstmStatic(static_class))
    {
        Msg("CHUDIcons::CreateCstmStatic: There is alryady exist cstm static with id %u ", static_class);
        return nullptr;
    }

    shared_str cstm_st_id = _ui_cstm_static_class[static_cast<u8>(static_class)];

    HUDIconInfo* cstm_info = xr_new<HUDIconInfo>();
    HUD().GetUI()->UIGame()->AddCustomStatic(cstm_st_id.c_str(), true);

    cstm_info->m_icon_id = cstm_st_id;
    UI_icons.emplace(static_class, cstm_info);

    Msg("CHUDIcons::CreateCstmStatic: Success create cstm static with id %u ", static_class);
    return cstm_info;
}

void CHUDIcons::DeleteCstmStatic(IconStaticClass static_class)
{ 
    HUDIconInfo* cstm_info = GetCstmStatic(static_class);
    if (!cstm_info)
    {
        Msg("CHUDIcons::DeleteCstmStatic: There is no static with id %u ", static_class);
        return;
    }

   HUD().GetUI()->UIGame()->RemoveCustomStatic(cstm_info->m_icon_id.c_str());
   

   xr_delete(cstm_info);
   UI_icons.erase(static_class);
   Msg("CHUDIcons::DeleteCstmStatic: Success delete cstm static with id %u ", static_class);
}

HUDIconInfo* CHUDIcons::GetCstmStatic(IconStaticClass static_class) 
{ 
	auto it = UI_icons.find(static_class);
    return it != UI_icons.end() ? it->second : nullptr;
}

void CHUDIcons::ClearCstmStatics()
{
    for (auto& [id, cstm_info] : UI_icons)
    {
        HUD().GetUI()->UIGame()->RemoveCustomStatic(cstm_info->m_icon_id.c_str());
        xr_delete(cstm_info);
    }

    UI_icons.clear();
}


void CHUDIcons::CreateCUIStatic(IconStaticClass static_class, IconStaticType static_type, u32 pos_id, Fvector w_pos)
{
    HUDIconInfo* cstm_info = GetCstmStatic(static_class);

    if (!cstm_info)
        cstm_info = CreateCstmStatic(static_class);

    if (!cstm_info)
        return;

    if (cstm_info->m_icons_statics.find(pos_id) != cstm_info->m_icons_statics.end())
    {
        Msg("CHUDIcons::CreateCUIStatic: Icon with id %u already exists!", pos_id);
        return;
    }

    SDrawStaticStruct* cstm_static = HUD().GetUI()->UIGame()->GetCustomStatic(cstm_info->m_icon_id.c_str());

    if (!cstm_static)
        return;

    CUIStatic* ui_static = CreateIcon(static_type);
    if (!ui_static)
        return;

    IconInfo icon_info;
    icon_info.w_position = w_pos.add(Fvector(0.f, 5.f, 0.f));
    icon_info.ui_static = ui_static;

    cstm_info->m_icons_statics.emplace(pos_id, icon_info);
    cstm_static->wnd()->AttachChild(ui_static);
}



void CHUDIcons::DeleteCUIStatic(IconStaticClass static_class, u32 pos_id) 
{ 
    HUDIconInfo* cstm_info = GetCstmStatic(static_class);
    if (!cstm_info)
    {
        Msg("CHUDIcons::DeleteCUIStatic: There is no static with id %u ", static_class);
        return;
    }

    auto it = cstm_info->m_icons_statics.find(pos_id);
    if (it == cstm_info->m_icons_statics.end())
        return;

    if (it->second.ui_static)
        it->second.ui_static->DetachFromParent();

    cstm_info->m_icons_statics.erase(it);
}

IconInfo CHUDIcons::GetCUIStatic(IconStaticClass static_class, u32 pos_id)
{
    HUDIconInfo* cstm_info = GetCstmStatic(static_class);
    if (!cstm_info)
        return {};

    auto it = cstm_info->m_icons_statics.find(pos_id);
    if (it == cstm_info->m_icons_statics.end())
        return {};

    return it->second;
}

CUIStatic* CHUDIcons::CreateIcon(IconStaticType static_type)
{
    //Здесь добавим позже логику поиска самой старой иконки, чтобы ограничить на каждый тип до 5-10

    CUIXml uiXml;
    bool xml_result = uiXml.Init(CONFIG_PATH, UI_PATH, "ui_sc_hud_icons_total.xml");
    R_ASSERT3(xml_result, " file parsing error ", uiXml.m_xml_file_name);

    CUIXmlInit xml_init;
    LPCSTR path = _ui_static_type_path[static_cast<u8>(static_type)];


    if (!path)
    {
        Msg("CHUDIcons::CreateStatic: There is not path with type %u", static_type);
        return nullptr;
    }

    CUIStatic* new_static = xr_new<CUIStatic>();
    new_static->SetAutoDelete(true);

    xml_init.InitStatic(uiXml, path, 0, new_static);

    return new_static;
}

constexpr float screen_width = 1024.f;
constexpr float screen_height = 768.f;

void CHUDIcons::Update(u32 dt)
{ 
    m_update_time += dt;

    if (m_update_time < icons_update)
        return;

    m_update_time %= icons_update;

    const float half_fov = tan(deg2rad(Device.fFOV) * 0.5f);

    for (auto& [class_id, data] : UI_icons)
    {
        if (!data || data->m_icons_statics.size() == 0)
            continue;

        for (auto& [id, icon_info] : data->m_icons_statics)
        {
            CUIStatic* ui_static = icon_info.ui_static;

            if (!ui_static)
                continue;

            Fvector relative;
            relative.sub(icon_info.w_position, Device.vCameraPosition);

            const float x = relative.dotproduct(Device.vCameraRight);
            const float y = relative.dotproduct(Device.vCameraTop);
            const float z = relative.dotproduct(Device.vCameraDirection);

            if (z <= 0.01f)
            {
                ui_static->Show(false);
                continue;
            }

            if (icon_info.max_distance > 0.f && relative.square_magnitude() > icon_info.max_distance * icon_info.max_distance)
            {
                ui_static->Show(false);
                continue;
            }

            const float screen_x = x * Device.fASPECT / (z * half_fov);
            const float screen_y = y / (z * half_fov);

            float pixel_x = (screen_x + 1.f) * screen_width * 0.5f;
            float pixel_y = (1.f - screen_y) * screen_height * 0.5f;

            if (pixel_x < 0.f || pixel_x > screen_width || pixel_y < 0.f || pixel_y > screen_height)
            {
                ui_static->Show(false);
                continue;
            }

            pixel_x -= ui_static->GetWndSize().x * 0.5f;
            pixel_y -= ui_static->GetWndSize().y * 0.5f;

            ui_static->SetWndPos(pixel_x, pixel_y);
            ui_static->Show(true);
        }
    }
}