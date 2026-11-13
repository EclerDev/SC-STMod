#include "stdafx.h"
#include "CSpotManager.h"
#include <cstring>
#include "game_sv_single.h"
#include "alife_simulator.h"
#include "Level.h"
#include "ai_space.h"
#include "xrServer_Objects.h"
#include "alife_object_registry.h"
#include <algorithm>
#include "HUDIcons.h"

CSpotManager g_CSpotManager;

void CSpotManager::LoadConfig()
{
    string_path config_path;
    FS.update_path(config_path, "$game_config$", "misc\\spots.ltx");

    CInifile ini(config_path, TRUE);

    //Msg("CSpotManager::LoadConfig this=%p, before clear=%u", this, m_spots.size());

    // LoadConfig полностью перечитывает конфигурацию.
    m_spots.clear();

    for (const auto& [name, section] : ini.sections_ordered())
    {
        LPCSTR section_name = name.c_str();

        if (std::strncmp(section_name, "spot_", 5) != 0)
            continue;

        SSpotData data;

        data.section = section_name;
        data.level_name = section->r_string("level");
        data.position = section->r_fvector3("position");
        data.radius = section->r_float("radius");
        data.maximum_npc = section->r_u32("maximum_npc");
        data.npc_section = section->r_string("npc_section");
        data.reload_time = section->r_u32("reload_time");

        const u32 id = section->r_u32("id");

        const auto [it, inserted] = m_spots.emplace(id, std::move(data));

        //if (!inserted)
           // Msg("! CSpotManager: Duplicate spot ID %u", id);
    }

    //Msg("CSpotManager::LoadConfig this=%p, after load=%u", this, m_spots.size());
}



CSE_Abstract* CSpotManager::SpawnNPC(const shared_str& section, const Fvector& position)
{
    if (!Level().Server)
    {
        Msg("! CSpotManager::SpawnNPC: Level server is not ready");
        return nullptr;
    }

    auto tpGame = smart_cast<game_sv_Single*>(Level().Server->game);

    if (!tpGame)
    {
        Msg("! CSpotManager::SpawnNPC: Game is not ready");
        return nullptr;
    }

    if (!ai().get_level_graph() || !ai().get_cross_table())
    {
        Msg("! CSpotManager::SpawnNPC: AI graphs are not ready");
        return nullptr;
    }

    const u32 level_vertex_id = ai().level_graph().vertex_id(u32(-1), position);

    if (!ai().level_graph().valid_vertex_id(level_vertex_id))
    {
        Msg("! CSpotManager::SpawnNPC: Invalid level vertex %u", level_vertex_id);
        return nullptr;
    }

    const GameGraph::_GRAPH_ID game_vertex_id = ai().cross_table().vertex(level_vertex_id).game_vertex_id();

    CSE_Abstract* spawned = tpGame->alife().spawn_item(section.c_str(), position, level_vertex_id, game_vertex_id, u16(-1));

    if (!spawned)
    {
        Msg("! CSpotManager::SpawnNPC: Failed to spawn '%s'", section.c_str());
        return nullptr;
    }

    //Msg("CSpotManager::SpawnNPC: Spawned '%s', id=%u", section.c_str(), spawned->ID);

    return spawned;
}



void CSpotManager::OnLevelLoaded()
{
    m_current_level = Level().name();
    m_enabled = true;

    hud_icons.CreateCstmStatic(IconStaticClass::eSpot);

    for (const auto& [id, data] : m_spots)
    {
        if (data.spot_state != SpotState::eNone)
        {
            hud_icons.CreateCUIStatic(IconStaticClass::eSpot, IconStaticType::eSpotZombie, id, data.position);
        }
    }

    Msg("CSpotManager::OnLevelLoaded: %s", m_current_level.c_str());
}

void CSpotManager::OnLoadGame() 
{
    m_enabled = false;
    
    hud_icons.ClearCstmStatics();
    ClearRuntimeData();
}

void CSpotManager::OnLevelChange()
{
    m_enabled = false;
    Msg("CSpotManager::OnLevelChange");
}

const u32 CombatSpotKD = 1000;

void CSpotManager::Update()
{
    if (!m_enabled)
    {
        OnLevelLoaded();
        return;
    }
      


    m_fCombatSpotsTimer += Device.dwTimeDelta;
    hud_icons.Update(Device.dwTimeDelta);


    if (m_fCombatSpotsTimer < CombatSpotKD)
        return;

    const u32 delta = m_fCombatSpotsTimer;
    m_fCombatSpotsTimer = 0;

    const Fvector actor_pos = Level().CurrentEntity()->Position();

    for (auto& [id, data] : m_spots)
    {
        if (m_current_level != data.level_name)
            continue;

        data.cd_time += delta;

        if (data.spot_state == SpotState::eNone)
        {
            if (data.reload_time <= data.cd_time)
            {
                data.cd_time = 0;
                MakeSpotAlive(id, &data);
            }

            continue;

        }

        const float distance_sqr = actor_pos.distance_to_sqr(data.position);

        //const float radius_sqr = data.radius * data.radius;

        if (data.npcs.size() <= 0)
        {
            MakeSpotNonExist(id, &data);
            continue;
        }

        if (data.spot_state == SpotState::eOffline)
        {
            if (distance_sqr <= 12)
                SetSpotState(id, SpotState::eOnline);
        }
        else if (data.spot_state == SpotState::eOnline)
        {
            if (distance_sqr >= 12)
                SetSpotState(id, SpotState::eOffline);
        }
    }

}

