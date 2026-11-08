#pragma once
#include "ui/UIWindow.h"
#include "ui/UIXmlInit.h"
#include "ui/UI3tButton.h"
#include "ui/UIButton.h"
#include "Artifact.h"

enum EItemInfoGroup
{
    EItemInfoCommonGroup,
    EItemInfoContainerGroup,
    EItemInfoContainerInfectionGroup,
    EItemInfoContainerCharGroup,
    EItemInfoGroupMax
};

enum EItemCommonInfo
{
    EitemInfoCommonName,
    EItemInfoCommonRank,
    EItemInfoCommonClass,
    EItemInfoCommonOwner,
    EItemInfoCommonWeight,
    EItemCommonInfoMax
};

enum EItemInfoContainer
{
    EItemInfoContainerType,
    EItemInfoContainerDefence,
    EItemInfoContainerEffect,
    EItemInfoContainerSlots,
    EItemInfoContainerMax
};


enum EInfection
{
    ERadiation_inf,
    EBiological_inf,
    EPsi_inf,
    ETemperature_inf,
    ECold_inf,
    EInfectionMax
};

enum EItemChar
{
    EJivuchest,
    ETemperature,
    EGorenie,
    EBlastDefence,
    EPower,
    EWeight,
    EBioV,
    EPsidefence,
    EReactOnElec,
    EHealthV,
    EPowerV,
    EBulletDefence,
    EWoundV,
    EWoundDefence,
    ERadiationV,
    ERunSpeed,
    EMoveSpeed,
    EFireDefence,
    ERecoil,
    EMutantDefence,
    EReactOnMutant,
    EElectroDefence,
    EHealPower,
    EBioDefence,
    EReactOnChem,
    ESkoykost,
    EPerioudHeal,
    EReactOnFire,
    ETempDefence,
    EPsiDefence,
    ERadiationDefence,
    EPokachivanie,
    EItemCharMax
};


namespace ItemChar
{
struct SItemCharValue
{
    float value = 0.f;
    bool exist = false;
};

typedef svector<SItemCharValue, EItemChar::EItemCharMax> ItemCharVector;
typedef svector<SItemCharValue, EInfection::EInfectionMax> ItemInfectionVector;
}; // namespace ItemChar

class CItemChar
{
public:
    CItemChar();
    virtual ~CItemChar() {};

    virtual void LoadChar(LPCSTR section, CInifile* ini_file);

    float* GetChar(EItemChar char_type);
    virtual void AffectChar(float value, EItemChar char_type) {};
    bool IsCharExist();


protected:
    ItemChar::ItemCharVector m_char_types;

public:
    const ItemChar::ItemCharVector& characteristc() const { return m_char_types; }
    virtual CItemChar* cast_char_types() { return this; }
};

class CItemInfection
{
public:
    CItemInfection();
    virtual ~CItemInfection() {};

    virtual void LoadInfection(LPCSTR section, CInifile* ini_file);
    bool IsInfectionExist();

    float* GetInfection(EInfection infection_type);

protected:
    ItemChar::ItemInfectionVector m_infection_types;

public:
    const ItemChar::ItemInfectionVector& infection() const { return m_infection_types; }
    CItemInfection* cast_infection_types() { return this; }
};



//Окно с названием и параметром
class CUIItemInfoRow : public CUIWindow
{
    using inherited = CUIWindow;

public:
    CUIItemInfoRow();
    virtual ~CUIItemInfoRow() = default;

    void Init(CUIXml& xml, LPCSTR path, int index = 0);

    void SetName(LPCSTR name);
    void SetValue(LPCSTR value);

    void SetNameColor(u32 color);
    void SetValueColor(u32 color);

    void EnableHighlight() { m_highlight = true; };
    void DisableHighlight() { m_highlight = false; };

    void SetHightLightColor(u32 color);


private:
    CUIStatic* m_name = nullptr;
    CUIStatic* m_value = nullptr;
    CUIStatic* m_back = nullptr;

    u32 highlight_color;
    bool m_highlight; 
};

//Окно с только навзванием под вопросиком


//Разделительная линия так как использоваться будет часто
class CUIItemInfoSeparator : public CUIWindow
{
    using inherited = CUIWindow;

public:
    CUIItemInfoSeparator() {};
    virtual ~CUIItemInfoSeparator() = default;

    void Init(CUIXml& xml, LPCSTR path, int index = 0);

    void CentrilizeLine();

private:
    CUIStatic* m_line = nullptr;

};

//Кнопка-надпись. Как например "база для получения"
class CUIItemInfoRowButton : public CUIItemInfoRow
{
    using inherited = CUIItemInfoRow;

public:
    CUIItemInfoRowButton() {};
    virtual ~CUIItemInfoRowButton() {};

    void Init(CUIXml& xml, LPCSTR path, int index = 0);

private:
    CUIStatic* m_line = nullptr;
};


template <typename Tkey>
class CUIItemInfoGroup : public CUIWindow
{
    using inherited = CUIWindow;



public:
    CUIItemInfoGroup() = default;
    virtual ~CUIItemInfoGroup() { ClearAllRows(); };
    void Init(CUIXml& xml, LPCSTR path, int index)
    {
        CUIXmlInit xml_init;
        xml_init.InitWindow(xml, path, index, this);
    };

