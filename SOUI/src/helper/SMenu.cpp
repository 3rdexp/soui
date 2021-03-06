#include "souistd.h"
#include "SApp.h"
#include "helper/SMenu.h"
#include "helper/mybuffer.h"
#include "gdialpha.h"

namespace SOUI
{

SMenuAttr::SMenuAttr()
    :m_pItemSkin(GETBUILTINSKIN(SKIN_SYS_MENU_SKIN))
    ,m_pSepSkin(GETBUILTINSKIN(SKIN_SYS_MENU_SEP))
    ,m_pCheckSkin(GETBUILTINSKIN(SKIN_SYS_MENU_CHECK))
    ,m_pIconSkin(NULL)
    ,m_hFont(0)
    ,m_nItemHei(0)
    ,m_nIconMargin(2)
    ,m_nTextMargin(5)
    ,m_szIcon(CX_ICON,CY_ICON)
{
    m_crTxtNormal=GetSysColor(COLOR_MENUTEXT)|0xff000000;
    m_crTxtSel=GetSysColor(COLOR_HIGHLIGHTTEXT)|0xff000000;
    m_crTxtGray=GetSysColor(COLOR_GRAYTEXT)|0xff000000;
}

void SMenuAttr::OnInitFinished( pugi::xml_node xmlNode )
{
    SASSERT(m_pItemSkin);
    if(m_nItemHei==0) m_nItemHei=m_pItemSkin->GetSkinSize().cy;
    if(!m_hFont) m_hFont=SFontPool::getSingleton().GetFont(FF_DEFAULTFONT);
}
//////////////////////////////////////////////////////////////////////////

SMenuODWnd::SMenuODWnd(HWND hMenuOwner):m_hMenuOwner(hMenuOwner)
{

}

void SMenuODWnd::OnInitMenu( HMENU menu )
{
    ::SendMessage(m_hMenuOwner,WM_INITMENU,(WPARAM)menu,0);
}

void SMenuODWnd::OnInitMenuPopup( HMENU menuPopup, UINT nIndex, BOOL bSysMenu )
{
    ::SendMessage(m_hMenuOwner,WM_INITMENUPOPUP,(WPARAM)menuPopup,MAKELPARAM(nIndex,bSysMenu));
}

void SMenuODWnd::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
    CRect rcItem=lpDrawItemStruct->rcItem;
    rcItem.MoveToXY(0,0);
    SMenuItemData *pdmmi=(SMenuItemData*)lpDrawItemStruct->itemData;