void CSpotManager::MakeSpotAlive(u32 id, SSpotData* data)
{ 
    if (!data)
    {
        Msg("CSpotManager::MakeSpotAlive. Data is null");
        return;
    }

    SetSpotState(id, SpotState::eOffline);
    SetSpotStatus(id, SpotStatus::eNormal);


    //#+# пока что отправляем чисто зомби, потом типы добавим в сам спот
    hud_icons.CreateCUIStatic(IconStaticClass::eSpot, IconStaticType::eSpotZombie, id, data->position);

    data->npcs.clear();

    for (u32 i = 0; i < data->maximum_npc; ++i)
    {
        const shared_str npc_section = data->npc_section;
        Fvector pos = data->position;

        pos.x = pos.x + CRandom().randF(-10.f, 10.f);
        pos.z = pos.z + CRandom().randF(-10.f, 10.f);


        CSE_Abstract* npc = SpawnNPC(npc_section, pos);
        if (!npc || npc->ID == u16(-1))
            continue;

       SSpotNpcData npc_data;
       npc_data.position = pos;
       npc_data.m_fHealth = 1.f;
       npc_data.section = npc_section;
       npc_data.id = npc->ID;

       data->npcs.push_back(std::move(npc_data));
       m_npcs.emplace(npc->ID, id);
    }

    Msg("CSpotManager::MakeSpotAlive: Spot %u, NPCs %u/%u", id, data->npcs.size(), data->maximum_npc);

}

void CSpotManager::MakeSpotNonExist(u32 id, SSpotData* data) 
{ 
    SetSpotState(id, SpotState::eNone);
    SetSpotStatus(id, SpotStatus::eNormal);

    hud_icons.DeleteCUIStatic(IconStaticClass::eSpot, id);

    data->npcs.clear();
    data->cd_time = 0;
}

void CSpotManager::SetSpotState(u32 id, SpotState state)
{
    SSpotData* data = GetSpot(id);
    data->spot_state = state;
}

void CSpotManager::SetSpotStatus(u32 id, SpotStatus status)
{
    SSpotData* data = GetSpot(id);
    data->spot_status = status;
}

void CSpotManager::OnNpcDeath(u32 npc_id)
{
    auto it = m_npcs.find(npc_id);

    if (it == m_npcs.end())
        return;

    const u32 spot_id = it->second;
    SSpotData* spot_data = GetSpot(spot_id);

    if (spot_data)
    {
        auto& npcs = spot_data->npcs;

        npcs.erase(std::remove_if(npcs.begin(), npcs.end(), [npc_id](const SSpotNpcData& npc) { return npc.id == npc_id; }), npcs.end());
    }

    m_npcs.erase(it);
}

void CSpotManager::ClearRuntimeData() 
{ 
    Msg("CSpotManager::ClearRunTimeData: Resetting %u spots", m_spots.size()); 
    for (auto& [id, data] : m_spots)
    {
        data.spot_state = SpotState::eNone;
        data.spot_status = SpotStatus::eNormal;
        data.cd_time = 0;
        data.npcs.clear();
    }
    m_current_level = "none";
    m_enabled = false;
    m_fCombatSpotsTimer = 0;
}

SSpotData* CSpotManager::GetSpot(u32 id)
{
    auto it = m_spots.find(id);

    if (it == m_spots.end())
        return nullptr;

    return &it->second;
}


