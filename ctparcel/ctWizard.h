#pragma once

#include "ctWin32Dialog.h"

namespace ctWin32Wizard
{
	// PARTCALLBACK(prc) ����һ��lambda �������ó�Ա�����Ļص�����
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
			ctd.setTitle( "���е����������" );
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

			ctd.createText( "��ӭʹ�� %s ��װ��", 180, 20, 300, 22, 16 );
			ctd.createText( "���ڽ���װ %s.\n\n�������ڼ���֮ǰ�ر�����Ӧ�ó���.\n\n���\"��һ��\"����,���ߵ��\"ȡ��\"�˳���װ.", 180, 50, 300, 100 );

			ctd.createbutton( "��һ��(N) >", 300, 300,PARTCALLBACK(page2) );
			ctd.createbutton( "ȡ��", 400, 300,  PARTCALLBACK( cancel ) );
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
			
			ctd.createText( "ѡ��Ŀ��λ��", 20, 15, 200, 15, 12 );
			ctd.createText( "xxxxxxxx Ҫ��װ������?", 40, 35, 200, 15 );

			ctd.drawBmp( "e:\\folders.bmp", 40, 75, 36, 36 );
			ctd.createText( "��װ���򽫰� %s ��װ�������ļ���.", 90, 85, 200, 15 );
			ctd.createText( "��Ҫ����,����\"��һ��\". �����Ҫ��һ���ļ���,����\"���\".", 40, 120, 400, 15 );
			ctd.createEdit( 40, 145, 300,20 , "path","c:\\testPath");
			ctd.createbutton( "���(R)...", 350, 144 );

			ctd.createbutton( "< ��һ��(B)", 200, 300, PARTCALLBACK( page1 ) );
			ctd.createbutton( "��һ��(N) >", 300, 300, PARTCALLBACK( page3 ) );
			ctd.createbutton( "ȡ��", 400, 300, PARTCALLBACK( cancel ) );
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

			ctd.createText( "ѡ��ʼ�˵��ļ���", 20, 15, 200, 15, 12 );
			ctd.createText( "����Ŀ�ݷ�ʽҪ��װ������?", 40, 35, 200, 15 );

			ctd.drawBmp( "e:\\folders.bmp", 40, 75, 36, 36 );
			ctd.createText( "��װ���������¿�ʼ�˵��ļ����д�������Ŀ�ݷ�ʽ.", 90, 85, 400, 15 );
			ctd.createText( "��Ҫ����,����\"��һ��\". �����Ҫ��һ���ļ���,����\"���\".", 40, 120, 400, 15 );
			ctd.createEdit( 40, 145, 300, 20, "path", "Test" );
			ctd.createbutton( "���(R)...", 350, 144 );

			ctd.createbutton( "< ��һ��(B)", 200, 300, PARTCALLBACK( page2 ) );
			ctd.createbutton( "��װ(I)", 300, 300 );
			ctd.createbutton( "ȡ��", 400, 300, PARTCALLBACK( cancel ) );
			return 0;
		}

		// Сģ��
		int CALLBACK cancel( HWND hDlg, DWORD windowId )
		{
			if(MessageBoxA( hDlg, "��װδ���!����������˳�,�����޷����!\n\nҪ�˳���װ��?", 
				"�˳���װ", MB_ICONQUESTION | MB_YESNO ) == IDYES)
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