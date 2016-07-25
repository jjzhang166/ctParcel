#pragma once

#include "ctWin32Dialog.h"

namespace ctWin32Wizard
{
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


	// PARTCALLBACK(prc) ����һ��lambda �������ó�Ա�����Ļص�����
	#define PARTCALLBACK(proc) [=](HWND hDlg,DWORD windowId)->int{return proc(hDlg,windowId);}
	// �����ɵ�����exe���ļ��������һ���ṹ,����ṹ�����˱����UIͼƬ/����/ѹ���ļ���
	#define RELATIVEPATH_TMP	".tmp"
	struct fileblock
	{
		char relativepath[150];			// ��ʱ�ļ�=".tmp" or ���·�� : aa/bb/
		char filename[50];				// �ļ���   : a.dll   >> ��ô����ļ���·��: setuppath/aa/bb/a.dll
		DWORD filesize;					// ��С
										//
										// +sizeof(fileblock) ֮������ļ����� byte* filebuf
	};
	struct addedSector
	{
		DWORD verifycode;				// 0

		char compressType[10];			// ѹ����ʽ
		char programName[20];			// ��װ���ĳ�����

		int nfiles;						// ���ļ�����
										//
										// +sizeof(addedSector) ֮�����ÿ���ļ��� fileblock
	};