    HDC dc(lpDrawItemStruct->hDC);
    CAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT,rcItem.Width(),rcItem.Height());

    if(pdmmi)
    {
        MENUITEMINFO mii= {sizeof(MENUITEMINFO),MIIM_FTYPE,0};
        HMENU menuPopup=pdmmi->hMenu;
        GetMenuItemInfo(menuPopup,pdmmi->nID,FALSE,&mii);

        BOOL bDisabled = lpDrawItemStruct->itemState & ODS_GRAYED;
        BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
        BOOL bChecked = lpDrawItemStruct->itemState & ODS_CHECKED;
        BOOL bRadio = mii.fType&MFT_RADIOCHECK;
        m_pItemSkin->Draw(pRT,rcItem,bSelected?1:0);    //draw background

        //draw icon
        CRect rcIcon;
        rcIcon.left=rcItem.left+m_nIconMargin;
        rcIcon.right=rcIcon.left+m_szIcon.cx;
        rcIcon.top=rcItem.top+(rcItem.Height()-m_szIcon.cy)/2;
        rcIcon.bottom=rcIcon.top+m_szIcon.cy;
        if(bChecked)
        {
            if(m_pCheckSkin)
            {
                m_pCheckSkin->Draw(pRT,rcIcon,bRadio?1:0);
            }
        }
        else if(pdmmi->itemInfo.iIcon!=-1 && m_pIconSkin)
        {
            m_pIconSkin->Draw(pRT,rcIcon,pdmmi->itemInfo.iIcon);
        }
        rcItem.left=rcIcon.right+m_nIconMargin;

        //draw text
        CRect rcTxt=rcItem;
        rcTxt.DeflateRect(m_nTextMargin,0);

        COLORREF crOld=pRT->SetTextColor(bDisabled?m_crTxtGray:(bSelected?m_crTxtSel:m_crTxtNormal));


        CAutoRefPtr<IFont> oldFont;
        pRT->SelectObject(m_hFont,(IRenderObj**)&oldFont);
        pRT->DrawText(pdmmi->itemInfo.strText,pdmmi->itemInfo.strText.GetLength(),&rcTxt,DT_SINGLELINE|DT_VCENTER|DT_LEFT);
        pRT->SelectObject(oldFont);

        pRT->SetTextColor(crOld);

        if(bSelected && m_pItemSkin->GetStates()>2)
        {
            //draw select mask
            CRect rcItem=lpDrawItemStruct->rcItem;
            rcItem.MoveToXY(0,0);
            m_pItemSkin->Draw(pRT,rcItem,2);
        }
    }
    else  //if(strcmp("sep",pXmlItem->Value())==0)
    {
        m_pItemSkin->Draw(pRT,rcItem,0);    //draw back
        if(m_pIconSkin)
        {
            rcItem.left += m_pIconSkin->GetSkinSize().cx+m_nIconMargin*2;
        }

        if(m_pSepSkin)
            m_pSepSkin->Draw(pRT,&rcItem,0);
        else
        {
            CAutoRefPtr<IPen> pen1,pen2,oldPen;
            pRT->CreatePen(PS_SOLID,RGBA(196,196,196,255),1,&pen1);
            pRT->CreatePen(PS_SOLID,RGBA(255,255,255,255),1,&pen2);
            pRT->SelectObject(pen1,(IRenderObj**)&oldPen);
            POINT pts[2]={{rcItem.left,rcItem.top},{rcItem.right,rcItem.top}};
            pRT->DrawLines(pts,2);
            pRT->SelectObject(pen2);
            pts[0].y++,pts[1].y++;
            pRT->DrawLines(pts,2);
            pRT->SelectObject(oldPen);
        }
    }
    rcItem=lpDrawItemStruct->rcItem;
    
    HDC hmemdc=pRT->GetDC(0);
    BitBlt(dc,rcItem.left,rcItem.top,rcItem.Width(),rcItem.Height(),hmemdc,0,0,SRCCOPY);
    pRT->ReleaseDC(hmemdc);
}

void SMenuODWnd::MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
    if(lpMeasureItemStruct->CtlType != ODT_MENU) return;

    SMenuItemData *pdmmi=(SMenuItemData*)lpMeasureItemStruct->itemData;
    if(pdmmi)
    {
        //menu item
        lpMeasureItemStruct->itemHeight = m_nItemHei;
        lpMeasureItemStruct->itemWidth = m_szIcon.cx+m_nIconMargin*2;

        CAutoRefPtr<IRenderTarget> pRT;
        GETRENDERFACTORY->CreateRenderTarget(&pRT,0,0);
        CAutoRefPtr<IFont> oldFont;
        pRT->SelectObject(m_hFont,(IRenderObj**)&oldFont);
        SIZE szTxt;
        pRT->MeasureText(pdmmi->itemInfo.strText,pdmmi->itemInfo.strText.GetLength(),&szTxt);
        lpMeasureItemStruct->itemWidth += szTxt.cx+m_nTextMargin*2;
        pRT->SelectObject(oldFont);
    }
    else
    {
        // separator
        lpMeasureItemStruct->itemHeight = m_pSepSkin?m_pSepSkin->GetSkinSize().cy:3;
        lpMeasureItemStruct->itemWidth=0;
    }

}

void SMenuODWnd::OnMenuSelect( UINT nItemID, UINT nFlags, HMENU menu )
{
    ::SendMessage(m_hMenuOwner,WM_MENUSELECT,MAKEWPARAM(nItemID,nFlags),(LPARAM)menu);
}