    CUIItemInfoRow* AddRow(CUIXml& xml, LPCSTR path, Tkey type, LPCSTR name, LPCSTR value) 
    {
        auto* wnd = xr_new<CUIItemInfoRow>();
        wnd->SetAutoDelete(true);

        wnd->Init(xml, path);
        wnd->SetName(name);
        wnd->SetValue(value);

        wnd->SetWndPos(Fvector2(0.0f, m_next_y));

        AttachChild(wnd);

        m_next_y += wnd->GetWndSize().y + 2.0f;
        SetWndSize(Fvector2(GetWndSize().x, m_next_y));

        m_elements.push_back(wnd);

        R_ASSERT(m_elements_map.find(type) == m_elements_map.end());
        m_elements_map.insert(std::make_pair(type, wnd));

        return wnd;
    };
    CUIItemInfoSeparator* AddSeparator(CUIXml& xml, LPCSTR path, bool centr) 
    {
        auto* wnd = xr_new<CUIItemInfoSeparator>();
        wnd->SetAutoDelete(true);

        wnd->Init(xml, path);

        if (centr)
            wnd->CentrilizeLine();

        wnd->SetWndPos(Fvector2(0.0f, m_next_y));

        AttachChild(wnd);
        m_elements.push_back(wnd);

        m_next_y += wnd->GetWndSize().y + 2.0f;

        SetWndSize(Fvector2(GetWndSize().x, m_next_y));

        return wnd;
    };
    CUIItemInfoRow* GetRow(Tkey type) 
    { 
        auto it = m_elements_map.find(type);
        return it != m_elements_map.end() ? it->second : nullptr;
    };

    const int GetGroupSize() const { return m_elements.size(); };

    void ShowRow(Tkey type, bool show)
        {
            if (auto* row = GetRow(type))
            {
                row->Show(show);
                RecalculateLayout();
            }
        }

    void RemoveRow(Tkey type)
    {
        auto it = m_elements_map.find(type);
        if (it == m_elements_map.end())
            return;

        CUIItemInfoRow* row = it->second;

        m_elements_map.erase(it);
        m_elements.erase(std::remove(m_elements.begin(), m_elements.end(), row), m_elements.end());

        DetachChild(row);

        RecalculateLayout();
    }

    void ClearAllRows()
    {
        for (auto& element : m_elements)
        {
            DetachChild(element);
        }
        m_elements.clear();
        m_elements_map.clear();
        m_next_y = 0.0f;
        SetWndSize(Fvector2(GetWndSize().x, 0.0f));
    }

private:

    void RecalculateLayout()
        {
            m_next_y = 0.0f;

            for (auto* element : m_elements)
            {
                if (!element->IsShown())
                    continue;

                element->SetWndPos(Fvector2(0.0f, m_next_y));
                m_next_y += element->GetWndSize().y + 2.0f;
            }

            SetWndSize(Fvector2(GetWndSize().x, m_next_y));
        }

    xr_vector<CUIWindow*> m_elements;
    xr_map<Tkey, CUIItemInfoRow*> m_elements_map;
    float m_next_y = 0.0f;
};

using CUIItemInfoCommonGroup = CUIItemInfoGroup<EItemCommonInfo>;
using CUIIteminfoContainerGroup = CUIItemInfoGroup<EItemInfoContainer>;
using CUIItemInfoInfectionGroup = CUIItemInfoGroup<EInfection>;
using CUIItemInfoCharGroup = CUIItemInfoGroup<EItemChar>;


class CUIItemInfoMainGroup
{
public:
    void AddGroup(EItemInfoGroup group, CUIWindow* wnd) { m_groups[group] = wnd; };
    void RemoveGroup(EItemInfoGroup group) { m_groups.erase(group); };
    void ClearGroups() { m_groups.clear(); };

    CUIWindow* GetGroup(EItemInfoGroup group)
        {
            auto it = m_groups.find(group);
            return it != m_groups.end() ? it->second : nullptr;
        }



private:
    xr_map<EItemInfoGroup, CUIWindow*> m_groups;
};

/*
class CUIItemContainerArtCell : public CUIWindow
{
private:
    using inherited = CUIWindow;

public:
    CUIItemContainerArtCell();
    virtual ~CUIItemContainerArtCell();

    virtual void Init();
    
    void SetArtefact(CArtefact* art) { m_item = art; };
    CArtefact* GetArtefact() { return m_item; }

    void SetCornerColor(u32 color) { UI_corner.SetColor(color); }
    void SetBackColor(u32 color) { UI_color_back.SetColor(color); }

    void SetItemIcon(LPCSTR tex_name) { UI_item_icon.InitTexture(tex_name); };




protected:
    CUIStatic UI_back;
    CUIStatic UI_corner;
    CUIStatic UI_color_back;
    CUIStatic UI_item_icon;

    CUIButton UI_main_btn;
    CUIButton UI_up_btn;

    CArtefact* m_item = nullptr;

};

//Надо с названием будет что-то подумать. Это ячейки, которые выскакивают при замене артефакта
class CUIitemContainerCellOffer : public CUIWindow
{
private:
    using inherited = CUIWindow;

public:
    CUIitemContainerCellOffer();
    virtual ~CUIitemContainerCellOffer();

    virtual void Init();

    void SetNullCell();

    void PrepareCell(CArtefact* art);
    CArtefact* GetArtefact() { return m_item; }

protected:
    CUIStatic UI_back;
    CUIStatic UI_corner;
    CUIStatic UI_color_back;
    CUIStatic UI_item_icon;

    CUIButton UI_btn;



    CArtefact* m_item = nullptr;
};


class CUIitemContainerCellStruct
{
public:
    void AddElement(CArtefact* item);
    void GetElement();
    void ClearElement();
    void AddNullElement(); // пустая ячейка, которая отвечает за снятие артефакта, если такой есть в слоте. Если в слоте нет ничего, то и элемента этого нет
    void Clear();


    //Пока надо подумать как реализовать
    void HideNullElement();
    void ShowNullElement();

    void SortElements();

protected:
    int m_iMax_row;
    int m_iMax_row_elem;
    int m_iNext_row;
    int m_iNext_line;

    xr_vector<CUIitemContainerCellOffer*> m_elements;
};
*/