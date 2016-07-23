// ctparcel.cpp : �������̨Ӧ�ó������ڵ㡣
//

//����ΪWindows�µ��Ӿ�
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
	ctd.createbutton( "< ��һ��(B)", 200, 300, 85, 22, page1 );
	ctd.createbutton( "��һ��(N) >", 300, 300, 85, 22 );
	ctd.createbutton( "ȡ��", 400, 300, 85, 22 );
	return 0;
}

int CALLBACK page1( HWND hDlg, DWORD windowId )
{
	ctd.clearDlg();

	ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
	ctd.drawBmp( "e:\\WizardImage.bmp", 0, 0, 160, 290 );
	ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,290} );

	ctd.createText( "����Ǳ���", 200, 30, 200, 22, true );
	ctd.createText( "���ݺܳ��ܳ��ܳ��ܳ�", 200, 60, 200, 22 );
	ctd.createbutton( "��һ��(N) >", 300, 300, 85, 22, page2 );
	ctd.createbutton( "ȡ��", 400, 300, 85, 22,
		[]( HWND hDlg, DWORD windowId )->int {MessageBoxA( hDlg, "����ȡ����ť", 0, 0 ); return 0; } );
	return 0;
}

void start( )
{
	ctd.createMainDialog( 510, 370 );
	ctd.setTitle( "���е����������" );
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
	ctd2->setTitle( "���е����������2" );
	ctd2->createText( "����Ǳ���2", 200, 30, 200, 22, true );
	ctd2->createText( "���ݺܳ��ܳ��ܳ��ܳ�2", 200, 60, 200, 22 );
	ctd2->showMainDialog();
	return 0;
}

