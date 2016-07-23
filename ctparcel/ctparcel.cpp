// ctparcel.cpp : 定义控制台应用程序的入口点。
//

//链接为Windows新的视觉
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


#include <thread>
#include "ctWin32Dialog.h"


//////////////////////////////////////////////////////////////////////////
ctWin32Dialog::ctDialog ctd;

int CALLBACK page1( HWND hDlg, DWORD windowId );

int CALLBACK page2( HWND hDlg, DWORD windowId )
{
	ctd.clearDlg();

	ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,60} );
	ctd.drawLine( 0, 60, 510, 60, (COLORREF)0xA0A0A0 );
	ctd.createbutton( "< 上一步(B)", 200, 300, 85, 22, page1 );
	ctd.createbutton( "下一步(N) >", 300, 300, 85, 22 );
	ctd.createbutton( "取消", 400, 300, 85, 22 );
	return 0;
}

int CALLBACK page1( HWND hDlg, DWORD windowId )
{
	ctd.clearDlg();

	ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
	ctd.drawBmp( "e:\\WizardImage.bmp", 0, 0, 160, 290 );
	ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,290} );

	ctd.createText( "这个是标题", 200, 30, 200, 22, true );
	ctd.createText( "内容很长很长很长很长", 200, 60, 200, 22 );
	ctd.createbutton( "下一步(N) >", 300, 300, 85, 22, page2 );
	ctd.createbutton( "取消", 400, 300, 85, 22,
		[]( HWND hDlg, DWORD windowId )->int {MessageBoxA( hDlg, "按了取消按钮", 0, 0 ); return 0; } );
	return 0;
}

void start( )
{
	ctd.createMainDialog( 510, 370 );
	ctd.setTitle( "呼叫地面测试中心" );
	page1( 0, 0 );
	ctd.showMainDialog();
}

int WINAPI WinMain( _In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd )
{
	std::thread t1(start);
	t1.detach();

	auto ctd2 = new ctWin32Dialog::ctDialog();
	ctd2->createMainDialog( 510, 370 );
	ctd2->setTitle( "呼叫地面测试中心2" );
	ctd2->createText( "这个是标题2", 200, 30, 200, 22, true );
	ctd2->createText( "内容很长很长很长很长2", 200, 60, 200, 22 );
	ctd2->showMainDialog();
	return 0;
}

