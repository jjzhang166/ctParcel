#pragma once

#include <shlobj.h>  
#include "ctWin32Dialog.h"
#include "unzip.h"
#include "resource.h"

namespace ctCommon
{
    //�����ļ�
    void Runexe( char* runpath, char* dir )
    {
        PROCESS_INFORMATION backpi;
        STARTUPINFO si;
        memset( &si, 0, sizeof( si ) );
        memset( &backpi, 0, sizeof( backpi ) );
        si.cb = sizeof( si );
        CreateProcessA( 0, runpath, 0, 0, 0, 0, 0, dir, &si, &backpi );
        CloseHandle( backpi.hProcess );
    }
    //�õ���ǰprogram·��
    BOOL GetProgramPath( char *pszPath )
    {
        return SHGetSpecialFolderPathA( NULL, pszPath, CSIDL_PROGRAM_FILES, FALSE );
    }
    //�õ���ǰ����·��
    BOOL GetDesktopPath( char *pszPath )
    {
        return SHGetSpecialFolderPathA( NULL, pszPath, CSIDL_DESKTOP, FALSE );
    }
    //���������ݷ�ʽ
    bool CreateDesktopShotCut( std::string shotcutName, std::string exeName, std::string strSourcePath )
    {
        if(FAILED( CoInitialize( NULL ) ))
            return FALSE;

        char Path[MAX_PATH];
        std::string strDestDir;

        //���������ݷ�ʽ������
        GetDesktopPath( Path );
        strDestDir = Path;
        strDestDir += "\\";
        strDestDir += shotcutName;
        strDestDir += ".lnk";

        IShellLink* psl;
        if(SUCCEEDED( CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl ) ))
        {
            //���ÿ�ݷ�ʽ����ʼλ�� 
            //����Ŀ��λ��ΪC:\windows\a.txt ��ʼλ�þ�Ӧ������ΪC:\windows����ᵼ�²���Ԥ�ϵĴ���
            //������ļ��еĿ�ݷ�ʽ��ʼλ�ú�Ŀ��λ�ÿ�������Ϊһ��
            psl->SetWorkingDirectory( strSourcePath.c_str() );
            //���ÿ�ݷ�ʽ��Ŀ��λ�� 
            psl->SetPath( (strSourcePath + "\\" + exeName).c_str() );

            //�����ݷ�ʽ������ 
            IPersistFile* ppf;
            psl->QueryInterface( IID_IPersistFile, (void**)&ppf );
            WCHAR wsz[MAX_PATH];
            MultiByteToWideChar( CP_THREAD_ACP, MB_PRECOMPOSED, (LPCTSTR)strDestDir.c_str(), -1, wsz, MAX_PATH );
            ppf->Save( wsz, TRUE );
            return true;
        }

        CoUninitialize();
        return false;;
    }
}


namespace ctPEFile
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

        PEFile() : filebuf( nullptr )
        {}
        ~PEFile()
        {
            if(filebuf)
                delete filebuf;
        }

        bool init( byte *infilebuf )
        {
            dosheader = (PIMAGE_DOS_HEADER)infilebuf;
            if(dosheader->e_magic == IMAGE_DOS_SIGNATURE)
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
}


namespace ctWin32Wizard
{
    //
    // PARTCALLBACK(prc) ����һ��lambda �������ó�Ա�����Ļص�����
    #define PARTCALLBACK(proc) [=](HWND hDlg,DWORD windowId)->int{return proc(hDlg,windowId);}
    struct addedSector
    {
        DWORD verifycode;               // 0

        char compressType[10];			// ѹ����ʽ
        char programName[20];			// ��װ���ĳ�����
        char autorun[150];              // �Զ����г�������·��

        int nfiles;						// ���ļ�����
                                        //
                                        // +sizeof(addedSector) ֮�����ÿ���ļ��� fileblock
    };
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
    ���ڰ�װ %s, ���Ժ�...
    ������ȡ�ļ�...
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
        ctPEFile::PEFile selfpe;

        //////////////////////////////////////////////////////////////////////////
        // setup����
        // ��ȡ����ӵ��ڴ�����λ��
        addedSector* getAddedSector()
        {
            char tmp[260];
            GetModuleFileNameA( NULL, tmp, 260 );
            if(selfpe.loadfile( tmp ))
            {
                //Խ�����һ������,����������ݵ�λ��
                PIMAGE_SECTION_HEADER lastsec = selfpe.secheader + selfpe.seccount - 1;
                return (addedSector*)(selfpe.filebuf + lastsec->PointerToRawData + lastsec->SizeOfRawData);
            }
            return nullptr;
        }

