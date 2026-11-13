#pragma once
#include "stdafx.h"
#include "UIGameCustom.h"


enum class IconStaticClass : u8
{
    eNone,
    eReward,
    eSpot,
    eTask
};

enum class IconStaticType : u8
{
    eNone,
    eSpotBase,
    eSpotZombie,
    eSpotMutant,
    eSpotBossNpc,
    eSpotBossMutant,
    eReward,
    eMainTask,
    eSecondTask,
};

struct IconInfo
{
    Fvector w_position;
    CUIStatic* ui_static = nullptr;
    float max_distance = 0.f;
};

struct HUDIconInfo
{
    shared_str m_icon_id;
    xr_map<u32, IconInfo> m_icons_statics;
};


//#+# класс отвечает за иконки в мировом пространстве
class CHUDIcons
{
public:
    HUDIconInfo* CreateCstmStatic(IconStaticClass static_class);
    void DeleteCstmStatic(IconStaticClass static_class);
    HUDIconInfo* GetCstmStatic(IconStaticClass static_class);
    void ClearCstmStatics();

    
    //#+# Пос айди является кастомным айди, который задается от айди спота\задания\награды
    void CreateCUIStatic(IconStaticClass static_class, IconStaticType static_type, u32 pos_id, Fvector w_pos);
    void DeleteCUIStatic(IconStaticClass static_class, u32 pos_id);
    IconInfo GetCUIStatic(IconStaticClass static_class, u32 pos_id);

    CUIStatic* CreateIcon(IconStaticType static_type);



    void Update(u32 dt);



protected:
    xr_map<IconStaticClass, HUDIconInfo*> UI_icons;
    u32 m_update_time = 0;

};