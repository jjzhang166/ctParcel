// ctparcel.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <list>
#include <map>
#include <functional>
#include <thread>


namespace ctWin32Dialog
{
	using namespace std;

	enum Color
	{
		COLOR_GRAY  = 0xF0F0F0,		//灰
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
		ctDialog()
		{
			appInstance = GetModuleHandleA( nullptr );
		}
		~ctDialog()
		{
			regCommandMap.clear();
			allcreated.clear();
			lineInfos.clear();

			// 对象销毁时清空ctDialogInfos中记录的object指针
			for(auto &it=ctDialogInfos.begin(); it!= ctDialogInfos.end(); it++)
			{
				if(it->hWnd == hMainDlg)
				{
					ctDialogInfos.erase( it );
					break;
				}
			}
		}

		// 对话框callback
		LRESULT CALLBACK mainWndCallback( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
		{
			switch(message)
			{
				case WM_COMMAND:
					{
						auto callback = [&]( HWND hWnd, DWORD windowId )->int {
							auto it = regCommandMap.find( windowId );
							if(it != regCommandMap.end() && it->second)
								return it->second( hWnd,windowId );
							return -1;
						};

						return callback( hWnd, LOWORD( wParam ) );
					}

				case WM_CTLCOLOREDIT:	//bg-edit
					{
						RECT rt;
						COLORREF tmpbg = bgColor;
						GetClientRect( (HWND)lParam, &rt );
						if(PtInRect( &forecolorRect, {rt.right / 2,rt.bottom / 2} ))
							tmpbg = foreColor;
						return (LONG)CreateSolidBrush( tmpbg );
					}
					

				case WM_CTLCOLORSTATIC: //bg-static
					{
						RECT rt;
						COLORREF tmpbg = bgColor;
						GetClientRect( (HWND)lParam, &rt );
						if(PtInRect( &forecolorRect, {rt.right / 2,rt.bottom / 2} ))
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
						auto brushWindow = [&](COLORREF color,RECT rt) {
							HBRUSH brush = CreateSolidBrush( color );		//内容
							HPEN pen = CreatePen( PS_NULL, 0, 0 );			//线条
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
						auto drawline = [&](COLORREF col,int sx,int sy,int ex,int ey) {
							HPEN pen = CreatePen( PS_SOLID, 0, col );			//线条
							SelectObject( hdc, pen );
							MoveToEx( hdc, sx, sy, NULL );
							LineTo( hdc, ex, ey );
							DeleteObject( pen );
						};
						for(auto &it : lineInfos)
							drawline( it.color, it.lstart.x, it.lstart.y, it.lend.x, it.lend.y );
						
						// show bmp
						auto drawbmp = [&]( string& filename, int cx, int cy, int width, int height ) {
							//这里如果要求效率高的话 要提前load
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
					PostQuitMessage( 0 );
					break;

				default:
					// 回去调用defwndproc
					return -1;
			}
			return 0;
		}
		// 初始化对话框面板
		bool createMainDialog( int xWidth, int xHeight, 
			COLORREF xbgColor = COLOR_GRAY, COLORREF xforeColor = COLOR_WHITE, RECT xforeRect = {0}, COLORREF xfontColor = COLOR_BALCK )
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
			wcex.lpfnWndProc = staticWndCallback;	//跳转到代理static函数
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hInstance = appInstance;
			wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszClassName = szClass;
			wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
			RegisterClassExW( &wcex );

			// create main window
			HWND hWnd = CreateWindow( szClass, TEXT( "ctDialog" ), WS_OVERLAPPEDWINDOW&~WS_MAXIMIZEBOX,
				CW_USEDEFAULT, 0, xWidth, xHeight, nullptr, nullptr, appInstance, nullptr );

			if(hWnd)
			{
				hMainDlg = hWnd;
				// 添加到ctDialogInfos记录中,以便中转callback调用类的函数
				ctDialogInfos.push_back( {hWnd,this} );

				ShowWindow( hWnd, 5 );
				UpdateWindow( hWnd );
				return true;
			}

			return false;
		}
		// 显示对话框
		// 阻塞函数 直到对话框被关闭
		void showMainDialog()
		{
			MSG msg;
			while(GetMessage( &msg, nullptr, 0, 0 ))
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}

		// 增加删除控件
		// CreateWindow@param : className , windowName, x ,y ,width, height (partType = partType | WS_VISIBLE | WS_CHILD )
		// partName@param provide for deletePart()
		// proc == callback : function<int CALLBACK( DWORD )>
		// ext : bCaptionFont == true ? fontsize=16 : fontsize=system-default
		bool createPart( string className, string windowName, DWORD partType, 
			int x, int y, int width, int height, string partName, CommandCallback proc = nullptr ,bool bCaptionFont=false )
		{
			// 获取一个id
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

			// 创建窗口
			HWND hWnd = CreateWindowA( className.c_str(), windowName.c_str(),  WS_VISIBLE | WS_CHILD | partType,
				x, y, width, height, hMainDlg, (HMENU)windowId, appInstance, nullptr );
			//字体
			HFONT font = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
			if(bCaptionFont)
			{
				font = CreateFont( 16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
					OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_SWISS, TEXT( "宋体" ) );
			}
			SendMessage( hWnd, WM_SETFONT, (WPARAM)font, TRUE );
			UpdateWindow( hMainDlg );

			allcreated.push_back( {partName,windowId,hWnd} );
			regCommandMap.insert( pair<int, CommandCallback>( windowId, proc ) );
			return true;
		}
		// partName == "*" 时为删除所有
		bool deletePart( string partName )
		{
			bool ret = false;
			for(auto &it = allcreated.begin(); it != allcreated.end();)
			{
				if(it->partname == partName || partName=="*" )
				{
					// 删除partWindow
					DestroyWindow( it->hWnd );
					// 取消注册的函数
					for(auto &itmap = regCommandMap.begin(); itmap != regCommandMap.end(); itmap++)
					{
						if(itmap->first == it->windowId)
						{
							regCommandMap.erase( itmap++ );
							break;
						}
					}
					// 删除其他记录
					allcreated.erase( it++ );
					ret = true;
					continue;
				}
				it++;
			}
			return ret;
		}
		void deleteAllPart()
		{
			deletePart( "*" );
		}
		// 直接清空当前面板
		void clearDlg()
		{
			deleteAllPart();
			eraseDrewAll();
		}

		//
		// 获取创建过的控件句柄
		HWND getWnd(string partName)
		{
			for(auto &it : allcreated)
			{
				if(it.partname == partName)
					return it.hWnd;
			}
			return 0;
		}

		//
		// 创建控件
		// 调用时带上partName时才可以删除指定的控件  
		// (当然也可以使用默认的partname == "static","button"...)
		//
		// text
		inline bool createText(string content,int x,int y,int width,int height,
			bool isCaption=false,string partName="static")
		{
			return createPart( "STATIC", content, NULL, x, y, width, height, partName, nullptr, isCaption );
		}
		// push-button
		inline bool createbutton( string content, int x, int y, int width, int height, 
			CommandCallback proc = nullptr, string partName = "button" )
		{
			return createPart( "button", content, BS_PUSHBUTTON, x, y, width, height, partName,proc );
		}

		//
		// 画控件 (注意:这样的控件不会保存进allcreated)
		//
		// 删除已经画上去的控件 (仅限drawxxxx上去的)
		// 
		template <typename T>
		void eraseDrewPart( T& infos, string partName )
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
		void eraseDrewAll()
		{
			lineInfos.clear();
			bmpInfos.clear();
			InvalidateRect( hMainDlg, NULL, FALSE );
		}
		// draw line (保存记录后从wm_paint里画的)
		// line =>  sx,sy -> ex,ey
		inline void drawLine( int sx, int sy, int ex, int ey, COLORREF col = RGB( 0, 0, 0 ), string partName = "line" )
		{
			lineInfos.push_back( {partName, col,{sx,sy},{ex,ey}} );
		}
		inline void eraseLine( string partName )
		{
			eraseDrewPart<list<LineInfo>>( lineInfos ,partName);
		}

		// draw images(bmp)
		void drawBmp( string filename,int cx, int cy, int width, int height, string partName = "bmp" )
		{
			bmpInfos.push_back( {partName, filename,cx,cy,width,height} );
		}
		inline void eraseBmp( string partName )
		{
			eraseDrewPart<list<BmpInfo>>( bmpInfos, partName );
		}

		// 主对话框样式
		void setTitle( string title )
		{
			SetWindowTextA( hMainDlg, title.c_str() );
		}
		void setbgcolor( COLORREF col)
		{
			bgColor = col;
			InvalidateRect( hMainDlg, NULL, FALSE );
		}
		void setFontColor( COLORREF col )
		{
			fontColor = col;
			InvalidateRect( hMainDlg, NULL, FALSE );
		}
		void setForecolor( COLORREF col, RECT foreRt = {0} )
		{
			foreColor = col;
			forecolorRect = foreRt;
			InvalidateRect( hMainDlg, NULL, FALSE );
		}
	
		//
		// static回调 (中转)
		static LRESULT CALLBACK staticWndCallback( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
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

	public:
		HWND hMainDlg;
		RECT hMainDlgRect;

	private:
		HINSTANCE appInstance;
		COLORREF bgColor, fontColor,foreColor;
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
		// 记录当前的所有对话框信息,以供调用对应的proc
		static list<ctDialogInfo> ctDialogInfos;
	};

	// 在类外定义类内的static数据
	list<ctDialogInfo> ctDialog::ctDialogInfos;
}


//////////////////////////////////////////////////////////////////////////
ctWin32Dialog::ctDialog ctd;

int CALLBACK page1( HWND hWnd, DWORD windowId );

int CALLBACK page2( HWND hWnd, DWORD windowId )
{
	ctd.clearDlg();

	ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,60} );
	ctd.drawLine( 0, 60, 510, 60, (COLORREF)0xA0A0A0 );
	ctd.createbutton( "< 上一步(B)", 200, 300, 85, 22, page1 );
	ctd.createbutton( "下一步(N) >", 300, 300, 85, 22 );
	ctd.createbutton( "取消", 400, 300, 85, 22 );
	return 0;
}

int CALLBACK page1( HWND hWnd, DWORD windowId )
{
	ctd.clearDlg();

	ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
	ctd.drawBmp( "e:\\WizardImage.bmp", 0, 0, 160, 290 );
	ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,290} );

	ctd.createText( "这个是标题", 200, 30, 200, 22, true );
	ctd.createText( "内容很长很长很长很长", 200, 60, 200, 22 );
	ctd.createbutton( "下一步(N) >", 300, 300, 85, 22, page2 );
	ctd.createbutton( "取消", 400, 300, 85, 22 );
	return 0;
}

int WINAPI WinMain( _In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd )
{
	ctd.createMainDialog( 510, 370 );
	ctd.setTitle( "呼叫地面测试中心" );
	page1(0,0);
	ctd.showMainDialog();
	return 0;
}

