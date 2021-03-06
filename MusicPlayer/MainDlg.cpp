/****************************************************************************
*  功    能：使用SOUI自主设计音乐播放器                                     *
*  作    者：小可                                                           *
*  添加时间：2014.01.09 17：00                                              *
*  版本类型：初始版本                                                       *
*  联系方式：QQ-1035144170                                                  *
****************************************************************************/

// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"



/****************************************************************************
*  函 数 名：GetAudioAllPath                                          
*  功能描述: 回调回去拖拽进列表的所有音频路径                                         
*  输入参数：std::vector<CString> vInfo      [IN] :单个或多个音频文件路径 
*            void *pUser                     [IN] :主窗口的指针
*  输出参数: 无
*  返 回 值：TRUE  执行成功
* 		     FALSE 执行失败
*  抛出异常：无
****************************************************************************/
BOOL __stdcall GetAudioAllPath(vector<CString> vInfo, void *pUser)
{
	if (NULL != pUser)
	{
		int i=0;
		//CString str;
		//str.Format("Dll回调的值： %d\n",lpMyStruct);
		CMainDlg *pThis = (CMainDlg *)pUser;
		pThis->InsertTreeBox(vInfo);
		//测试：播放音乐
		pThis->bPlayFlag=false;
		pThis->OnButPlay();
	}
	return TRUE;
}

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
    m_bCut=false;
	m_3DType=false;
	m_bLayoutInited=FALSE;

	bPlayFlag=false;
	m_pMuOPreat=NULL;
} 

CMainDlg::~CMainDlg()
{
	shellNotifyIcon.Hide();	
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// 		MARGINS mar = {5,5,30,5};
	// 		DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;
	shellNotifyIcon.Create(m_hWnd,GETRESPROVIDER->LoadIcon(_T("ICON_LOGO"),16));
	shellNotifyIcon.Show();
	//InitListCtrl();

	//拖拽功能
	SWindow *pListBox = FindChildByName(L"music_tree_box");
	if(pListBox)
	{
		HRESULT hr=::RegisterDragDrop(m_hWnd,GetDropTarget());
		CTestDropTarget1 *pDrop=new CTestDropTarget1(pListBox);
		RegisterDragDrop(pListBox->GetSwnd(),pDrop);
		pDrop->GetDragData(GetAudioAllPath,this);
	}
    
	//初始化音频操作类
	m_pMuOPreat=new CMusicOpreat(m_hWnd);
	m_pMuOPreat->InitDatas();

	return 0;
}


//TODO:消息映射
void CMainDlg::OnClose()
{
	PostMessage(WM_QUIT);
	
}

void CMainDlg::OnMaximize()
{
	//SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
	if (!m_bCut)
	{
		STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_main");
		if(pTab)
		{
			pTab->SetCurSel(_T("在线音乐"));
			m_bCut=true;
		}
		
	}else
	{
		STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_main");
		if(pTab)
		{
			pTab->SetCurSel(_T("我的音乐"));
			m_bCut=false;
		}
	}

}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

#include <helper/smenu.h>
LRESULT CMainDlg::OnIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL/* bHandled*/)
{
	switch (lParam)
	{
	case  WM_RBUTTONDOWN:
	{
	    SetForegroundWindow(m_hWnd);
            //显示右键菜单
            SMenu menu;
            menu.LoadMenu(_T("menu_tray"),_T("smenu"));
            POINT pt;
            GetCursorPos(&pt);
            menu.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
	}break;
	case WM_LBUTTONDOWN:
	{
		//显示/隐藏主界面
		if (IsIconic())
		   ShowWindow(SW_SHOWNORMAL);
		else
		   ShowWindow(SW_MINIMIZE);
	}break;
	default:
		break;
	}
	return S_OK;
}

//演示如何响应菜单事件
void CMainDlg::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
{
	if (uNotifyCode == 0)
	{
		switch (nID)
		{
		case 6:
			PostMessage(WM_CLOSE);
			break;
		default:
			break;
		}
	}
}


BOOL CMainDlg::OnTurn3D( EventArgs *pEvt )
{
	EventTurn3d *pEvt3dTurn = (EventTurn3d*)pEvt;
	STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_3d");
	if(pEvt3dTurn->bTurn2Front_)
	{
		pTab->SetCurSel(_T("musiclist"));
		
	}else
	{
		pTab->SetCurSel(_T("lrc"));
	}
	return TRUE;
}

void CMainDlg::OnTurn3DBut()
{
	if (!m_3DType)
	{
		STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_3d");
		if(pTab)
		{
			STurn3dView * pTurn3d = FindChildByName2<STurn3dView>(L"turn3d");
			if(pTurn3d)
			{
				pTurn3d->Turn(pTab->GetPage(_T("musiclist")),pTab->GetPage(_T("lrc")),FALSE);
				m_3DType=true;
			}
		}
	}else
	{
		STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_3d");
		if(pTab)
		{
			STurn3dView * pTurn3d = FindChildByName2<STurn3dView>(L"turn3d");
			if(pTurn3d)
			{
				pTurn3d->Turn(pTab->GetPage(_T("lrc")),pTab->GetPage(_T("musiclist")),TRUE);
				m_3DType=false;
			}
		}

	}
	
}