void CSpotManager::load(IReader& reader)
{
    Msg("CSpotManager::Load: Start. this=%p, spots=%u", this, m_spots.size());

    // Сбрасываем runtime-данные, но не конфигурацию и текущую локацию.
    for (auto& [id, data] : m_spots)
    {
        data.spot_state = SpotState::eNone;
        data.spot_status = SpotStatus::eNormal;
        data.cd_time = 0;
        data.npcs.clear();
    }

    IReader* chunk = reader.open_chunk(SPOT_MANAGER_CHUNK_DATA);

    if (!chunk)
    {
        Msg("! CSpotManager::Load: No saved data found");
        return;
    }

    if (chunk->elapsed() < sizeof(u32) * 2)
    {
        Msg("! CSpotManager::Load: Invalid chunk size");
        chunk->close();
        return;
    }

    const u32 format = chunk->r_u32();

    if (format != 1)
    {
        Msg("! CSpotManager::Load: Unsupported format %u", format);
        chunk->close();
        return;
    }

    const u32 spot_count = chunk->r_u32();

    u32 loaded_spots = 0;
    u32 loaded_npcs = 0;

    for (u32 i = 0; i < spot_count; ++i)
    {
        constexpr u32 spot_header_size = sizeof(u32) + sizeof(u8) * 2 + sizeof(u32);

        if (chunk->elapsed() < spot_header_size)
        {
            Msg("! CSpotManager::Load: Unexpected end of chunk at spot %u", i);
            break;
        }

        const u32 id = chunk->r_u32();
        const u8 spot_state = chunk->r_u8();
        const u8 spot_status = chunk->r_u8();
        const u32 npc_count = chunk->r_u32();

        auto it = m_spots.find(id);
        const bool spot_exists = it != m_spots.end();

        if (!spot_exists)
        {
            Msg("! CSpotManager::Load: Spot with ID %u not found in config", id);
        }
        else
        {
            SSpotData& data = it->second;

            if (spot_state <= static_cast<u8>(SpotState::eOnline))
                data.spot_state = static_cast<SpotState>(spot_state);
            else
                Msg("! CSpotManager::Load: Invalid state %u for spot %u", spot_state, id);

            if (spot_status <= static_cast<u8>(SpotStatus::eCombat))
                data.spot_status = static_cast<SpotStatus>(spot_status);
            else
                Msg("! CSpotManager::Load: Invalid status %u for spot %u", spot_status, id);

            ++loaded_spots;
        }

        // Минимальный размер одной записи:
        // строка с завершающим нулём + health + position.
        constexpr u32 min_npc_size = sizeof(u16) + sizeof(char) + sizeof(float) + sizeof(Fvector);

        if (npc_count > chunk->elapsed() / min_npc_size)
        {
            Msg("! CSpotManager::Load: Invalid NPC count %u for spot %u", npc_count, id);
            break;
        }

        for (u32 j = 0; j < npc_count; ++j)
        {
            u16 npc_id = chunk->r_u16();
            shared_str npc_section;
            chunk->r_stringZ(npc_section);

            if (chunk->elapsed() < sizeof(float) + sizeof(Fvector))
            {
                Msg("! CSpotManager::Load: Unexpected end of NPC data");
                break;
            }

            const float health = chunk->r_float();

            Fvector position;
            chunk->r_fvector3(position);

            ++loaded_npcs;

            // Если спота нет в конфиге, просто пропускаем его NPC.
            if (!spot_exists)
                continue;

            SSpotNpcData npc_data;
            npc_data.position = position;
            npc_data.m_fHealth = health;
            npc_data.section = npc_section;
            npc_data.id = npc_id;

            it->second.npcs.push_back(std::move(npc_data));
        }
    }

    chunk->close();

    Msg("CSpotManager::Load: Loaded %u spots and %u NPCs", loaded_spots, loaded_npcs);
}

void CSpotManager::save(IWriter& writer)
{
    Msg("CSpotManager::Save: this=%p, spots=%u", this, m_spots.size());

    writer.open_chunk(SPOT_MANAGER_CHUNK_DATA);

    writer.w_u32(1); // Format version
    writer.w_u32(static_cast<u32>(m_spots.size()));

    u32 saved_spots = 0;
    u32 saved_npcs = 0;

    for (const auto& [id, data] : m_spots)
    {
        writer.w_u32(id);
        writer.w_u8(static_cast<u8>(data.spot_state));
        writer.w_u8(static_cast<u8>(data.spot_status));
        writer.w_u32(static_cast<u32>(data.npcs.size()));

        for (const auto& npc : data.npcs)
        {
            writer.w_u16(npc.id);
            writer.w_stringZ(*npc.section);
            writer.w_float(npc.m_fHealth);
            writer.w_fvector3(npc.position);

            ++saved_npcs;
        }

        ++saved_spots;
    }

    writer.close_chunk();

    Msg("CSpotManager::Save: Saved %u spots and %u NPCs", saved_spots, saved_npcs);
}


void CSpotManager::RebuildNpcRegistry()
{
    m_npcs.clear();

    if (!ai().get_alife())
    {
        Msg("! CSpotManager::RebuildNpcRegistry: ALife is not ready");
        return;
    }

    auto& alife = ai().alife();

    for (auto& [spot_id, data] : m_spots)
    {
        for (auto it = data.npcs.begin(); it != data.npcs.end();)
        {
            if (it->id == u16(-1))
            {
                ++it;
                continue;
            }

            CSE_ALifeDynamicObject* object = alife.objects().object(it->id, true);

            if (!object)
            {
                Msg("! CSpotManager::RebuildNpcRegistry: NPC %u from spot %u not found in ALife", it->id, spot_id);

                it = data.npcs.erase(it);
                continue;
            }

            const auto [map_it, inserted] = m_npcs.emplace(it->id, spot_id);

            if (!inserted)
            {
                Msg("! CSpotManager::RebuildNpcRegistry: Duplicate NPC ID %u", it->id);

                it = data.npcs.erase(it);
                continue;
            }

            ++it;
        }
    }
}