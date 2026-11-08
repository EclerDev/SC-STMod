#pragma once

#pragma once
#include "inventory_item.h"
#include "Artifact.h"
#include "ui/UIDialogWnd.h"
#include "ui/UIScrollBar.h"
#include "ui/UIScrollView.h"
#include "SC_UI.h"

class CPhysicItem;
class CEntityAlive;

#include <sstream>
#include <iomanip>
#include <xrstring.h>

template <typename T>
shared_str ValueToString(const T& value)
{
    std::ostringstream stream;

    if constexpr (std::is_floating_point_v<T>)
        stream << std::setprecision(6) << std::defaultfloat;

    stream << value;

    return shared_str(stream.str().c_str());
}


// Отвечает за хранение артефактов
class CSCContainerCont
{
private:
public:
    xr_vector<CArtefact*> m_arts;
    int GetSize() { return m_arts.size(); }
    void AddArtefact(CArtefact* art) { m_arts.push_back(art); }
    void RemoveArtefact(CArtefact* art) {};
    void RemoveAllArtefact() { m_arts.clear(); }
};



class CSCItemContainer : public CInventoryItem
{
private:
    typedef CInventoryItem inherited;

private:
    CPhysicItem* m_physic_item;

public:
    CSCItemContainer();
    virtual ~CSCItemContainer() {};
    virtual DLL_Pure* _construct();
    virtual CSCItemContainer* cast_container_item() { return this; }

    virtual void Load(LPCSTR section);
    virtual bool Useful() const;

    virtual BOOL net_Spawn(CSE_Abstract* DC);

    virtual void OnH_B_Independent(bool just_before_destroy);
    virtual void UseBy(CEntityAlive* npc) {};

protected:
    // CSCContainerCont* m_arts;
    // CUIItemContainer* main_ui;
    // CUIItemContainerInfo* item_frame_info_ui;
    // CUIItemSmallInfo* item_small_frame_info_ui;

protected:
    int m_afSlots;
    float m_fInsideDefence;
    float m_fEffective;

    // Защита от отдельных заражений
    CItemInfection item_infection;
    CItemChar item_char;

public:
    float* GetInfection(EInfection type) { return item_infection.GetInfection(type); };
    float* GetChar(EItemChar type) { return item_char.GetChar(type); };

    float GetContainerDefence() { return m_fInsideDefence; };
    float GetContainerEffective() { return m_fEffective; };
    int GetContainerArtSlots() { return m_afSlots; };
};


class CContainerItemObject : public CSCItemContainer, public CPhysicItem
{
public:
    CContainerItemObject();
    virtual ~CContainerItemObject();
    virtual DLL_Pure* _construct();

public:
    virtual CPhysicsShellHolder* cast_physics_shell_holder() { return this; }
    virtual CInventoryItem* cast_inventory_item() { return this; }
    virtual CAttachableItem* cast_attachable_item() { return this; }
    virtual CWeapon* cast_weapon() { return 0; }
    virtual CFoodItem* cast_food_item() { return 0; }
    virtual CMissile* cast_missile() { return 0; }
    virtual CHudItem* cast_hud_item() { return 0; }
    virtual CWeaponAmmo* cast_weapon_ammo() { return 0; }
    virtual CGameObject* cast_game_object() { return this; }

public:
    virtual void Load(LPCSTR section);
    virtual void Hit(SHit* pHDS);

    virtual void OnH_B_Independent(bool just_before_destroy);
    virtual void OnH_A_Independent();
    virtual void OnH_B_Chield();
    virtual void OnH_A_Chield();
    virtual void UpdateCL();
    virtual void OnEvent(NET_Packet& P, u16 type);
    virtual BOOL net_Spawn(CSE_Abstract* DC);
    virtual void net_Destroy();
    virtual void net_Export(CSE_Abstract* E);
    virtual void save(NET_Packet& output_packet);
    virtual void load(IReader& input_packet);
    virtual BOOL net_SaveRelevant() { return TRUE; }
    virtual void renderable_Render(u32 context_id, IRenderable* root) override;
    virtual void reload(LPCSTR section);
    virtual void reinit();
    virtual void activate_physic_shell();
    virtual void on_activate_physic_shell();

public:
    virtual u32 ef_weapon_type() const;
    virtual bool Useful() const;
};




// Большой фрейм с полной информацией об предмете и его характектеристиках
class CUIItemContainerInfo : public CUIDialogWnd
{
private:
    typedef CUIDialogWnd inherited;

public:
    CUIItemContainerInfo();
    virtual ~CUIItemContainerInfo();

    virtual void Init();

    void SetCommonInfo(CSCItemContainer* item);
    void ClearCommonInfo();

    void SetContainerInfo(CSCItemContainer* item);
    void ClearContainerInfo();

    void SetInfectionInfo(CSCItemContainer* item);
    void ClearInfectionInfo();

    void SetFinalCharInfo(CSCItemContainer* item);
    void ClearFinalCharInfo();


   // virtual void SendMessage(CUIWindow* pWnd, u16 msg, void* pData) {};
   // virtual bool OnMouse(float x, float y, EUIMessages mouse_action) {};
    //virtual bool Onkeyboard(int dik, EUIMessages keyboard_action) {};

    //virtual void Update() override {};
    virtual void Draw() override;

    virtual void Show(CSCItemContainer* item);
    virtual void Hide() override;



protected:
    //#+# Пока инитим чисто в тупую статики
    CUIStatic UI_back;
    CUIStatic UI_item_name;

    CUIScrollView* m_stats_scroll = nullptr;
    CUIStatic m_stats_scroll_back;

    CUIItemInfoMainGroup UI_main_group;
    //CUIItemInfoCommonGroup* UI_common_info_group = nullptr;

    CUIWindow UI_small_char_info;

    CSCContainerCont* m_arts = nullptr;
    //CSCItemContainer* m_container = nullptr;
    CActor* owner = nullptr;

protected:
    bool m_need_reinit;
    bool m_need_update_scroll;
    void ClearAllInfo() {};

    bool OnButtonClick(CUI3tButton* btn) {};
    bool OnStaticFocus(CUIStatic* pStatic) {};
    bool OnStaticFocusLost(CUIStatic* pStatic) {};


    void PrepareSmallCharInfo() {};

    void ShowCharInfo() { UI_small_char_info.Show(true); }
    void HideCharInfo() { UI_small_char_info.Show(false); }
};


// Маленький фрейм, который высвечивается при наведении на предмет. Оба классса бы потом вынести отсюда, потому что они нужны будут везде
class CUIItemSmallInfo;

// Отвечает за уи контейнера
/* class CUIItemContainer : public CUIDialogWnd
{
private:
    typedef CUIDialogWnd inherited;

public:
    CUIItemContainer();
    virtual ~CUIItemContainer();

    virtual void Init();

    virtual bool StopAnyMove();

    virtual void SendMessage(CUIWindow* pwnd, s16 msg, void* pData);
    virtual bool OnMouse(float x, float y, EUIMessages mouse_action);
    virtual bool OnKeyboard(int dik, EUIMessages keyboard_action);

    virtual void Update();
    virtual void Draw();

    virtual void Show();
    virtual void Hide();

    void HideSlotHighlight();
    void ShowSlotHighlight();

    void HideInventoryArts();
    void ShowInventoryArts();

protected:
    enum eContainerSound
    {

    };


    void PlaySnd(eContainerSound a);
};
*/

enum eContainerType
{
    eCompactCont,
    eStandartCont,
    eVmestCont,
    eBigCont,
    eLightBag,
    eStandBag,
    eHeavyBag
};

