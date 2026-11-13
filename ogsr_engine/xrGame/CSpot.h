#pragma once

#include "script_zone.h"

class CSpot : public CScriptZone
{
    using inherited = CScriptZone;

public:
    CSpot() {};

    void OnActivate() {};
    void OnDeactivate() {};

    void OnAttacked() {};

    void SpawnNPC() {};
    void RemoveNPC() {};

private:
    u32 m_spot_id = 0;
};