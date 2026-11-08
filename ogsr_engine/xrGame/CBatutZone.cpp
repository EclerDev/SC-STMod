#include "stdafx.h"
#include "CBatutZone.h"
#include "hudmanager.h"
#include "ParticlesObject.h"
#include "level.h"
#include "physicsshellholder.h"
#include "Actor.h"

CBatutZone::CBatutZone(void)
{
    m_dwDeltaTime = 0;
    m_fHitImpulseScale = 1.f;

    m_bLastBlowoutUpdate = false;

    m_vBatutDir = Fvector().set(0.f, 1.f, 0.f);
    m_fBatutPower = 1.f;
}

CBatutZone::~CBatutZone(void) {}

void CBatutZone::Load(LPCSTR section)
{
    inherited::Load(section);

    m_vBatutDir = pSettings->r_fvector3(section, "batut_dir");
    m_fBatutPower = pSettings->r_float(section, "batut_power");
}

void CBatutZone::Postprocess(f32 /**val/**/) {}

bool CBatutZone::BlowoutState()
{
    bool result = inherited::BlowoutState();
    if (!result)
    {
        m_bLastBlowoutUpdate = false;
        UpdateBlowout();
    }
    else if (!m_bLastBlowoutUpdate)
    {
        m_bLastBlowoutUpdate = true;
        UpdateBlowout();
    }

    return result;
}

void CBatutZone::Affect(SZoneObjectInfo* O)
{
    CPhysicsShellHolder* pGameObject = smart_cast<CPhysicsShellHolder*>(O->object);
    if (!pGameObject)
        return;

    if (O->zone_ignore)
        return;

    Fvector P;
    XFORM().transform_tiny(P, CFORM()->getSphere().P);

    Fvector hit_dir;
    hit_dir.set(::Random.randF(-.5f, .5f), ::Random.randF(.0f, 1.f), ::Random.randF(-.5f, .5f));
    hit_dir.normalize();

    VERIFY(!pGameObject->getDestroy());

    float dist = pGameObject->Position().distance_to(P) - pGameObject->Radius();
    float power = Power(dist > 0.f ? dist : 0.f);

    // статистика по объекту
    O->hit_num++;

    if (power > 0.01f)
    {
        m_dwDeltaTime = 0;
        // Msg("Tryng to batut");
        CActor* actor = smart_cast<CActor*>(pGameObject);
        if (!actor)
            return;
        // Msg("There is no actor");

        actor->ApplyForceOnActor(m_vBatutDir, m_fBatutPower);
        // Msg("Appling");
    }
}
