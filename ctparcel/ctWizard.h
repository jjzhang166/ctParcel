#pragma once

#include "ctWin32Dialog.h"

namespace ctWin32Wizard
{
	// PARTCALLBACK(prc) 生成一个lambda 用来调用成员函数的回调函数
	#define PARTCALLBACK(proc) [=](HWND hDlg,DWORD windowId)->int{return proc(hDlg,windowId);}

	class ctWizard
	{
	public:
		ctWizard() : step(0)
		{
			createMain();
		}

		void createMain(int w=510,int h=370)
		{
			ctd.createMainDialog( w, h );
			ctd.setTitle( "呼叫地面测试中心" );
			page1( 0, 0 );
			ctd.showMainDialog();
		}
		int CALLBACK page1( HWND hDlg, DWORD windowId )
		{
			step = 1;
			ctd.clearDlg();

			ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
			ctd.drawBmp( "e:\\WizardImage.bmp", 0, 0, 160, 290 );
			ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,290} );

			ctd.createText( "欢迎使用 %s 安装向导", 180, 20, 300, 22, 16 );
			ctd.createText( "现在将安装 %s.\n\n建议你在继续之前关闭其他应用程序.\n\n点击\"下一步\"继续,或者点击\"取消\"退出安装.", 180, 50, 300, 100 );

			ctd.createbutton( "下一步(N) >", 300, 300,PARTCALLBACK(page2) );
			ctd.createbutton( "取消", 400, 300,  PARTCALLBACK( cancel ) );
			return 0;
		}
		int CALLBACK page2( HWND hDlg, DWORD windowId )
		{
			step = 2;
			ctd.clearDlg();

			ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,60} );
			ctd.drawLine( 0, 60, 510, 60, (COLORREF)0xA0A0A0 );
			ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
			ctd.drawBmp( "e:\\WizardSmallImage.bmp", 435, 0, 55, 55 );
			
			ctd.createText( "选择目标位置", 20, 15, 200, 15, 12 );
			ctd.createText( "xxxxxxxx 要安装到哪里?", 40, 35, 200, 15 );

			ctd.drawBmp( "e:\\folders.bmp", 40, 75, 36, 36 );
			ctd.createText( "安装程序将把 %s 安装到以下文件夹.", 90, 85, 200, 15 );
			ctd.createText( "若要继续,请点击\"下一步\". 如果你要换一个文件夹,请点击\"浏览\".", 40, 120, 400, 15 );
			ctd.createEdit( 40, 145, 300,20 , "path","c:\\testPath");
			ctd.createbutton( "浏览(R)...", 350, 144 );

			ctd.createbutton( "< 上一步(B)", 200, 300, PARTCALLBACK( page1 ) );
			ctd.createbutton( "下一步(N) >", 300, 300, PARTCALLBACK( page3 ) );
			ctd.createbutton( "取消", 400, 300, PARTCALLBACK( cancel ) );
			return 0;
		}
		int CALLBACK page3( HWND hDlg, DWORD windowId )
		{
			step = 3;
			ctd.clearDlg();

			ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,60} );
			ctd.drawLine( 0, 60, 510, 60, (COLORREF)0xA0A0A0 );
			ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
			ctd.drawBmp( "e:\\WizardSmallImage.bmp", 435, 0, 55, 55 );

			ctd.createText( "选择开始菜单文件夹", 20, 15, 200, 15, 12 );
			ctd.createText( "程序的快捷方式要安装到哪里?", 40, 35, 200, 15 );

			ctd.drawBmp( "e:\\folders.bmp", 40, 75, 36, 36 );
			ctd.createText( "安装程序将在以下开始菜单文件夹中创建程序的快捷方式.", 90, 85, 400, 15 );
			ctd.createText( "若要继续,请点击\"下一步\". 如果你要换一个文件夹,请点击\"浏览\".", 40, 120, 400, 15 );
			ctd.createEdit( 40, 145, 300, 20, "path", "Test" );
			ctd.createbutton( "浏览(R)...", 350, 144 );

			ctd.createbutton( "< 上一步(B)", 200, 300, PARTCALLBACK( page2 ) );
			ctd.createbutton( "安装(I)", 300, 300 );
			ctd.createbutton( "取消", 400, 300, PARTCALLBACK( cancel ) );
			return 0;
		}

		// 小模块
		int CALLBACK cancel( HWND hDlg, DWORD windowId )
		{
			if(MessageBoxA( hDlg, "安装未完成!如果你现在退出,程序将无法完成!\n\n要退出安装吗?", 
				"退出安装", MB_ICONQUESTION | MB_YESNO ) == IDYES)
			{
				PostQuitMessage( 0 );
			}
			return 0;
		}

	private:
		unsigned int step;
		ctWin32Dialog::ctDialog ctd;
	};
}