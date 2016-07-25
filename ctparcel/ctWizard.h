#pragma once

#include "ctWin32Dialog.h"

namespace ctWin32Wizard
{
	// PARTCALLBACK(prc) ����һ��lambda �������ó�Ա�����Ļص�����
	#define PARTCALLBACK(proc) [=](HWND hDlg,DWORD windowId)->int{return proc(hDlg,windowId);}

	// �����ɵ�����exe���ļ��������һ���ṹ,����ṹ�����˱����UIͼƬ/����/ѹ���ļ���
	struct fileblock
	{
		char relativepath[80];			// ��ʱ�ļ�="tmp" or ���·�� : aa
		char filename[20];				// �ļ���   : a.dll   >> ��ô����ļ���·��: setuppath/aa/a.dll
		DWORD filesize;					// ��С
		byte* filebuf;					// ����
	};

	struct addedSector
	{
		DWORD verifycode;			//
		char compressType[10];		// ѹ����ʽ
		char programName[20];		// ��װ���ĳ�����

		int nfiles;
		fileblock fileblocks[1];	// �ļ��б�
	};


	/*
	[�ļ�]
	ÿ���ļ�ӵ��һ���ṹ
	struct fileblock
	{
		char relativepath[80];			// ���·�� : aa
		char filename[20];				// �ļ���   : a.dll   >> ��ô����ļ���·��: setuppath/aa/a.dll
		DWORD filesize;					// ��С
		byte * filebuf;					// ����
	};
	������fileblock˳�����һ����ڴ���
	fileblock[xx]

	-----------(�涨)-----------

	Ĭ��ʹ�õ�1���ļ�(txt-file)��Ϊ��װʱ���ַ�����ȡ:
	**************************************
	[follow this:]

	��װ - %s
	< ��һ��(B)
	��һ��(N) >
	ȡ��
	����
	���(F)
	��ӭʹ�� %s ��װ��
	���ڽ���װ %s��
	�������ڼ���֮ǰ�ر�����Ӧ�ó���
	���"��һ��"����,���ߵ��"ȡ��"�˳���װ��
	ѡ��Ŀ��λ��
	%s Ҫ��װ�����
	��װ���򽫰� %s ��װ�������ļ��С�
	��Ҫ����,����"��һ��"�� �����Ҫ��һ���ļ���,����"���"��
	���(R)������
	׼����װ
	��װ����׼������ĵ����ϰ�װ��
	���"��װ"����,��������޸���������"��һ��"��
	���ڰ�װ
	���ڰ�װ %s, ���Ժ󡣡���
	������ȡ�ļ�������
	%s ��װ�����
	��װ�����������ĵ����а�װ�� %s �� ��Ӧ�ó������ͨ��ѡ��װ�Ŀ�ݷ�ʽ���С�
	����'���'�˳���װ����
	���� %s
	�˳���װ
	��װδ���!����������˳�,�����޷���ɣ�
	Ҫ�˳���װ��

	**************************************

	Ĭ��ʹ�õ�2,3,4���ļ���Ϊ3����װʱ��ͼƬ:
	WizardImage.bmp
	SmallWizardImage.bmp
	folders.bmp
	*/



	//
	// simple load pe class
	//	
	class PEFile
	{
	public:
		byte* filebuf;
		PIMAGE_DOS_HEADER dosheader;
		PIMAGE_NT_HEADERS ntheader;
		PIMAGE_SECTION_HEADER secheader;
		int seccount;

		PEFile() : filebuf( NULL )
		{}
		~PEFile()
		{
			if(filebuf)
				delete filebuf;
		}

		bool init( byte *infilebuf )
		{
			dosheader = (PIMAGE_DOS_HEADER)infilebuf;
			if(dosheader->e_magic == 0x5A4D)
			{
				ntheader = (PIMAGE_NT_HEADERS)(infilebuf + (DWORD)(dosheader->e_lfanew));
				secheader = (PIMAGE_SECTION_HEADER)(((DWORD)ntheader) + sizeof( IMAGE_NT_HEADERS ));
				seccount = ntheader->FileHeader.NumberOfSections;
				if(ntheader->Signature == IMAGE_NT_SIGNATURE)
				{
					filebuf = infilebuf;
					return true;
				}
			}
			return false;
		}

		// load file
		bool loadfile( const char* filename )
		{
			FILE * f = nullptr;
			fopen_s( &f, filename, "rb" );
			if(f)
			{
				fseek( f, 0, SEEK_END );
				int filelen = ftell( f );
				filebuf = new byte[filelen];
				fseek( f, 0, SEEK_SET );
				fread( filebuf, filelen, 1, f );
				fclose( f );

				if(init( filebuf ))
					return true;
			}
			return false;
		}
	};



	//
	// Wizard��
	//
	class ctWizard
	{
	public:
		PEFile selfpe;

		//////////////////////////////////////////////////////////////////////////
		// setup����
		// ��ȡ����ӵ��ڴ�����λ��
		addedSector* getAddedSector()
		{
			char tmp[260];
			GetModuleFileNameA( NULL, tmp, 260 );
			if(selfpe.loadfile( tmp ))
			{
				//�����һ�����εõ�֮ǰ�ļ��Ĵ�С,�����ľ�����ӵ�������
				PIMAGE_SECTION_HEADER lastsec = selfpe.secheader + selfpe.seccount -1;
				return (addedSector*)( lastsec->PointerToRawData + lastsec->SizeOfRawData );
			}
			return nullptr;
		}

