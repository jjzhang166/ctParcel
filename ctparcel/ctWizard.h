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
			start();
		}

		void start(int w=510,int h=370)
		{
			ctd.createMainDialog( w, h );
			ctd.setTitle( "地面测试中心" );
			page1( ctd.hMainDlg, 0 );
			
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
			ctd.createbutton( "下一步(N) >", 300, 300, PARTCALLBACK( page2 ) );
			ctd.createbutton( "取消", 400, 300, PARTCALLBACK( cancel ) );
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
			ctd.createbutton( "浏览(R)...", 350, 144, PARTCALLBACK( choosefile ) );
			ctd.createbutton( "< 上一步(B)", 200, 300, PARTCALLBACK( page1 ) );
			ctd.createbutton( "下一步(N) >", 300, 300, PARTCALLBACK( page3 ) );
			ctd.createbutton( "取消", 400, 300, PARTCALLBACK( cancel ) );
			return 0;
		}
		int CALLBACK page3( HWND hDlg, DWORD windowId )
		{
			step = 3;
			setuppath = ctd.getEditText( "path" );		//先读取之前page2中的path
			ctd.clearDlg();

			ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,60} );
			ctd.drawLine( 0, 60, 510, 60, (COLORREF)0xA0A0A0 );
			ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
			ctd.drawBmp( "e:\\WizardSmallImage.bmp", 435, 0, 55, 55 );
			ctd.createText( "准备安装", 20, 15, 200, 15, 12 );
			ctd.createText( "安装程序准备在你的电脑上安装.", 40, 35, 200, 15 );
			ctd.createText( "点击\"安装\"继续,如果你想修改设置请点击\"上一步\".", 40, 75, 400, 15 );
			ctd.createEdit( 40, 100, 400, 170, "lastshow" );
			PostMessageW( ctd.getWnd("lastshow"), EM_SETREADONLY, 1, 0 );		//设置为只读
			ctd.setEditText( "lastshow", "目标位置:\r\n\tc:\\testSetup" );
			ctd.createbutton( "< 上一步(B)", 200, 300, PARTCALLBACK( page2 ) );
			ctd.createbutton( "安装(I)", 300, 300, PARTCALLBACK( page4 ) );
			ctd.createbutton( "取消", 400, 300, PARTCALLBACK( cancel ) );
			return 0;
		}
		int CALLBACK page4( HWND hDlg, DWORD windowId )
		{
			step = 4;
			ctd.clearDlg();

			ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,60} );
			ctd.drawLine( 0, 60, 510, 60, (COLORREF)0xA0A0A0 );
			ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
			ctd.drawBmp( "e:\\WizardSmallImage.bmp", 435, 0, 55, 55 );
			ctd.createText( "正在安装", 20, 15, 200, 15, 12 );
			ctd.createText( "正在安装 %s, 请稍后...", 40, 35, 200, 15 );
			ctd.createText( "正在提取文件...", 40, 75, 400, 15 );
			ctd.createbutton( "取消", 400, 300, PARTCALLBACK( cancel ) );
			
			//这里可以设置进度为文件数量,每次释放一个文件 +1进度
			ctd.createProgress( 40, 120, "extract",100 );
			std::thread t1( &ctWizard::setupInfo ,this);
			t1.detach();

			//创建一个隐藏按钮,可以通过他到page5
			ctd.createbutton( "继续", 300, 300, PARTCALLBACK( page5 ),85,22,"overbutton" );
			ShowWindow( ctd.getWnd( "overbutton" ), 0 );
			return 0;
		}
		int CALLBACK page5( HWND hDlg, DWORD windowId )
		{
			step = 5;
			ctd.clearDlg();

			ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
			ctd.drawBmp( "e:\\WizardImage.bmp", 0, 0, 160, 290 );
			ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,290} );
			ctd.createText( "%s 安装向导完成", 180, 20, 300, 22, 16 );
			ctd.createText( "安装程序已在您的电脑中安装了 %s . 此应用程序可以通过选择安装的快捷方式运行.\n\n单击'完成'退出安装程序.", 180, 50, 300, 200 );
			ctd.createCheckbox( "运行 %s", 180, 111 );

			ctd.createbutton( "完成(F)", 300, 300,PARTCALLBACK( end ) );
			return 0;
		}


		//
		// 功能
		//
		int CALLBACK end( HWND hDlg, DWORD windowId )
		{
			PostQuitMessage( 0 );
			return 0;
		}
		void setupInfo()
		{
			for(int i=0; i < 100; i++)
			{
				ctd.setProgressPos( "extract", i );
				Sleep( 10 );
			}

			SendMessageA( ctd.getWnd( "overbutton" ), BM_CLICK, 0, 0 );
		}
		int CALLBACK choosefile( HWND hDlg, DWORD windowId )
		{
			ctd.setEditText( "path", ctd.chooseFolders() );
			return 0;
		}
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
		std::string setuppath;
		ctWin32Dialog::ctDialog ctd;
	};
}