void CMainDlg::OnBtnMyMusic()
{
	STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_main");
	if(pTab)
	{
		pTab->SetCurSel(_T("我的音乐"));
	}

}

void CMainDlg::OnBtnOnlineMusic()
{
	STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_main");
	if(pTab)
	{
		pTab->SetCurSel(_T("在线音乐"));
	}

}

void CMainDlg::OnBtnMyDevice()
{
	STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_main");
	if(pTab)
	{
		pTab->SetCurSel(_T("我的设备"));
	}

}

void CMainDlg::OnButPrev()         // 上一曲
{
	m_pMuOPreat->OnButPrev();
	if (m_pMuOPreat->m_MusicManager.empty())
	{
		return;
	}
	DrawMusicInfo();
}

void CMainDlg::OnButPlay()         // 播放
{
	if (!bPlayFlag)
	{
		m_pMuOPreat->OnButPlay();
		bPlayFlag=true;

	}else
	{
		m_pMuOPreat->OnButPause();
		bPlayFlag=false;
	}
	if (m_pMuOPreat->m_MusicManager.empty())
	{
		return;
	}
	DrawMusicInfo();

	//测试：播放音乐
	//if (hStream)
	//{
	//	BASS_ChannelStop(hStream);
	//	hStream=NULL;
	//}
	//CString lpszFileName="C:/Users/Administrator/Desktop/Music/BEYOND - 不再犹豫.mp3";
	//hStream=BASS_StreamCreateFile(FALSE, lpszFileName.GetBuffer(),0,0,BASS_SAMPLE_MONO);
	//if (hStream)
	//{
	//	//开始播放
	//	BOOL bResult = BASS_ChannelPlay(hStream, FALSE);
	//	if (bResult)
	//	{
	//		int i=0;
	//	}
	//}
}

void CMainDlg::OnButPause()      // 暂停
{
	m_pMuOPreat->OnButPause();
}

void CMainDlg::OnButPlayNext()     // 下一曲
{
	m_pMuOPreat->OnButPlayNext();
	if (m_pMuOPreat->m_MusicManager.empty())
	{
		return;
	}
	DrawMusicInfo();
}

void CMainDlg::OnButLyric()        // 歌词面板
{

	STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_3d");
	if(pTab&&!m_3DType)
	{
		STurn3dView * pTurn3d = FindChildByName2<STurn3dView>(L"turn3d");
		if(pTurn3d)
		{
			pTurn3d->Turn(pTab->GetPage(_T("musiclist")),pTab->GetPage(_T("lrc")),FALSE);
		}
		m_3DType=true;
	}
}

void CMainDlg::OnButMusicList()    // 歌词列表
{

	STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_3d");
	if(pTab&&m_3DType)
	{
		STurn3dView * pTurn3d = FindChildByName2<STurn3dView>(L"turn3d");
		if(pTurn3d)
		{
			pTurn3d->Turn(pTab->GetPage(_T("lrc")),pTab->GetPage(_T("musiclist")),TRUE);
		}
		m_3DType=false;
	}
}

void CMainDlg::OnFlywndState( EventArgs *pEvt )
{
    FlyStateEvent *pEvtFlywnd = (FlyStateEvent*)pEvt;
    if(pEvtFlywnd->nPercent == SAnimator::PER_END)
    {
        //测试：隐藏音乐频谱 (可以用定时器判断分层窗口的收缩状态来控制)

        SWindow *pSpectrum = FindChildByName2<SWindow>("spectrum");
        if(pSpectrum) pSpectrum->SetVisible(pEvtFlywnd->bEndPos,TRUE);        
//         if (!m_bCut)
//         {
//             m_bCut=true;
//         }else
//         {
//             SWindow *pSpectrum = FindChildByName2<SWindow>("spectrum");
//             if(pSpectrum) pSpectrum->SetVisible(FALSE,TRUE);
//             m_bCut=false;
//         }        
    }
}

void CMainDlg::InsertTreeBox(vector<CString> vInfo)
{
	//找到列表控件,先清干净列表
	STreeBox *pTreeBox=FindChildByName2<STreeBox>(L"music_tree_box");
	if(pTreeBox)
	{
		pTreeBox->RemoveAllItems();
	}

	for (int i=0;i<vInfo.size();++i)
	{
		tagMusicInfo pMusicInfo;
		m_pMuOPreat->InsertMapInfo(i,vInfo[i],pMusicInfo);

		//往ListBox中添加新数据
		InsertSingleInfo(pMusicInfo,i);
	}

}