	/*
	-----------(�涨)-----------

	Ĭ��ʹ�õ�1���ļ�(txt-file)��Ϊ��װʱ���ַ�����ȡ:
	**************************************
	[follow this:]  gb2312

	��װ - %s
	< ��һ��(B)
	��һ��(N) >
	ȡ��
	����
	��װ(I)
	���(F)
	��ӭʹ�� %s ��װ��
	���ڽ���װ %s��
	�������ڼ���֮ǰ�ر�����Ӧ�ó���
	���"��һ��"����,���ߵ��"ȡ��"�˳���װ��
	ѡ��Ŀ��λ��
	%s Ҫ��װ�����
	��װ���򽫰� %s ��װ�������ļ��С�
	��Ҫ����,����"��һ��"�� �����Ҫ��һ���ļ���,����"���"��
	���(R)...
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
	2: WizardImage.bmp
	3: SmallWizardImage.bmp
	4: folders.bmp
	*/

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
				return (addedSector*)(selfpe.filebuf + lastsec->PointerToRawData + lastsec->SizeOfRawData );
			}
			return nullptr;
		}

		//��ȡָ�����ļ���λ��
		fileblock* getFileBlock(int ifile)
		{
			if(ifile < addedsec->nfiles)
			{
				//�ȵ���һ��
				fileblock* fb = (fileblock*)((DWORD)addedsec + sizeof( addedSector ));
				//һֱ������ָ����λ��
				for(int i = 0; i < ifile; i++)
					fb = (fileblock*)((DWORD)fb + fb->filesize + sizeof( fileblock ));
				return fb;
			}
			return nullptr;
		}

		//�ͷ��ļ���ָ��λ��
		void outputFile(int ifile)
		{
			fileblock* fb = getFileBlock( ifile );
			if(fb)
			{
				char extractPath[260] = {0};
				if(strcmp( fb->relativepath, ".tmp" ) == 0)
				{
					//��ʱ�ļ�,ֱ���ͷŵ���ʱĿ¼
					GetTempPathA( 260, extractPath );
					lstrcatA( extractPath, fb->filename );
				}

				byte* filebuf = (byte*)((DWORD)fb + sizeof( fileblock ));
				FILE *f = nullptr;
				fopen_s( &f, extractPath, "wb" );
				if(f)
				{
					fwrite( filebuf, 1, fb->filesize, f );
					fclose( f );
				}

				//MessageBoxA( 0, extractPath, 0, 0 );
			}
		}

		//��temp path����ļ�·��
		std::string getTempFile( char* fileName )
		{
			char tmp[260];
			GetTempPathA( 260, tmp );
			lstrcatA( tmp, fileName );
			return std::string( tmp );
		}
		#define  TMP_WizardImage		getTempFile("WizardImage.bmp").c_str()
		#define  TMP_WizardSmallImage	getTempFile("WizardSmallImage.bmp").c_str()
		#define  TMP_folders			getTempFile("folders.bmp").c_str()
		#define  TMP_str				getTempFile("str.txt").c_str()
		
		//����ʱ�ļ�����ȥ��ȡstr.txtָ��������
		//��programName���и�ʽ��
		std::string getString( int snum )
		{
			char tmp[200] = {0}, tmpall[300] = {0};
			FILE* f = nullptr;
			fopen_s( &f, TMP_str, "rt" );
			if(f)
			{
				int i = 0;
				while(fgets( tmp, 200, f ))
				{
					if(++i == snum)
						break;
				}
				if(tmp[0])
					wsprintfA( tmpall,(PCSTR)tmp,addedsec->programName );
				fclose( f );
			}
			return std::string( tmpall );
		}
		#define GETSETUPSTRING(i) getString(i).c_str()


		//��ʼ����Ҫ��װ��һЩ��Ϣ
		void initSetupInfomation()
		{
			addedsec = getAddedSector();

			//�ͷ���ʱ�ļ�����ʱ�ļ���
			for(int i=0;i<4;i++)
				outputFile(i);
		}

		//////////////////////////////////////////////////////////////////////////
		// UI
	public:
		ctWizard() : step(0)
		{
			initSetupInfomation();
			start();
		}
		
		void start(int w=510,int h=370)
		{
			ctd.createMainDialog( w, h );
			ctd.setTitle( GETSETUPSTRING(1) );
			page1( ctd.hMainDlg, 0 );
			
			ctd.showMainDialog();
		}
		int CALLBACK page1( HWND hDlg, DWORD windowId )
		{
			step = 1;
			ctd.clearDlg();

			ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
			ctd.drawBmp( TMP_WizardImage, 0, 0, 160, 290 );
			ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,290} );
			ctd.createText( GETSETUPSTRING(8), 180, 20, 300, 40, 16 );
			ctd.createText( GETSETUPSTRING(9),180,65,300,25);
			ctd.createText( GETSETUPSTRING(10), 180, 90, 300, 25 );
			ctd.createText( GETSETUPSTRING(11), 180, 115, 300, 25 );

			ctd.createbutton( GETSETUPSTRING(3), 300, 300, PARTCALLBACK( page2 ) );
			ctd.createbutton( GETSETUPSTRING(4), 400, 300, PARTCALLBACK( cancel ) );
			return 0;
		}
		int CALLBACK page2( HWND hDlg, DWORD windowId )
		{
			step = 2;
			ctd.clearDlg();

			ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,60} );
			ctd.drawLine( 0, 60, 510, 60, (COLORREF)0xA0A0A0 );
			ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
			ctd.drawBmp( TMP_WizardSmallImage, 435, 0, 55, 55 );
			ctd.createText( GETSETUPSTRING(12), 20, 15, 200, 15, 12 );
			ctd.createText( GETSETUPSTRING( 13 ), 40, 35, 300, 15 );
			ctd.drawBmp( TMP_folders, 40, 75, 36, 36 );
			ctd.createText( GETSETUPSTRING( 14 ), 90, 85, 350, 15 );
			ctd.createText( GETSETUPSTRING( 15 ), 40, 120, 400, 15 );
			ctd.createEdit( 40, 145, 300,20 , "path","c:\\testPath");
			ctd.createbutton( GETSETUPSTRING( 16 ), 350, 144, PARTCALLBACK( choosefile ) );
			ctd.createbutton( GETSETUPSTRING(2), 200, 300, PARTCALLBACK( page1 ) );
			ctd.createbutton( GETSETUPSTRING(3), 300, 300, PARTCALLBACK( page3 ) );
			ctd.createbutton( GETSETUPSTRING(4), 400, 300, PARTCALLBACK( cancel ) );
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
			ctd.drawBmp( TMP_WizardSmallImage, 435, 0, 55, 55 );
			ctd.createText( GETSETUPSTRING(17), 20, 15, 200, 15, 12 );
			ctd.createText( GETSETUPSTRING( 18 ), 40, 35, 200, 15 );
			ctd.createText( GETSETUPSTRING( 19 ), 40, 75, 400, 15 );
			ctd.createEdit( 40, 100, 400, 170, "lastshow" );
			PostMessageW( ctd.getWnd("lastshow"), EM_SETREADONLY, 1, 0 );		//����Ϊֻ��
			ctd.setEditText( "lastshow", "Ŀ��λ��:\r\n\tc:\\testSetup" );
			ctd.createbutton( GETSETUPSTRING(2), 200, 300, PARTCALLBACK( page2 ) );
			ctd.createbutton( GETSETUPSTRING(6), 300, 300, PARTCALLBACK( page4 ) );
			ctd.createbutton( GETSETUPSTRING(4), 400, 300, PARTCALLBACK( cancel ) );
			return 0;
		}
		int CALLBACK page4( HWND hDlg, DWORD windowId )
		{
			step = 4;
			ctd.clearDlg();

			ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,60} );
			ctd.drawLine( 0, 60, 510, 60, (COLORREF)0xA0A0A0 );
			ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
			ctd.drawBmp( TMP_WizardSmallImage, 435, 0, 55, 55 );
			ctd.createText( GETSETUPSTRING( 20 ), 20, 15, 200, 15, 12 );
			ctd.createText( GETSETUPSTRING( 21 ), 40, 35, 300, 15 );
			ctd.createText( GETSETUPSTRING( 22 ), 40, 75, 400, 15 );
			ctd.createbutton( GETSETUPSTRING(3), 400, 300, PARTCALLBACK( cancel ) );
			
			//����������ý���Ϊ�ļ�����,ÿ���ͷ�һ���ļ� +1���� tmp=100
			ctd.createProgress( 40, 120, "extract",100 );
			std::thread t1( &ctWizard::setupInfo ,this);
			t1.detach();

			//����һ�����ذ�ť,����ͨ������page5
			ctd.createbutton( GETSETUPSTRING(5), 300, 300, PARTCALLBACK( page5 ),85,22,"overbutton" );
			ShowWindow( ctd.getWnd( "overbutton" ), 0 );
			return 0;
		}
		int CALLBACK page5( HWND hDlg, DWORD windowId )
		{
			step = 5;
			ctd.clearDlg();

			ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
			ctd.drawBmp( TMP_WizardImage, 0, 0, 160, 290 );
			ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,290} );
			ctd.createText( GETSETUPSTRING(23), 180, 20, 300, 22, 16 );
			ctd.createText( GETSETUPSTRING(24), 180, 50, 300, 30 );
			ctd.createText( GETSETUPSTRING(25), 180, 85, 300, 30 );
			ctd.createCheckbox( GETSETUPSTRING(26), 180, 111 );

			ctd.createbutton( GETSETUPSTRING(7), 300, 300,PARTCALLBACK( end ) );
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
			std::string msg = GETSETUPSTRING(28);
			msg += "\n";
			msg += GETSETUPSTRING(29);

			if(MessageBoxA( hDlg, msg.c_str(), GETSETUPSTRING(27), MB_ICONQUESTION | MB_YESNO ) == IDYES)
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