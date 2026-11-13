#pragma once
#include <cstring>
#include "xrCore.h"
#include "HUDIcons.h"

class CSE_Abstract;

enum class SpotState : u8
{
    eNone,
    eOffline,
    eOnline
};

enum class SpotStatus: u8
{
    eNormal,
    eCombat
};


struct SSpotNpcData
{
    shared_str section;
    u16 id = u16(-1);

    float m_fHealth = 1.f;
    Fvector position;
};


struct SSpotData
{
    shared_str section;
    shared_str level_name;

    Fvector position;
    float radius;
    u32 reload_time = 0;
    u32 cd_time = 0;

    SpotState spot_state = SpotState::eNone;
    SpotStatus spot_status = SpotStatus::eNormal;

    u32 maximum_npc;
    shared_str npc_section;
    xr_vector<SSpotNpcData> npcs;

};


class CSpotManager
{
public:
    void LoadConfig();

    void OnLevelChange();
    void OnLevelLoaded();
    void OnLoadGame();

    void OnSaveLoaded();

    void Update();

    SSpotData* GetSpot(u32 id);

    void MakeSpotAlive(u32 id, SSpotData* data);
    void MakeSpotNonExist(u32 id, SSpotData* data);
    void SetSpotState(u32 id, SpotState state);
    void SetSpotStatus(u32 id, SpotStatus status);
    CSE_Abstract* SpawnNPC(const shared_str& section, const Fvector& position);
    bool ReleaseNPC(const u32 id);


    void ClearRuntimeData();
    void ClearPersistentData();
    void RebuildNpcRegistry();

    void save(IWriter& writer);
    void load(IReader& reader);

    void OnNpcDeath(u32 id);

private:
    xr_map<u32, SSpotData> m_spots;
    //Быстрый поиск нужного спота путем айди нпс - айди спота
    xr_map<u16, u32> m_npcs;

    shared_str m_current_level;
    bool m_enabled = false;
    bool m_initialized = false;
    bool m_on_delete = false;

    u32 m_fCombatSpotsTimer;
    
    CHUDIcons hud_icons;

};

 extern CSpotManager g_CSpotManager;