void CMainDlg::InsertSingleInfo(tagMusicInfo pMusicInfo, int nNum)
{
	//找到列表控件
	STreeBox *pTreeBox=FindChildByName2<STreeBox>(L"music_tree_box");

	if(pTreeBox)
	{
		SStringW m_sTemp;
		HSTREEITEM rootItem;

		SStringW StrTotleTime;
		DWORD dwTotleTime = pMusicInfo.dwTime;
		StrTotleTime.Format(L"%u:%02u",dwTotleTime/60,dwTotleTime%60);

		if (nNum%2)
		{
			//m_sTemp=_T("<item>\
			//	          <img skin=\"_skin.logo.skin\" pos=\"5,5,55,55\" />\
			//			  <text pos=\"70,5\"  valign=\"middle\" align=\"left\" colorText=\"#CDD7E1\" >歌曲：BEYOND - 不再犹豫</text>\
			//			  <text pos=\"70,20\" valign=\"middle\" align=\"left\" colorText=\"#CDD7E1\" >歌手：小可 </text>\
			//			  <text pos=\"70,35\" valign=\"middle\" align=\"left\" colorText=\"#CDD7E1\" >时长：05:01 交流群:229313785 </text>\
			//		    </item>"); 
			m_sTemp.Format(L"<item>\
							  <img skin=\"_skin.logo.skin\" pos=\"5,5,55,55\" />\
							  <text pos=\"70,5\"  valign=\"middle\" align=\"left\" colorText=\"#CDD7E1\" >歌曲：%s </text>\
							  <text pos=\"70,20\" valign=\"middle\" align=\"left\" colorText=\"#CDD7E1\" >歌手：%s </text>\
							  <text pos=\"70,35\" valign=\"middle\" align=\"left\" colorText=\"#CDD7E1\" >时长：%s 交流群:229313785 </text>\
							  </item>",pMusicInfo.szTitle,pMusicInfo.szArtist,StrTotleTime);
		}else
		{
	        //m_sTemp=_T("<item>\
					  //    <img skin=\"_skin.spectrum\" pos=\"5,5,55,55\" />\
					  //    <text pos=\"70,5\"  valign=\"middle\" align=\"left\" colorText=\"#CDD7E1\" >歌曲：本兮 - 寂夜</text>\
					  //    <text pos=\"70,20\" valign=\"middle\" align=\"left\" colorText=\"#CDD7E1\" >歌手：小可 </text>\
					  //    <text pos=\"70,35\" valign=\"middle\" align=\"left\" colorText=\"#CDD7E1\" >时长：04:50 QQ:1035144170 </text>\
					  //  </item>"); 
			m_sTemp.Format(L"<item>\
							<img skin=\"_skin.spectrum\" pos=\"5,5,55,55\" />\
							  <text pos=\"70,5\"  valign=\"middle\" align=\"left\" colorText=\"#CDD7E1\" >歌曲：%s </text>\
							  <text pos=\"70,20\" valign=\"middle\" align=\"left\" colorText=\"#CDD7E1\" >歌手：%s </text>\
							  <text pos=\"70,35\" valign=\"middle\" align=\"left\" colorText=\"#CDD7E1\" >时长：%s 小可QQ:1035144170 </text>\
							</item>",pMusicInfo.szTitle,pMusicInfo.szArtist,StrTotleTime);
		}

		STreeItem *pItem;
		pItem=pTreeBox->InsertItem((LPCWSTR )m_sTemp,(DWORD)rootItem,STVI_ROOT,STVI_LAST,FALSE);
		//if (pItem)
		//{
		//  m_sTemp.Format(,strTemp);
		//	pItem->FindChildByName(_T("我的音乐"))->SetWindowText(_T("..."));
		//}

	}
}

void CMainDlg::OnMusicTreeBoxEvent( EventArgs *pEvt )
{
	EventOfPanel *pEvtOfPanel = (EventOfPanel*)pEvt;
	if(pEvtOfPanel->pOrgEvt->GetID() == EventCmd::EventID
		&& pEvtOfPanel->pOrgEvt->sender->IsClass(SButton::GetClassName()))
	{
		HSTREEITEM hItem = (HSTREEITEM)pEvtOfPanel->pPanel->GetItemIndex();
		SStringT strMsg;
		strMsg.Format(_T("收到treebox item:0x%08x中的name为%s的窗口点击事件"),hItem,S_CW2T(pEvtOfPanel->pOrgEvt->nameFrom));
		SMessageBox(m_hWnd,strMsg,_T("EVENTOFPANEL"),MB_OK|MB_ICONEXCLAMATION); 

		/*应该怎么获取TreeBox双击的响应消息*/

		/*该如何响应当前 双击的选项播放音乐*/
	}
}

void CMainDlg::DrawMusicInfo()
{
	CString m_sTemp;
	CString StrTotleTime;
	tagMusicInfo pMusicInfo;
	pMusicInfo=*(m_pMuOPreat->m_MusicManager[m_pMuOPreat->nIndex]);
	DWORD dwTotleTime = pMusicInfo.dwTime;
	StrTotleTime.Format(TEXT("%u:%02u"),dwTotleTime/60,dwTotleTime%60);

	//找到列表控件
	SWindow *pText=FindChildByName2<SWindow>(L"play_title");
	if (pText)
	{
		m_sTemp.Format(_T("当前播放:%s 总长:%s"),pMusicInfo.szTitle,StrTotleTime);
		pText->SetWindowText(m_sTemp);
	}
}
LRESULT CMainDlg::ReDraw(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL/* bHandled*/)
{
	DrawMusicInfo();
	return 0;
}