        //��ȡָ�����ļ���λ��
        fileblock* getFileBlock( int ifile )
        {
            if(ifile < addedsec->nfiles)
            {
                //���ҵ���һ��
                fileblock* fb = (fileblock*)((DWORD)addedsec + sizeof( addedSector ));
                //һֱ������ָ����λ��
                for(int i = 0; i < ifile; i++)
                    fb = (fileblock*)((DWORD)fb + fb->filesize + sizeof( fileblock ));
                return fb;
            }
            return nullptr;
        }

        //
        //�ͷ��ļ���ָ��λ��
        //
        //���������ļ���������->������crash
        //
        void extractFile( int ifile )
        {
            fileblock* fb = getFileBlock( ifile );
            if(fb)
            {
                char extractPath[260] = {0};
                if(strcmp( fb->relativepath, ".tmp" ) == 0)
                {
                    //��ʱ�ļ�,ֱ���ͷŵ���ʱĿ¼
                    GetTempPathA( 260, extractPath );
                    strcat( extractPath, fb->filename );
                }
                else
                {
                    if(fb->relativepath[0])
                    {
                        //if relativepath exist, create folders
                        char dir[260];
                        wsprintfA( dir, "%s\\%s", setuppath.c_str(), fb->relativepath );
                        SHCreateDirectoryExA( NULL, dir, NULL );
                        wsprintfA( extractPath, "%s\\%s\\%s", setuppath.c_str(), fb->relativepath, fb->filename );
                    }
                    else
                    {
                        wsprintfA( extractPath, "%s\\%s", setuppath.c_str(), fb->filename );
                    }
                }

                // ��������ʾ��ǰ��ѹ�ļ�������
                ctd.setText( "showpath", extractPath );
                //Sleep( 100 );
                //return;

                auto writetoexfile = [&]( void* filebuf, DWORD fsize ) {
                    FILE *f = nullptr;
                    fopen_s( &f, extractPath, "wb" );
                    if(f)
                    {
                        fwrite( filebuf, 1, fsize, f );
                        fclose( f );
                    }
                };

                byte* filebuf = (byte*)((DWORD)fb + sizeof( fileblock ));

                // ����ѹ��ʱ,�Ƚ�ѹ
                if(compress == "zip")
                {
                    //-unzip to a membuffer -
                    HZIP hz = OpenZip( filebuf, fb->filesize, 0 );
                    if(hz)
                    {
                        ZIPENTRY ze;
                        int i;
                        FindZipItem( hz, fb->filename, true, &i, &ze );
                        char *ibuf = new char[ze.unc_size];
                        UnzipItem( hz, i, ibuf, ze.unc_size );
                        writetoexfile( ibuf, ze.unc_size );         //д��
                        delete[] ibuf;
                        CloseZip( hz );
                    }
                }
                else
                {
                    writetoexfile( filebuf, fb->filesize );         //д��
                }
            }
        }

