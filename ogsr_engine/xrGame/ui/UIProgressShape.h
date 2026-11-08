#pragma once

#include "UIWindow.h"

class CUIStatic;

class CUIProgressShape : public CUIWindow
{
    friend class CUIXmlInit;

public:
    CUIProgressShape();
    virtual ~CUIProgressShape();
    void SetPos(int pos, int max);
    void SetPos(float pos);
    void SetTextVisible(bool b);

    // #+#
    void SetSectorCount(u32 sector_count) { m_sectorCount = sector_count; }
    void SetAngleBegin(float angle) { m_angle_begin = angle; }
    void SetAngleEnd(float angle) { m_angle_end = angle; }

    virtual void Draw();

protected:
    bool m_bClockwise{};
    u32 m_sectorCount{};
    float m_stage;
    CUIStatic* m_pTexture;
    CUIStatic* m_pBackground;
    bool m_bText;
    bool m_blend;

    float m_angle_begin;
    float m_angle_end;
};