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
		COLOR_GRAY = 0xF0F0F0,		//灰
		COLOR_WHITE = 0xFFFFFF,		//白
		COLOR_BALCK = 0	     		//黑
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
	// ct对话框基础类
	class ctDialog
	{
	public:
		ctDialog();
		~ctDialog();

		// 对话框callback
		LRESULT CALLBACK mainWndCallback( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		// 初始化对话框面板
		bool createMainDialog( int xWidth, int xHeight,
			COLORREF xbgColor = COLOR_GRAY, COLORREF xforeColor = COLOR_WHITE,
			RECT xforeRect = {0}, COLORREF xfontColor = COLOR_BALCK );
		// 显示对话框
		// 阻塞函数 直到对话框被关闭
		void showMainDialog();

		// chooseFolders -> 选择文件夹界面 -> 选定后直接放入partNameEdit中 
		string chooseFolders( );

		// 增加删除控件
		// CreateWindow@param : className , windowName, x ,y ,width, height (partType = partType | WS_VISIBLE | WS_CHILD )
		// partName@param provide for deletePart()
		// proc == callback : function<int CALLBACK( DWORD )>
		// ext : bCaptionFont == true ? fontsize=16 : fontsize=system-default
		bool createPart( string className, string windowName, DWORD partType,
			int x, int y, int width, int height, string partName, CommandCallback proc = nullptr, int bCaptionFontsize = 0 );
		// partName == "*" 时为删除所有
		bool deletePart( string partName );
		void deleteAllPart();
		// 直接清空当前面板
		void clearDlg();

		//
		// 获取创建过的控件句柄
		HWND getWnd( string partName );

		//
		// 创建控件
		// 调用时带上partName时才可以删除指定的控件  
		// (当然也可以使用默认的partname == "static","button"...)
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
		// 画控件 (注意:这样的控件不会保存进allcreated)
		//
		// 删除已经画上去的控件 (仅限drawxxxx上去的)
		// 
		template <typename T> void eraseDrewPart( T& infos, string partName );
		void eraseDrewAll();

		// draw line (保存记录后从wm_paint里画的)
		// line =>  sx,sy -> ex,ey
		void drawLine( int sx, int sy, int ex, int ey, COLORREF col = RGB( 0, 0, 0 ), string partName = "line" );
		void eraseLine( string partName );

		// draw images(bmp)
		void drawBmp( string filename, int cx, int cy, int width, int height, string partName = "bmp" );
		void eraseBmp( string partName );

		// 主对话框样式
		void setTitle( string title );
		void setbgcolor( COLORREF col );
		void setFontColor( COLORREF col );
		void setForecolor( COLORREF col, RECT foreRt = {0} );

		//
		// static回调 (中转)
		static LRESULT CALLBACK staticWndCallback( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

	public:
		HWND hMainDlg;
		RECT hMainDlgRect;

	private:
		HINSTANCE appInstance;
		COLORREF bgColor, fontColor, foreColor;
		RECT forecolorRect;
		// 记录所有bmpImages的信息
		list<BmpInfo> bmpInfos;
		// 记录所有LineTo的信息
		list<LineInfo> lineInfos;
		// 记录当前对话框中的所有组件信息
		list<PartInfo> allcreated;
		// record all part's callback
		map<int, CommandCallback> regCommandMap;

		// static数据:
		// 记录当前创建的所有的面板,以供调用对应的proc
		static list<ctDialogInfo> ctDialogInfos;
	};
}


#endif
