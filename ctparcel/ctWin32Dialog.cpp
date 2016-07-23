
#include <windows.h>
#include <list>
#include <map>
#include <functional>
#include <thread>

#include "ctWin32Dialog.h"

namespace ctWin32Dialog
{
	using namespace std;

	// �����ⶨ�����ڵ�static����
	list<ctDialogInfo> ctDialog::ctDialogInfos;

	//////////////////////////////////////////////////////////////////////////
	// ct�Ի��������
	ctDialog::ctDialog()
	{
		appInstance = GetModuleHandleA( nullptr );
	}
	ctDialog::~ctDialog()
	{
		regCommandMap.clear();
		allcreated.clear();
		lineInfos.clear();
		bmpInfos.clear();

		// ����Ҫ��һ���ж�: ctDialogInfos�Ƿ�ɶ�
		// �����౻����Ϊȫ�ֶ���,�ڳ����˳�ʱ��������ʱ,��static-var�����Ѿ���ɾ��
		if(ctDialogInfos.size())
		{
			// ��������ʱ���ctDialogInfos�м�¼��objectָ��
			for(auto &it = ctDialogInfos.begin(); it != ctDialogInfos.end(); it++)
			{
				if(it->hWnd == hMainDlg)
				{
					ctDialogInfos.erase( it );
					break;
				}
			}
		}
	}

