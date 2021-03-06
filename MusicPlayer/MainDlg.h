/****************************************************************************
*  功    能：使用SOUI自主设计音乐播放器                                     *
*  作    者：小可                                                           *
*  添加时间：2014.01.09 17：00                                              *
*  版本类型：初始版本                                                       *
*  联系方式：QQ-1035144170                                                  *
****************************************************************************/

// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

//#include "BassMusic.h"
//#include "DropEx.h"
#include "MusicOperat.h"


class CMainDlg : public SHostWnd
{
public:
	CMainDlg();
	~CMainDlg();

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnSize(UINT nType, CSize size);

	void OnBtnMsgBox();
	//托盘通知消息处理函数
	LRESULT OnIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL/* bHandled*/);
	//演示如何响应菜单事件
	void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

	void OnTurn3DBut();
	BOOL OnTurn3D(EventArgs *pEvt);
	void OnFlywndState(EventArgs *pEvt);
	void OnMusicTreeBoxEvent(EventArgs *pEvt);

	void OnBtnMyMusic();
	void OnBtnOnlineMusic();
	void OnBtnMyDevice();

	void OnButPrev();
	void OnButPlay();
	void OnButPause();
	void OnButPlayNext();
	void OnButLyric();
	void OnButMusicList();
   
	void DrawMusicInfo();
	LRESULT ReDraw(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL/* bHandled*/);

	void InsertTreeBox(vector<CString> vInfo);
	void InsertSingleInfo(tagMusicInfo pMusicInfo, int nNum);
protected:
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)

		EVENT_NAME_COMMAND(L"btn_my_music",OnBtnMyMusic)
		EVENT_NAME_COMMAND(L"btn_online_music",OnBtnOnlineMusic)
		EVENT_NAME_COMMAND(L"btn_my_device",OnBtnMyDevice)

		EVENT_NAME_COMMAND(L"btn_prev",OnButPrev);
		EVENT_NAME_COMMAND(L"btn_play",OnButPlay);
		EVENT_NAME_COMMAND(L"btn_play_next",OnButPlayNext);
		EVENT_NAME_COMMAND(L"btn_lyric",OnButLyric);
		EVENT_NAME_COMMAND(L"btn_music_list",OnButMusicList);


        //EVENT_NAME_COMMAND(L"btn_msgbox",OnBtnMsgBox)
		EVENT_NAME_COMMAND(L"btn_3D",OnTurn3DBut)
		EVENT_NAME_HANDLER(L"turn3d",EventTurn3d::EventID,OnTurn3D);
        EVENT_NAME_HANDLER(L"myflywnd",FlyStateEvent::EventID,OnFlywndState)
		EVENT_NAME_HANDLER(L"music_tree_box",EVT_OFPANEL,OnMusicTreeBoxEvent)//响应EVT_OFPANEL事件
	EVENT_MAP_END()
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)

		//托盘消息处理
		MESSAGE_HANDLER(WM_ICONNOTIFY, OnIconNotify)
		MSG_WM_COMMAND(OnCommand)
		MESSAGE_HANDLER(MSG_USER_REDRAW,CMainDlg::ReDraw)

		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
protected:
	//////////////////////////////////////////////////////////////////////////
	//  辅助函数
private:
	bool                m_bCut;
	bool			    m_3DType;
	BOOL			    m_bLayoutInited;
	SShellNotifyIcon    shellNotifyIcon;

public:
	bool                bPlayFlag;
	CMusicOpreat		*m_pMuOPreat;
};