        //��temp path����ļ�·��
        std::string getTempFile( char* fileName )
        {
            char tmp[260];
            GetTempPathA( 260, tmp );
            strcat( tmp, fileName );
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
                    wsprintfA( tmpall, (PCSTR)tmp, addedsec->programName );
                fclose( f );
            }
            return std::string( tmpall );
        }
        #define GETSETUPSTRING(i) getString(i).c_str()


        //////////////////////////////////////////////////////////////////////////
        ctWizard()
        {
            addedsec = getAddedSector();

            //�жϰ�װ����Ч��
            //if( addedsec->verifycode != 0 ||  addedsec->nfiles < 4) {
            //  MessageBoxA( 0, "��װ������!", 0, MB_ICONERROR );
            //  return;
            //}

            if(addedsec)
            {
                filescount = addedsec->nfiles;
                compress = addedsec->compressType;

                char programpath[260];
                ctCommon::GetProgramPath( programpath );
                setuppath = programpath;
                setuppath += "\\";
                setuppath += addedsec->programName;

                //�ͷ���ʱ�ļ�[setup temp file]����ʱ�ļ��� 
                for(int i = 0; i < 4; i++)
                    extractFile( i );

                start();
            } 
        }

        //
        // UI 
        //
        void start()
        {
            ctd.createMainDialog( 510, 370 );
            ctd.setTitle( GETSETUPSTRING( 1 ) );
            page1( ctd.hMainDlg, 0 );

            HICON hIcon = LoadIconA( ctd.appInstance, MAKEINTRESOURCEA( IDI_ICON1 ) );
            SendMessageA( ctd.hMainDlg, WM_SETICON, TRUE, (LPARAM)hIcon );

            //UI�߳���������
            ctd.showMainDialog();
        }
        int CALLBACK page1( HWND hDlg, DWORD windowId )
        {
            ctd.clearDlg();

            ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
            ctd.drawBmp( TMP_WizardImage, 0, 0, 160, 290 );
            ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,290} );
            ctd.createText( GETSETUPSTRING( 8 ), 180, 20, 300, 40, 16 );
            ctd.createText( GETSETUPSTRING( 9 ), 180, 65, 300, 25 );
            ctd.createText( GETSETUPSTRING( 10 ), 180, 90, 300, 25 );
            ctd.createText( GETSETUPSTRING( 11 ), 180, 115, 300, 25 );

            ctd.createbutton( GETSETUPSTRING( 3 ), 300, 300, PARTCALLBACK( page2 ) );
            ctd.createbutton( GETSETUPSTRING( 4 ), 400, 300, PARTCALLBACK( cancel ) );
            return 0;
        }
        int CALLBACK page2( HWND hDlg, DWORD windowId )
        {
            ctd.clearDlg();

            ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,60} );
            ctd.drawLine( 0, 60, 510, 60, (COLORREF)0xA0A0A0 );
            ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
            ctd.drawBmp( TMP_WizardSmallImage, 435, 0, 55, 55 );
            ctd.createText( GETSETUPSTRING( 12 ), 20, 15, 200, 15, 12 );
            ctd.createText( GETSETUPSTRING( 13 ), 40, 35, 300, 15 );
            ctd.drawBmp( TMP_folders, 40, 75, 36, 36 );
            ctd.createText( GETSETUPSTRING( 14 ), 90, 85, 350, 15 );
            ctd.createText( GETSETUPSTRING( 15 ), 40, 120, 400, 15 );
            ctd.createEdit( 40, 145, 300, 20, "path", setuppath.c_str() );
            ctd.createbutton( GETSETUPSTRING( 16 ), 350, 144, PARTCALLBACK( choosefile ) );
            ctd.createbutton( GETSETUPSTRING( 2 ), 200, 300, PARTCALLBACK( page1 ) );
            ctd.createbutton( GETSETUPSTRING( 3 ), 300, 300, PARTCALLBACK( page3 ) );
            ctd.createbutton( GETSETUPSTRING( 4 ), 400, 300, PARTCALLBACK( cancel ) );
            return 0;
        }
        int CALLBACK page3( HWND hDlg, DWORD windowId )
        {
            setuppath = ctd.getEditText( "path" );		//�ȶ�ȡ֮ǰpage2�е�path
            ctd.clearDlg();

            char tmp[200];
            wsprintfA( tmp, "%s\r\n\t%s", GETSETUPSTRING( 30 ), setuppath.c_str() );

            ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,60} );
            ctd.drawLine( 0, 60, 510, 60, (COLORREF)0xA0A0A0 );
            ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
            ctd.drawBmp( TMP_WizardSmallImage, 435, 0, 55, 55 );
            ctd.createText( GETSETUPSTRING( 17 ), 20, 15, 200, 15, 12 );
            ctd.createText( GETSETUPSTRING( 18 ), 40, 35, 200, 15 );
            ctd.createText( GETSETUPSTRING( 19 ), 40, 75, 400, 15 );
            ctd.createEdit( 40, 100, 400, 170, "lastshow" );
            PostMessageW( ctd.getWnd( "lastshow" ), EM_SETREADONLY, 1, 0 );		//����Ϊֻ��
            ctd.setText( "lastshow", tmp );
            ctd.createbutton( GETSETUPSTRING( 2 ), 200, 300, PARTCALLBACK( page2 ) );
            ctd.createbutton( GETSETUPSTRING( 6 ), 300, 300, PARTCALLBACK( page4 ) );
            ctd.createbutton( GETSETUPSTRING( 4 ), 400, 300, PARTCALLBACK( cancel ) );
            return 0;
        }
        int CALLBACK page4( HWND hDlg, DWORD windowId )
        {
            ctd.clearDlg();

            ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,60} );
            ctd.drawLine( 0, 60, 510, 60, (COLORREF)0xA0A0A0 );
            ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
            ctd.drawBmp( TMP_WizardSmallImage, 435, 0, 55, 55 );
            ctd.createText( GETSETUPSTRING( 20 ), 20, 15, 200, 15, 12 );
            ctd.createText( GETSETUPSTRING( 21 ), 40, 35, 300, 15 );
            ctd.createText( GETSETUPSTRING( 22 ), 40, 75, 400, 15 );
            ctd.createbutton( GETSETUPSTRING( 4 ), 400, 300, PARTCALLBACK( cancel ) );
            //����һ��text �Թ���ʾ��װ���ļ�����Ϣ
            ctd.createText( "", 40, 95, 450, 15, 0, "showpath" );
            //����һ�����ذ�ť,����ͨ������page5
            ctd.createbutton( GETSETUPSTRING( 5 ), 300, 300, PARTCALLBACK( page5 ), 85, 22, "overbutton" );
            ShowWindow( ctd.getWnd( "overbutton" ), 0 );

            //����������ý���Ϊ�ļ�����,ÿ���ͷ�һ���ļ� +1����
            ctd.createProgress( 40, 120, "extract", filescount );
            std::thread t1( &ctWizard::installing, this );
            t1.detach();
            return 0;
        }
        int CALLBACK page5( HWND hDlg, DWORD windowId )
        {
            ctd.clearDlg();

            ctd.drawLine( 0, 290, 510, 290, (COLORREF)0xA0A0A0 );
            ctd.drawBmp( TMP_WizardImage, 0, 0, 160, 290 );
            ctd.setForecolor( RGB( 255, 255, 255 ), {0,0,ctd.hMainDlgRect.right,290} );
            ctd.createText( GETSETUPSTRING( 23 ), 180, 20, 300, 22, 16 );
            ctd.createText( GETSETUPSTRING( 24 ), 180, 50, 300, 30 );
            ctd.createText( GETSETUPSTRING( 25 ), 180, 85, 300, 30 );
            ctd.createCheckbox( GETSETUPSTRING( 26 ), 180, 111 );

            ctd.createbutton( GETSETUPSTRING( 7 ), 300, 300, PARTCALLBACK( end ) );
            return 0;
        }

        //
        // UI function
        //
        int CALLBACK end( HWND hDlg, DWORD windowId )
        {
            //��װ��Ϻ� �����ݷ�ʽ/�Զ����еĳ���
            if(addedsec->autorun[0])
            {
                //create desktop shortcut
                ctCommon::CreateDesktopShotCut( addedsec->programName,addedsec->autorun, setuppath.c_str() );

                //run
                char runpath[260];
                wsprintfA( runpath, "%s\\%s", setuppath.c_str(), addedsec->autorun );
                ctCommon::Runexe( runpath, (char*)(setuppath.c_str()) );
            }

            PostQuitMessage( 0 );
            return 0;
        }
        void installing()
        {
            CreateDirectoryA(setuppath.c_str(),NULL);

            // from 4 to max  [0-3 is tmpfile]
            for(int i = 4; i < filescount; i++)
            {
                extractFile( i );
                ctd.setProgressPos( "extract", i );
            }

            // goto end page  
            SendMessageA( ctd.getWnd( "overbutton" ), BM_CLICK, 0, 0 );
        }
        int CALLBACK choosefile( HWND hDlg, DWORD windowId )
        {
            ctd.setText( "path", ctd.chooseFolders() );
            return 0;
        }
        int CALLBACK cancel( HWND hDlg, DWORD windowId )
        {
            std::string msg = GETSETUPSTRING( 28 );
            msg += "\n";
            msg += GETSETUPSTRING( 29 );

            if(MessageBoxA( hDlg, msg.c_str(), GETSETUPSTRING( 27 ), MB_ICONQUESTION | MB_YESNO ) == IDYES)
            {
                PostQuitMessage( 0 );
            }
            return 0;
        }
        //////////////////////////////////////////////////////////////////////////


    private:
        int filescount;
        std::string setuppath;
        std::string compress;
        ctWin32Dialog::ctDialog ctd;
        addedSector* addedsec;
    };
}