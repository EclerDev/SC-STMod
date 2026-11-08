#include "StdAfx.h"
#include "MetalDetector.h"
#include "ui/ArtefactDetectorUI.h"
#include "script_game_object.h"
#include "player_hud.h"
#include "hudmanager.h"
#include "ui/UIStatic.h"
#include "ui/UIXmlInit.h"
#include "HudSound.h"
#include "ui/UIProgressShape.h"
constexpr const char* AF_SIGN = "af_sign";

CMetallSpots g_metal_spots;

void CMetalDetector::CreateUI()
{
    R_ASSERT(!m_ui);
    m_ui = xr_new<CUIMetallDetector>();
    ui().construct(this);
}

CUIMetallDetector& CMetalDetector::ui() { return *((CUIMetallDetector*)m_ui); }

void CMetalDetector::Load(LPCSTR section)
{
    inherited::Load(section);

    m_snd_freq = pSettings->r_fvector2(section, "snd_freq");

    HUD_SOUND::LoadSound(section, "snd_detect", m_sound_detect, SOUND_TYPE_ITEM_USING);
}

void CMetalDetector::UpdateAf()
{
    ui().Clear();

    if (g_metal_spots.IsEmpty())
        return;

    const auto& spots = g_metal_spots.GetAllSpots();

    float min_dist = flt_max;
    const Fvector* nearest_spot = nullptr;
    auto near_pos = Fvector{};

    Fvector detector_pos = Position();
    for (const auto& pos : spots)
    {
        const float dist = detector_pos.distance_to(pos);
        if (dist < min_dist)
        {
            min_dist = dist;
            nearest_spot = &pos;
            near_pos = pos;
        }
    }

    if (!nearest_spot || min_dist > m_fAfDetectRadius)
    {
        m_snd_time = 0.f;
        return;
    }

    const float fRelPow = _min(min_dist / m_fAfDetectRadius, 1.f);

    m_cur_period = m_snd_freq.x + (m_snd_freq.y - m_snd_freq.x) * (fRelPow * fRelPow);

    const float snd_freq = 0.9f + (1.4f - 0.9f) * (1.f - fRelPow);
    ui().RegisterItemToDraw(near_pos, "af_sign");
    if (m_snd_time > m_cur_period)
    {
        m_snd_time = 0.f;

        HUD_SOUND::PlaySound(m_sound_detect, Fvector{}, this, true);

        if (m_sound_detect.m_activeSnd)
            m_sound_detect.m_activeSnd->snd.set_frequency(snd_freq);
    }
    else
    {
        m_snd_time += Device.fTimeDelta;
    }
}

void CUIMetallDetector::Draw()
{
    if (!m_wrk_area)
        return;

    Fmatrix LM;
    GetUILocatorMatrix(LM);

    UIRender->CacheSetXformWorld(LM);

    CUIWindow::Draw();

    if (!m_pSector || m_items_to_draw.empty())
    {
        if (m_pSector)
            m_pSector->Show(false);

        return;
    }

    m_pSector->Show(true);

    //.	Frect r						= m_wrk_area->GetWndRect();
    Fvector2 wrk_sz = m_wrk_area->GetWndSize();
    Fvector2 rp;
    m_wrk_area->GetAbsolutePos(rp);

    Fmatrix M, Mc;
    float h, p;
    Device.vCameraDirection.getHP(h, p);
    Mc.setHPB(h, 0, 0);
    Mc.c.set(Device.vCameraPosition);
    M.invert(Mc);

    UI()->ScreenFrustumLIT().CreateFromRect(Frect().set(rp.x, rp.y, wrk_sz.x, wrk_sz.y));

    const float detect_radius = m_parent->m_fAfDetectRadius;
    const float radar_radius = _min(wrk_sz.x, wrk_sz.y) * 0.5f;

    float nearest_dist_sq = flt_max;
    float nearest_x = 0.f;
    float nearest_y = 0.f;

    for (const auto& item : m_items_to_draw)
    {
        Fvector pt3d;
        M.transform_tiny(pt3d, item.pos);

        // Координаты на радаре: X вправо, Y вверх.
        const float x = pt3d.x / detect_radius * radar_radius;
        const float y = pt3d.z / detect_radius * radar_radius;

        const float dist_sq = x * x + y * y;

        if (dist_sq < nearest_dist_sq)
        {
            nearest_dist_sq = dist_sq;
            nearest_x = x;
            nearest_y = y;
        }
    }

    if (nearest_dist_sq == flt_max)
    {
        m_pSector->Show(false);
    }
    else
    {
        m_pSector->Show(true);

        const float nearest_dist = _sqrt(nearest_dist_sq);

        // 0 — рядом, 1 — далеко.
        const float t = _min(_max(nearest_dist / radar_radius, 0.f), 1.f);

        // Рядом — максимальный угол, далеко — минимальный.
        const float min_angle = m_sector_min_angle * PI / 180.f;
        const float max_angle = m_sector_max_angle * PI / 180.f;

        const float angle = max_angle + (min_angle - max_angle) * t;

        // Инвертируем горизонтальное направление, поскольку сейчас
        // наблюдается перестановка правой и левой сторон.
        const float direction_angle = -atan2(nearest_x, nearest_y);

        // Центр сектора направлен на точку.
        m_pSector->SetAngleBegin(direction_angle + angle * 0.5f);
        m_pSector->SetAngleEnd(direction_angle - angle * 0.5f);
    }
}

