#ifndef CTWIN32DIALOG_H
#define CTWIN32DIALOG_H

#include <windows.h>
#include <list>
#include <map>
#include <functional>
#include <thread>
#include <Shlobj.h>
#include <commctrl.h>			
#pragma comment(lib, "comctl32") 

namespace ctWin32Dialog
{
	using namespace std;

	enum Color
	{
		COLOR_GRAY = 0xF0F0F0,		//��
		COLOR_WHITE = 0xFFFFFF,		//��
		COLOR_BALCK = 0	     		//��
	};

	#define CommandCallback	function<int CALLBACK( HWND, DWORD )>
	class ctDialog;

	struct ctDialogInfo
	{
		HWND hWnd;
		ctDialog* object;
	};
	struct PartInfo
	{
		string partname;
		DWORD windowId;
		HWND hWnd;
	};
	struct LineInfo
	{
		string partname;
		COLORREF color;
		POINT lstart, lend;
	};
	struct BmpInfo
	{
		string partname;
		string filename;
		int cx, cy;
		int width, height;
	};

	//////////////////////////////////////////////////////////////////////////
	// ct�Ի��������
	class ctDialog
	{
	public:
		ctDialog();
		~ctDialog();

		// �Ի���callback
		LRESULT CALLBACK mainWndCallback( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		// ��ʼ���Ի������
		bool createMainDialog( int xWidth, int xHeight,
			COLORREF xbgColor = COLOR_GRAY, COLORREF xforeColor = COLOR_WHITE,
			RECT xforeRect = {0}, COLORREF xfontColor = COLOR_BALCK );
		// ��ʾ�Ի���
		// �������� ֱ���Ի��򱻹ر�
		void showMainDialog();

		// chooseFolders -> ѡ���ļ��н��� -> ѡ����ֱ�ӷ���partNameEdit�� 
		string chooseFolders( );

		// ����ɾ���ؼ�
		// CreateWindow@param : className , windowName, x ,y ,width, height (partType = partType | WS_VISIBLE | WS_CHILD )
		// partName@param provide for deletePart()
		// proc == callback : function<int CALLBACK( DWORD )>
		// ext : bCaptionFont == true ? fontsize=16 : fontsize=system-default
		bool createPart( string className, string windowName, DWORD partType,
			int x, int y, int width, int height, string partName, CommandCallback proc = nullptr, int bCaptionFontsize = 0 );
		// partName == "*" ʱΪɾ������
		bool deletePart( string partName );
		void deleteAllPart();
		// ֱ����յ�ǰ���
		void clearDlg();

		//
		// ��ȡ�������Ŀؼ����
		HWND getWnd( string partName );

		//
		// �����ؼ�
		// ����ʱ����partNameʱ�ſ���ɾ��ָ���Ŀؼ�  
		// (��ȻҲ����ʹ��Ĭ�ϵ�partname == "static","button"...)
		//
		// text
		bool createText( string content, int x, int y, int width, int height,
			int isCaptionSize = 0, string partName = "static" );
		// push-button
		bool createbutton( string content, int x, int y, CommandCallback proc = nullptr, 
			int width = 85, int height = 22, string partName = "button" );
		// edit
		bool createEdit( int x, int y, int width, int height = 20, 
			string partName = "edit",  string defaultContent = "",  DWORD partType = WS_BORDER );
		bool setEditText( string partName, string editContent );
		string getEditText( string partName );
		// progress base
		bool createProgress(int x,int y, string partName = "progress", int defaultRange=100, int width = 400, int height = 20 )
		{
			bool bret = createPart( "msctls_progress32", "progress", NULL, x, y, width, height, partName, nullptr );
			SendMessage( getWnd( partName ), PBM_SETRANGE, 0, MAKELONG( 0, defaultRange ) );
			SendMessage( getWnd( partName ), PBM_SETPOS, 0, 0 );
			return bret;
		}

		//
		// ���ؼ� (ע��:�����Ŀؼ����ᱣ���allcreated)
		//
		// ɾ���Ѿ�����ȥ�Ŀؼ� (����drawxxxx��ȥ��)
		// 
		template <typename T> void eraseDrewPart( T& infos, string partName );
		void eraseDrewAll();

		// draw line (�����¼���wm_paint�ﻭ��)
		// line =>  sx,sy -> ex,ey
		void drawLine( int sx, int sy, int ex, int ey, COLORREF col = RGB( 0, 0, 0 ), string partName = "line" );
		void eraseLine( string partName );

		// draw images(bmp)
		void drawBmp( string filename, int cx, int cy, int width, int height, string partName = "bmp" );
		void eraseBmp( string partName );

		// ���Ի�����ʽ
		void setTitle( string title );
		void setbgcolor( COLORREF col );
		void setFontColor( COLORREF col );
		void setForecolor( COLORREF col, RECT foreRt = {0} );

		//
		// static�ص� (��ת)
		static LRESULT CALLBACK staticWndCallback( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

	public:
		HWND hMainDlg;
		RECT hMainDlgRect;

	private:
		HINSTANCE appInstance;
		COLORREF bgColor, fontColor, foreColor;
		RECT forecolorRect;
		// ��¼����bmpImages����Ϣ
		list<BmpInfo> bmpInfos;
		// ��¼����LineTo����Ϣ
		list<LineInfo> lineInfos;
		// ��¼��ǰ�Ի����е����������Ϣ
		list<PartInfo> allcreated;
		// record all part's callback
		map<int, CommandCallback> regCommandMap;

		// static����:
		// ��¼��ǰ���������е����,�Թ����ö�Ӧ��proc
		static list<ctDialogInfo> ctDialogInfos;
	};
}


#endif