LRESULT SMenuODWnd::OnMenuChar(UINT nChar, UINT nFlags, HMENU hMenu)
{
    wchar_t cChar = tolower(nChar);
    
    int nMenuItems = ::GetMenuItemCount(hMenu);
    for(int i=0;i<nMenuItems;i++)
    {
        MENUITEMINFO mi={sizeof(mi),0};
        mi.fMask = MIIM_DATA;
        ::GetMenuItemInfo(hMenu,i,TRUE,&mi);
        SMenuItemData * pItemData =(SMenuItemData*)mi.dwItemData;
        if(pItemData && pItemData->itemInfo.vHotKey == nChar)
        {
            return MAKELONG(i,MNC_EXECUTE);
        }
    }
    return MAKELONG(0,MNC_IGNORE);
}

//////////////////////////////////////////////////////////////////////////

SMenu::SMenu():m_pParent(NULL),m_hMenu(0)
{
}

SMenu::SMenu( const SMenu & src )
{
    m_pParent=src.m_pParent;
    m_hMenu=src.m_hMenu;
    m_menuAttr=src.m_menuAttr;    
}

SMenu::~SMenu(void)
{
    DestroyMenu();
}

BOOL SMenu::LoadMenu( LPCTSTR pszResName ,LPCTSTR pszType)
{
    if(::IsMenu(m_hMenu)) return FALSE;

    pugi::xml_document xmlDoc;
    if(!LOADXML(xmlDoc,pszResName,pszType)) return FALSE;

    pugi::xml_node xmlMenu=xmlDoc.child(L"menu");
    if(!xmlMenu)  return FALSE;

    return LoadMenu(xmlMenu);
}


BOOL SMenu::LoadMenu( pugi::xml_node xmlMenu )
{
    m_hMenu=CreatePopupMenu();
    if(!m_hMenu) return FALSE;

    m_menuAttr.InitFromXml(xmlMenu);
    SASSERT(m_menuAttr.m_pItemSkin);

    BuildMenu(m_hMenu,xmlMenu);

    return TRUE;
}

SMenu SMenu::GetSubMenu(int nPos)
{
    HMENU hSubMenu=::GetSubMenu(m_hMenu,nPos);
    SMenu ret;
    ret.m_pParent=this;
    ret.m_hMenu=hSubMenu;
    ret.m_menuAttr=m_menuAttr;
    return ret;
}

void SMenu::InitMenuItemData(SMenuItemInfo & itemInfo, const SStringW & strTextW)
{
    itemInfo.strText=S_CW2T(TR(strTextW,m_menuAttr.m_strTrCtx));

    //查找快捷键
    int iHotKey = strTextW.Find(L"&");
    if(iHotKey != -1 && iHotKey < strTextW.GetLength()-1)
    {
        itemInfo.vHotKey = tolower(strTextW[iHotKey+1]);
    }else
    {
        itemInfo.vHotKey = 0;
    }
}

BOOL SMenu::InsertMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem,LPCTSTR strText, int iIcon)
{
    nFlags|=MF_OWNERDRAW;
    if(nFlags&MF_SEPARATOR)
    {
        return ::InsertMenu(m_hMenu,nPosition,nFlags,(UINT_PTR)0,(LPCTSTR)NULL);
    }

    SMenuItemData *pMenuData=new SMenuItemData;
    pMenuData->hMenu=m_hMenu;
    pMenuData->itemInfo.iIcon=iIcon;
    InitMenuItemData(pMenuData->itemInfo,S_CT2W(strText));

    if(nFlags&MF_POPUP)
    {
        //插入子菜单，
        SMenu *pSubMenu=(SMenu*)(LPVOID)nIDNewItem;
        SASSERT(pSubMenu->m_pParent==NULL);
        pMenuData->nID=(UINT_PTR)pSubMenu->m_hMenu;
    }
    else
    {
        pMenuData->nID=nIDNewItem;
    }

    if(!::InsertMenu(m_hMenu,nPosition,nFlags,pMenuData->nID,(LPCTSTR)pMenuData))
    {
        delete pMenuData;
        return FALSE;
    }

    SMenu *pRootMenu=this;
    while(pRootMenu->m_pParent) pRootMenu=pRootMenu->m_pParent;
    //将分配的内存放到根菜单的内存节点列表中
    pRootMenu->m_arrDmmi.Add(pMenuData);

    if(nFlags&MF_POPUP)
    {
        //对子菜单还需要做数据迁移处理
        SMenu *pSubMenu=(SMenu*)(LPVOID)nIDNewItem;
        for(UINT i=0; i<pSubMenu->m_arrDmmi.GetCount(); i++)
            pRootMenu->m_arrDmmi.Add(pSubMenu->m_arrDmmi[i]);
        pSubMenu->m_arrDmmi.RemoveAll();
        pSubMenu->m_pParent=this;
    }
    return TRUE;
}