		//////////////////////////////////////////////////////////////////////////
		// UI
	public:
		ctWizard() : step(0)
		{
			addedsec = getAddedSector();
			char tmp[200];
			wsprintfA( tmp, "%X", addedsec );
			MessageBoxA( 0, tmp, 0, 0 );

			start();
		}
		
		void start(int w=510,int h=370)
		{
			ctd.createMainDialog( w, h );
			ctd.setTitle( "��װ - %s" );
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
			ctd.createText( "��ӭʹ�� %s ��װ��", 180, 20, 300, 22, 16 );
			ctd.createText( "���ڽ���װ %s",180,50,300,25);
			ctd.createText( "�������ڼ���֮ǰ�ر�����Ӧ�ó���.", 180, 75, 300, 25 );
			ctd.createText( "���\"��һ��\"����,���ߵ��\"ȡ��\"�˳���װ.", 180, 100, 300, 25 );

			ctd.createbutton( "��һ��(N) >", 300, 300, PARTCALLBACK( page2 ) );
			ctd.createbutton( "ȡ��", 400, 300, PARTCALLBACK( cancel ) );
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
			ctd.createText( "%s Ҫ��װ������?", 40, 35, 200, 15 );
			ctd.drawBmp( "e:\\folders.bmp", 40, 75, 36, 36 );
			ctd.createText( "��װ���򽫰� %s ��װ�������ļ���.", 90, 85, 200, 15 );
			ctd.createText( "��Ҫ����,����\"��һ��\". �����Ҫ��һ���ļ���,����\"���\".", 40, 120, 400, 15 );
			ctd.createEdit( 40, 145, 300,20 , "path","c:\\testPath");
			ctd.createbutton( "���(R)...", 350, 144, PARTCALLBACK( choosefile ) );
			ctd.createbutton( "< ��һ��(B)", 200, 300, PARTCALLBACK( page1 ) );
			ctd.createbutton( "��һ��(N) >", 300, 300, PARTCALLBACK( page3 ) );
			ctd.createbutton( "ȡ��", 400, 300, PARTCALLBACK( cancel ) );
			return 0;
		}
		int CALLBACK page3( HWND hDlg, DWORD windowId )
		{
			step = 3;
			setuppath = ctd.getEditText( "path" );		//�ȶ�ȡ֮ǰpage2�е�path
			ctd.clearDlg();

			ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,60} );
			ctd.drawLine( 0, 60, 510, 60, (COLORREF)0xA0A0A0 );
			ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
			ctd.drawBmp( "e:\\WizardSmallImage.bmp", 435, 0, 55, 55 );
			ctd.createText( "׼����װ", 20, 15, 200, 15, 12 );
			ctd.createText( "��װ����׼������ĵ����ϰ�װ.", 40, 35, 200, 15 );
			ctd.createText( "���\"��װ\"����,��������޸���������\"��һ��\".", 40, 75, 400, 15 );
			ctd.createEdit( 40, 100, 400, 170, "lastshow" );
			PostMessageW( ctd.getWnd("lastshow"), EM_SETREADONLY, 1, 0 );		//����Ϊֻ��
			ctd.setEditText( "lastshow", "Ŀ��λ��:\r\n\tc:\\testSetup" );
			ctd.createbutton( "< ��һ��(B)", 200, 300, PARTCALLBACK( page2 ) );
			ctd.createbutton( "��װ(I)", 300, 300, PARTCALLBACK( page4 ) );
			ctd.createbutton( "ȡ��", 400, 300, PARTCALLBACK( cancel ) );
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
			ctd.createText( "���ڰ�װ", 20, 15, 200, 15, 12 );
			ctd.createText( "���ڰ�װ %s, ���Ժ�...", 40, 35, 200, 15 );
			ctd.createText( "������ȡ�ļ�...", 40, 75, 400, 15 );
			ctd.createbutton( "ȡ��", 400, 300, PARTCALLBACK( cancel ) );
			
			//����������ý���Ϊ�ļ�����,ÿ���ͷ�һ���ļ� +1���� tmp=100
			ctd.createProgress( 40, 120, "extract",100 );
			std::thread t1( &ctWizard::setupInfo ,this);
			t1.detach();

			//����һ�����ذ�ť,����ͨ������page5
			ctd.createbutton( "����", 300, 300, PARTCALLBACK( page5 ),85,22,"overbutton" );
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
			ctd.createText( "%s ��װ�����", 180, 20, 300, 22, 16 );
			ctd.createText( "��װ�����������ĵ����а�װ�� %s . ��Ӧ�ó������ͨ��ѡ��װ�Ŀ�ݷ�ʽ����.", 180, 50, 300, 30 );
			ctd.createText( "����'���'�˳���װ����.", 180, 85, 300, 30 );
			ctd.createCheckbox( "���� %s", 180, 111 );

			ctd.createbutton( "���(F)", 300, 300,PARTCALLBACK( end ) );
			return 0;
		}


		//
		// UI����
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
			std::string msg = "��װδ���!����������˳�,�����޷����!";
			msg += "\n\n";
			msg += "Ҫ�˳���װ��?";

			if(MessageBoxA( hDlg, msg.c_str(),"�˳���װ", MB_ICONQUESTION | MB_YESNO ) == IDYES)
			{
				PostQuitMessage( 0 );
			}
			return 0;
		}
		//////////////////////////////////////////////////////////////////////////


	private:
		unsigned int step;
		std::string setuppath;
		ctWin32Dialog::ctDialog ctd;
		addedSector* addedsec;
	};
}