void CUIMetallDetector::construct(CMetalDetector* p)
{
    m_parent = p;
    CUIXml uiXml;
    bool xml_result = uiXml.Init(CONFIG_PATH, UI_PATH, "ui_detector_artefact.xml");
    if (xml_result)
    {
        string512 buff;
        xr_strcpy(buff, p->ui_xml_tag());

        CUIXmlInit::InitWindow(uiXml, buff, 0, this);

        m_wrk_area = xr_new<CUIWindow>();

        xr_sprintf(buff, "%s:wrk_area", p->ui_xml_tag());

        CUIXmlInit::InitWindow(uiXml, buff, 0, m_wrk_area);
        m_wrk_area->SetAutoDelete(true);
        AttachChild(m_wrk_area);

        xr_sprintf(buff, "%s", p->ui_xml_tag());
        XML_NODE* pStoredRoot = uiXml.GetLocalRoot();
        uiXml.SetLocalRoot(uiXml.NavigateToNode(buff, 0));

        const int num = uiXml.GetNodesNum(buff, 0, "palette");
        if (num > 0)
        {
            for (int idx = 0; idx < num; ++idx)
            {
                CUIStatic* S = xr_new<CUIStatic>();
                shared_str name = uiXml.ReadAttrib("palette", idx, "id");
                m_palette[name] = S;
                CUIXmlInit::InitStatic(uiXml, "palette", idx, S);
                S->SetAutoDelete(true);
                m_wrk_area->AttachChild(S);
                S->SetCustomDraw(true);
            }
        }
        else
        {
            CUIStatic* S = xr_new<CUIStatic>();
            m_palette[AF_SIGN] = S;
            CUIXmlInit::InitStatic(uiXml, AF_SIGN, 0, S);
            S->SetAutoDelete(true);
            m_wrk_area->AttachChild(S);
            S->SetCustomDraw(true);
        }

        m_pSector = xr_new<CUIProgressShape>();

        CUIXmlInit::InitProgressShape(uiXml, "progress", 0, m_pSector);
        m_pSector->SetAutoDelete(true);
        m_wrk_area->AttachChild(m_pSector);
        m_pSector->Show(false);

        uiXml.SetLocalRoot(pStoredRoot);
    }
    else
    {
        // R_ASSERT3(xml_result, "xml file not found", "ui_detector_artefact.xml");
        Msg("!![%s] xml file [ui_detector_artefact.xml] not found!", __FUNCTION__);
    }

    Fvector _map_attach_p = pSettings->r_fvector3(m_parent->cNameSect(), "ui_p");
    Fvector _map_attach_r = pSettings->r_fvector3(m_parent->cNameSect(), "ui_r");

    if (pSettings->line_exist(m_parent->cNameSect(), "ui_sector_min_angle"))
    {
        m_sector_min_angle = pSettings->r_float(m_parent->cNameSect(), "ui_sector_min_angle");
    }
    if (pSettings->line_exist(m_parent->cNameSect(), "ui_sector_max_angle"))
    {
        m_sector_max_angle = pSettings->r_float(m_parent->cNameSect(), "ui_sector_max_angle");
    }

    m_sector_min_angle = _min(_max(m_sector_min_angle, 1.f), 179.f);
    m_sector_max_angle = _min(_max(m_sector_max_angle, 1.f), 179.f);

    if (m_sector_min_angle > m_sector_max_angle)
    {
        std::swap(m_sector_min_angle, m_sector_max_angle);
    }

    _map_attach_r.mul(PI / 180.f);
    m_map_attach_offset.setHPB(_map_attach_r.x, _map_attach_r.y, _map_attach_r.z);
    m_map_attach_offset.translate_over(_map_attach_p);
}

void CUIMetallDetector::update()
{
    inherited::update();
    CUIWindow::Update();
}

void CUIMetallDetector::GetUILocatorMatrix(Fmatrix& _m)
{
    Fmatrix trans = m_parent->HudItemData()->m_item_transform;
    u16 bid = m_parent->HudItemData()->m_model->LL_BoneID("cover");
    Fmatrix cover_bone = m_parent->HudItemData()->m_model->LL_GetTransform(bid);
    _m.mul(trans, cover_bone);
    _m.mulB_43(m_map_attach_offset);
}

void CUIMetallDetector::Clear() { m_items_to_draw.clear(); }
void CUIMetallDetector::RegisterItemToDraw(const Fvector& p, const shared_str& palette_idx)
{
    auto it = m_palette.find(palette_idx);
    if (it == m_palette.end())
    {
        // Msg("! RegisterItemToDraw. static not found for [%s]", palette_idx.c_str());
        return;
    }
    CUIStatic* S = m_palette[palette_idx];
    SDrawOneItem itm(S, p);
    m_items_to_draw.push_back(itm);
}
