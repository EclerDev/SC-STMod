#pragma once

#include "CustomDetector.h"
#include "EliteDetector.h"
#include "HudSound.h"
#include "ui/UIProgressShape.h"

class CUIMetallDetector;

/* struct METAL_DETECTOR_TYPE
{
    Fvector2 freq;
    HUD_SOUND detect_snds;
};
*/
class CMetallSpots
{
public:
    void AddSpot(const Fvector& pos) { m_spots.push_back(pos); }

    void RemoveSpot(const Fvector& pos)
    {
        auto it = std::find_if(m_spots.begin(), m_spots.end(), [&](const Fvector& value) { return pos.distance_to(value) <= 0.01f; });

        if (it != m_spots.end())
            m_spots.erase(it);
    }

    const xr_vector<Fvector>& GetAllSpots() const { return m_spots; }
    bool IsEmpty() const { return m_spots.empty(); }
    void ClearSpots() { m_spots.clear(); }

private:
    xr_vector<Fvector> m_spots;
};

extern CMetallSpots g_metal_spots;

class CMetalDetector : public CEliteDetector
{
    typedef CEliteDetector inherited;

public:
    CMetallSpots m_spots;

    ~CMetalDetector() { m_spots.ClearSpots(); }
    Fvector2 m_snd_freq;
    float m_snd_time = 0.f;
    float m_cur_period = 0.f;
    virtual void Load(LPCSTR section) override;
    virtual LPCSTR ui_xml_tag() const { return "metal"; }
    HUD_SOUND m_sound_detect;
    // CUIProgressShape* m_pSector = nullptr;
    CUIMetallDetector& GetCUI() { return ui(); }

protected:
    virtual void UpdateAf() override;
    virtual void CreateUI() override;
    CUIMetallDetector& ui();
};
