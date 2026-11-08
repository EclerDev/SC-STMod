#pragma once
#include "customzone.h"
#include "script_export_space.h"
class CBatutZone : public CCustomZone
{
private:
    typedef CCustomZone inherited;

public:
    CBatutZone(void);
    virtual ~CBatutZone(void);

    virtual void Load(LPCSTR section);
    // virtual BOOL net_Spawn(CSE_Abstract* DC);
    // virtual void net_Destroy(CSE_Abstract* E);
    virtual void Postprocess(f32 val);

    virtual void Affect(SZoneObjectInfo* O);

    Fvector m_vBatutDir;
    float m_fBatutPower;

    Fvector GetBatutDir() { return m_vBatutDir; }
    void SetBatutDir(Fvector& dir) { m_vBatutDir.set(dir); }

    float GetBatutPower() { return m_fBatutPower; }
    void SetBatutPower(float power) { m_fBatutPower = power; }

protected:
    virtual bool BlowoutState();
    // для того чтобы blowout обновился один раз
    // после того как зона перключилась в другое состояние
    bool m_bLastBlowoutUpdate;
    DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CBatutZone)
#undef script_type_list
#define script_type_list save_type_list(CBatutZone)