UINT SMenu::TrackPopupMenu(
    UINT uFlags,
    int x,
    int y,
    HWND hWnd,
    LPCRECT prcRect
)
{
    SASSERT(IsMenu(m_hMenu));

    SMenuODWnd menuOwner(hWnd);
    *(static_cast<SMenuAttr*>(&menuOwner))=m_menuAttr;
    menuOwner.Create(NULL,WS_POPUP,WS_EX_NOACTIVATE,0,0,0,0,NULL,NULL);
    UINT uNewFlags=uFlags|TPM_RETURNCMD;
    UINT uRet=::TrackPopupMenu(m_hMenu,uNewFlags,x,y,0,menuOwner.m_hWnd,prcRect);
    menuOwner.DestroyWindow();
    if(uRet && !(uFlags&TPM_RETURNCMD)) ::SendMessage(hWnd,WM_COMMAND,uRet,0);
    return uRet;
}

void SMenu::BuildMenu( HMENU menuPopup,pugi::xml_node xmlNode )
{
    pugi::xml_node xmlItem=xmlNode.first_child();

    while(xmlItem)
    {
        if(wcscmp(L"item",xmlItem.name())==0)
        {
            SMenuItemData *pdmmi=new SMenuItemData;
            pdmmi->hMenu=menuPopup;
            pdmmi->itemInfo.iIcon=xmlItem.attribute(L"icon").as_int(-1);
            SStringW strText = xmlItem.text().get();
            strText.TrimBlank();
            InitMenuItemData(pdmmi->itemInfo,strText);

            int nID=xmlItem.attribute(L"id").as_int(0);
            BOOL bCheck=xmlItem.attribute(L"check").as_bool(false);
            BOOL bRadio=xmlItem.attribute(L"radio").as_bool(false);
            BOOL bDisable=xmlItem.attribute(L"disable").as_bool(false);


            pugi::xml_writer_buff writer;
            xmlItem.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
            SStringW str(writer.buffer(),writer.size());

            pugi::xml_node xmlChild=xmlItem.first_child();
            while(xmlChild && xmlChild.type()==pugi::node_pcdata) xmlChild=xmlChild.next_sibling();


            if(!xmlChild)
            {
                pdmmi->nID=nID;
                UINT uFlag=MF_OWNERDRAW;
                if(bCheck) uFlag|=MF_CHECKED;
                if(bDisable) uFlag |= MF_GRAYED;
                if(bRadio) uFlag |= MFT_RADIOCHECK|MF_CHECKED;
                AppendMenu(menuPopup,uFlag,(UINT_PTR)pdmmi->nID,(LPCTSTR)pdmmi);
            }
            else
            {
                HMENU hSubMenu=::CreatePopupMenu();
                pdmmi->nID=(UINT_PTR)hSubMenu;
                UINT uFlag=MF_OWNERDRAW|MF_POPUP;
                if(bDisable) uFlag |= MF_GRAYED;
                AppendMenu(menuPopup,uFlag,(UINT_PTR)hSubMenu,(LPCTSTR)pdmmi);
                BuildMenu(hSubMenu,xmlItem);//build sub menu
            }
            m_arrDmmi.Add(pdmmi);
        }
        else if(wcscmp(L"sep",xmlItem.name())==0)
        {
            AppendMenu(menuPopup,MF_SEPARATOR|MF_OWNERDRAW,(UINT_PTR)0,(LPCTSTR)NULL);
        }
        xmlItem=xmlItem.next_sibling();
    }
}

void SMenu::DestroyMenu()
{
    if(!m_pParent)
    {
        if(m_hMenu) ::DestroyMenu(m_hMenu);
        for(UINT i=0; i<m_arrDmmi.GetCount(); i++) 
            delete m_arrDmmi[i];
        m_arrDmmi.RemoveAll();
    }
}

}//namespace SOUI