	// �Ի���callback
	LRESULT CALLBACK ctDialog::mainWndCallback( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch(message)
		{
			case WM_COMMAND:
				{
					auto callback = [&]( HWND hWnd, DWORD windowId )->int {
						auto it = regCommandMap.find( windowId );
						if(it != regCommandMap.end() && it->second)
							return it->second( hWnd, windowId );
						return -1;
					};

					return callback( hWnd, LOWORD( wParam ) );
				}

			case WM_CTLCOLORSTATIC: //bg-static
				{
					char tmp[200];
					GetWindowTextA( (HWND)lParam, tmp, 200 );

					RECT rt;
					COLORREF tmpbg = bgColor;
					GetWindowRect( (HWND)lParam, &rt );
					POINT pt = {rt.left,rt.top};
					ScreenToClient( hWnd, &pt );
					if(PtInRect( &forecolorRect, {pt.x,pt.y} ))
						tmpbg = foreColor;
					SetTextColor( (HDC)wParam, fontColor );
					SetBkColor( (HDC)wParam, tmpbg );
					SetBkMode( (HDC)wParam, TRANSPARENT );
					return (INT_PTR)CreateSolidBrush( tmpbg );
				}

			case WM_PAINT:
				{
					PAINTSTRUCT ps;
					HDC hdc;
					RECT rect;
					GetClientRect( hWnd, &rect );
					hdc = BeginPaint( hWnd, &ps );

					// brush color
					auto brushWindow = [&]( COLORREF color, RECT rt ) {
						HBRUSH brush = CreateSolidBrush( color );		//����
						HPEN pen = CreatePen( PS_NULL, 0, 0 );			//����
						SelectObject( hdc, brush );
						SelectObject( hdc, pen );
						Rectangle( hdc, 0, 0, rt.right, rt.bottom );
						DeleteObject( pen );
						DeleteObject( brush );
					};
					// brush background-color
					brushWindow( bgColor, rect );
					// brush forecolor
					brushWindow( foreColor, forecolorRect );

					// Draw Lines
					auto drawline = [&]( COLORREF col, int sx, int sy, int ex, int ey ) {
						HPEN pen = CreatePen( PS_SOLID, 0, col );			//����
						SelectObject( hdc, pen );
						MoveToEx( hdc, sx, sy, NULL );
						LineTo( hdc, ex, ey );
						DeleteObject( pen );
					};
					for(auto &it : lineInfos)
						drawline( it.color, it.lstart.x, it.lstart.y, it.lend.x, it.lend.y );

					// show bmp
					auto drawbmp = [&]( string& filename, int cx, int cy, int width, int height ) {
						//�������Ҫ��Ч�ʸߵĻ� Ҫ��ǰload
						HBITMAP hbmp = (HBITMAP)LoadImageA( NULL, filename.c_str(),
							IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION );
						HDC hMemDC = CreateCompatibleDC( hdc );
						SelectObject( hMemDC, hbmp );
						BitBlt( hdc, cx, cy, width, height, hMemDC, 0, 0, SRCCOPY );
						DeleteObject( hbmp );
						DeleteObject( hMemDC );
					};
					for(auto &it : bmpInfos)
						drawbmp( it.filename, it.cx, it.cy, it.width, it.height );

					EndPaint( hWnd, &ps );
					return FALSE;
				}

			case WM_DESTROY:
				{
					PostQuitMessage( 0 );
					break;
				}


			default:
				// ��ȥ����defwndproc
				return -1;
		}
		return 0;
	}
	// ��ʼ���Ի������
	bool ctDialog::createMainDialog( int xWidth, int xHeight,
		COLORREF xbgColor, COLORREF xforeColor, RECT xforeRect, COLORREF xfontColor )
	{
		// constants set
		const auto szClass = TEXT( "ctWizard" );

		// param set
		bgColor = xbgColor;
		fontColor = xfontColor;
		foreColor = xforeColor;
		forecolorRect = xforeRect;
		hMainDlgRect = {0,0,xWidth,xHeight};

		// register class
		WNDCLASSEXW wcex = {0};
		wcex.cbSize = sizeof( WNDCLASSEX );
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = staticWndCallback;	//��ת������static����
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = appInstance;
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszClassName = szClass;
		wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
		RegisterClassExW( &wcex );

		// create main window
		HWND hWnd = CreateWindow( szClass, TEXT( "ctDialog" ), WS_OVERLAPPEDWINDOW&~WS_MAXIMIZEBOX&~WS_THICKFRAME,
			CW_USEDEFAULT, 0, xWidth, xHeight, nullptr, nullptr, appInstance, nullptr );

		if(hWnd)
		{
			hMainDlg = hWnd;
			// ��ӵ�ctDialogInfos��¼��,�Ա���תcallback������ĺ���
			ctDialogInfos.push_back( {hWnd,this} );

			ShowWindow( hWnd, 5 );
			UpdateWindow( hWnd );
			return true;
		}

		return false;
	}
	// ��ʾ�Ի���
	// �������� ֱ���Ի��򱻹ر�
	void ctDialog::showMainDialog()
	{
		MSG msg;
		while(GetMessage( &msg, nullptr, 0, 0 ))
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	// chooseFolders -> ѡ���ļ��н��� -> ѡ����ֱ�ӷ���partNameEdit�� 
	bool ctDialog::chooseFolders( string partNameEdit )
	{
		BROWSEINFOA bi = {0};
		LPITEMIDLIST pIDList;
		bi.hwndOwner = hMainDlg;
		bi.lpszTitle = "ѡ���ļ���";
		bi.ulFlags = BIF_RETURNONLYFSDIRS;
		pIDList = SHBrowseForFolderA( &bi );
		if(pIDList)
		{
			char tmp[260];
			SHGetPathFromIDListA( pIDList, tmp );
			setEditText( partNameEdit, tmp );
			return true;
		}
		return FALSE;
	}

	// ����ɾ���ؼ�
	// CreateWindow@param : className , windowName, x ,y ,width, height (partType = partType | WS_VISIBLE | WS_CHILD )
	// partName@param provide for deletePart()
	// proc == callback : function<int CALLBACK( DWORD )>
	// ext : bCaptionFont == true ? fontsize=16 : fontsize=system-default
	bool ctDialog::createPart( string className, string windowName, DWORD partType,
		int x, int y, int width, int height, string partName, CommandCallback proc, int bCaptionFontsize )
	{
		// ��ȡһ��id
		auto getwindowid = [&]()->int {
			int iret = 100;  //iwIdStart=100
			for(auto it : allcreated)
			{
				if(it.windowId == iret)
					iret++;
			}
			return iret;
		};
		DWORD windowId = getwindowid();

		// ��������
		HWND hWnd = CreateWindowA( className.c_str(), windowName.c_str(), WS_VISIBLE | WS_CHILD | partType,
			x, y, width, height, hMainDlg, (HMENU)windowId, appInstance, nullptr );
		//����
		HFONT font = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
		if(bCaptionFontsize)
		{
			font = CreateFont( bCaptionFontsize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
				OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_SWISS, TEXT( "����" ) );
		}
		SendMessage( hWnd, WM_SETFONT, (WPARAM)font, TRUE );
		UpdateWindow( hMainDlg );

		allcreated.push_back( {partName,windowId,hWnd} );
		regCommandMap.insert( pair<int, CommandCallback>( windowId, proc ) );
		return true;
	}
	// partName == "*" ʱΪɾ������
	bool ctDialog::deletePart( string partName )
	{
		bool ret = false;
		for(auto &it = allcreated.begin(); it != allcreated.end();)
		{
			if(it->partname == partName || partName == "*")
			{
				// ɾ��partWindow
				DestroyWindow( it->hWnd );
				// ȡ��ע��ĺ���
				for(auto &itmap = regCommandMap.begin(); itmap != regCommandMap.end(); itmap++)
				{
					if(itmap->first == it->windowId)
					{
						regCommandMap.erase( itmap++ );
						break;
					}
				}
				// ɾ��������¼
				allcreated.erase( it++ );
				ret = true;
				continue;
			}
			it++;
		}
		return ret;
	}
	 void ctDialog::deleteAllPart()
	{
		deletePart( "*" );
	}
	// ֱ����յ�ǰ���
	void ctDialog::clearDlg()
	{
		deleteAllPart();
		eraseDrewAll();
	}

	//
	// ��ȡ�������Ŀؼ����
	HWND ctDialog::getWnd( string partName )
	{
		for(auto &it : allcreated)
		{
			if(it.partname == partName)
				return it.hWnd;
		}
		return 0;
	}

	//
	// �����ؼ�
	// ����ʱ����partNameʱ�ſ���ɾ��ָ���Ŀؼ�  
	// (��ȻҲ����ʹ��Ĭ�ϵ�partname == "static","button"...)
	//
	// text
	bool ctDialog::createText( string content, int x, int y, int width, int height,
		int isCaptionSize, string partName )
	{
		return createPart( "STATIC", content, NULL, x, y, width, height, partName, nullptr, isCaptionSize );
	}
	// push-button
	bool ctDialog::createbutton( string content, int x, int y, 
		CommandCallback proc,int width, int height, string partName )
	{
		return createPart( "button", content, BS_PUSHBUTTON, x, y, width, height, partName, proc );
	}
	// edit
	// @partType = WS_BORDER
	bool ctDialog::createEdit( int x, int y, int width, int height, 
		string partName, string defaultContent, DWORD partType )
	{
		return createPart( "EDIT", defaultContent, partType, x, y, width, height, partName );
	}
	bool ctDialog::setEditText(string partName,string editContent)
	{
		HWND hedit = getWnd( partName );
		if(hedit)
		{
			SetWindowTextA( hedit, editContent.c_str() );
			return true;
		}
		return false;
	}
	string ctDialog::getEditText( string partName )
	{
		HWND hedit = getWnd( partName );
		if(hedit)
		{
			char tmp[500];
			GetWindowTextA( hedit, tmp, 500 );
			return string(tmp);
		}
		return nullptr;
	}
	//
	// ���ؼ� (ע��:�����Ŀؼ����ᱣ���allcreated)
	//
	// ɾ���Ѿ�����ȥ�Ŀؼ� (����drawxxxx��ȥ��)
	// 
	template <typename T>
	void ctDialog::eraseDrewPart( T& infos, string partName )
	{
		for(auto &it = infos.begin(); it != infos.end(); it++)
		{
			if(it->partname == partName)
			{
				infos.erase( it );
				InvalidateRect( hMainDlg, NULL, FALSE );
				break;
			}
		}
	}
	void ctDialog::eraseDrewAll()
	{
		lineInfos.clear();
		bmpInfos.clear();
		InvalidateRect( hMainDlg, NULL, FALSE );
	}
	// draw line (�����¼���wm_paint�ﻭ��)
	// line =>  sx,sy -> ex,ey
	void ctDialog::drawLine( int sx, int sy, int ex, int ey, COLORREF col, string partName )
	{
		lineInfos.push_back( {partName, col,{sx,sy},{ex,ey}} );
		InvalidateRect( hMainDlg, NULL, FALSE );
	}
	void ctDialog::eraseLine( string partName )
	{
		eraseDrewPart<list<LineInfo>>( lineInfos, partName );
	}

	// draw images(bmp)
	void ctDialog::drawBmp( string filename, int cx, int cy, int width, int height, string partName )
	{
		bmpInfos.push_back( {partName, filename,cx,cy,width,height} );
		InvalidateRect( hMainDlg, NULL, FALSE );
	}
	void ctDialog::eraseBmp( string partName )
	{
		eraseDrewPart<list<BmpInfo>>( bmpInfos, partName );
	}

	// ���Ի�����ʽ
	void ctDialog::setTitle( string title )
	{
		SetWindowTextA( hMainDlg, title.c_str() );
	}
	void ctDialog::setbgcolor( COLORREF col )
	{
		bgColor = col;
		InvalidateRect( hMainDlg, NULL, FALSE );
	}
	void ctDialog::setFontColor( COLORREF col )
	{
		fontColor = col;
		InvalidateRect( hMainDlg, NULL, FALSE );
	}
	void ctDialog::setForecolor( COLORREF col, RECT foreRt )
	{
		foreColor = col;
		forecolorRect = foreRt;
		InvalidateRect( hMainDlg, NULL, FALSE );
	}

	//
	// static�ص� (��ת)
	LRESULT CALLBACK ctDialog::staticWndCallback( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		for(auto &it : ctDialogInfos)
		{
			if(hWnd == it.hWnd)
			{
				LRESULT lret = (it.object)->mainWndCallback( hWnd, message, wParam, lParam );
				if(lret == -1)
					break;
				return lret;
			}
		}
		return DefWindowProc( hWnd, message, wParam, lParam );
	}
}
