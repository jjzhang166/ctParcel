#pragma once

#include <shlobj.h>  
#include "ctWin32Dialog.h"
#include "unzip.h"
#include "resource.h"

namespace ctCommon
{
    //运行文件
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
    //得到当前program路径
    BOOL GetProgramPath( char *pszPath )
    {
        return SHGetSpecialFolderPathA( NULL, pszPath, CSIDL_PROGRAM_FILES, FALSE );
    }
    //得到当前桌面路径
    BOOL GetDesktopPath( char *pszPath )
    {
        return SHGetSpecialFolderPathA( NULL, pszPath, CSIDL_DESKTOP, FALSE );
    }
    //创建桌面快捷方式
    bool CreateDesktopShotCut( std::string shotcutName, std::string exeName, std::string strSourcePath )
    {
        if(FAILED( CoInitialize( NULL ) ))
            return FALSE;

        char Path[MAX_PATH];
        std::string strDestDir;

        //设置桌面快捷方式的名字
        GetDesktopPath( Path );
        strDestDir = Path;
        strDestDir += "\\";
        strDestDir += shotcutName;
        strDestDir += ".lnk";

        IShellLink* psl;
        if(SUCCEEDED( CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl ) ))
        {
            //设置快捷方式的起始位置 
            //比如目标位置为C:\windows\a.txt 起始位置就应该设置为C:\windows否则会导致不可预料的错误
            //如果是文件夹的快捷方式起始位置和目标位置可以设置为一样
            psl->SetWorkingDirectory( strSourcePath.c_str() );
            //设置快捷方式的目标位置 
            psl->SetPath( (strSourcePath + "\\" + exeName).c_str() );

            //保存快捷方式到桌面 
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
    // PARTCALLBACK(prc) 生成一个lambda 用来调用成员函数的回调函数
    #define PARTCALLBACK(proc) [=](HWND hDlg,DWORD windowId)->int{return proc(hDlg,windowId);}
    struct addedSector
    {
        DWORD verifycode;               // 0

        char compressType[10];			// 压缩方式
        char programName[20];			// 安装包的程序名
        char autorun[150];              // 自动运行程序的相对路径

        int nfiles;						// 总文件数量
                                        //
                                        // +sizeof(addedSector) 之后就是每个文件块 fileblock
    };
    // 在生成的最终exe的文件后面添加一个结构,这个结构包含了必须的UI图片/配置/压缩文件等
    #define RELATIVEPATH_TMP	".tmp"
    struct fileblock
    {
        char relativepath[150];			// 临时文件=".tmp" or 相对路径 : aa/bb/
        char filename[50];				// 文件名   : a.dll   >> 那么这个文件的路径: setuppath/aa/bb/a.dll
        DWORD filesize;					// 大小
                                        //
                                        // +sizeof(fileblock) 之后就是文件内容 byte* filebuf
    };

    /*
    -----------(规定)-----------

    默认使用第1个文件(txt-file)作为安装时的字符串读取:
    **************************************
    [follow this:]  gb2312

    安装 - %s
    < 上一步(B)
    下一步(N) >
    取消
    继续
    安装(I)
    完成(F)
    欢迎使用 %s 安装向导
    现在将安装 %s。
    建议你在继续之前关闭其他应用程序。
    点击"下一步"继续,或者点击"取消"退出安装。
    选择目标位置
    %s 要安装到哪里？
    安装程序将把 %s 安装到以下文件夹。
    若要继续,请点击"下一步"。 如果你要换一个文件夹,请点击"浏览"。
    浏览(R)...
    准备安装
    安装程序准备在你的电脑上安装。
    点击"安装"继续,如果你想修改设置请点击"上一步"。
    正在安装
    正在安装 %s, 请稍后...
    正在提取文件...
    %s 安装向导完成
    安装程序已在您的电脑中安装了 %s 。 此应用程序可以通过选择安装的快捷方式运行。
    单击'完成'退出安装程序。
    运行 %s
    退出安装
    安装未完成!如果你现在退出,程序将无法完成！
    要退出安装吗？

    **************************************

    默认使用第2,3,4个文件作为3个安装时的图片:
    2: WizardImage.bmp
    3: SmallWizardImage.bmp
    4: folders.bmp
    */

    //
    // Wizard类
    //
    class ctWizard
    {
    public:
        ctPEFile::PEFile selfpe;

        //////////////////////////////////////////////////////////////////////////
        // setup功能
        // 获取被添加的内存所在位置
        addedSector* getAddedSector()
        {
            char tmp[260];
            GetModuleFileNameA( NULL, tmp, 260 );
            if(selfpe.loadfile( tmp ))
            {
                //越过最后一个区段,就是添加数据的位置
                PIMAGE_SECTION_HEADER lastsec = selfpe.secheader + selfpe.seccount - 1;
                return (addedSector*)(selfpe.filebuf + lastsec->PointerToRawData + lastsec->SizeOfRawData);
            }
            return nullptr;
        }

        //获取指定的文件块位置
        fileblock* getFileBlock( int ifile )
        {
            if(ifile < addedsec->nfiles)
            {
                //先找到第一个
                fileblock* fb = (fileblock*)((DWORD)addedsec + sizeof( addedSector ));
                //一直遍历到指定的位置
                for(int i = 0; i < ifile; i++)
                    fb = (fileblock*)((DWORD)fb + fb->filesize + sizeof( fileblock ));
                return fb;
            }
            return nullptr;
        }

        //
        //释放文件到指定位置
        //
        //如果传入的文件数量错误->函数会crash
        //
        void extractFile( int ifile )
        {
            fileblock* fb = getFileBlock( ifile );
            if(fb)
            {
                char extractPath[260] = {0};
                if(strcmp( fb->relativepath, ".tmp" ) == 0)
                {
                    //临时文件,直接释放到临时目录
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

                // 界面上显示当前解压文件的名称
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

                // 存在压缩时,先解压
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
                        writetoexfile( ibuf, ze.unc_size );         //写入
                        delete[] ibuf;
                        CloseZip( hz );
                    }
                }
                else
                {
                    writetoexfile( filebuf, fb->filesize );         //写入
                }
            }
        }

        //在temp path里的文件路径
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

        //到临时文件夹里去获取str.txt指定的文字
        //和programName进行格式化
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

            //判断安装包有效性
            //if( addedsec->verifycode != 0 ||  addedsec->nfiles < 4) {
            //  MessageBoxA( 0, "安装包错误!", 0, MB_ICONERROR );
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

                //释放临时文件[setup temp file]到临时文件夹 
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

            //UI线程这里阻塞
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
            setuppath = ctd.getEditText( "path" );		//先读取之前page2中的path
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
            PostMessageW( ctd.getWnd( "lastshow" ), EM_SETREADONLY, 1, 0 );		//设置为只读
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
            //创建一个text 以供显示安装的文件名信息
            ctd.createText( "", 40, 95, 450, 15, 0, "showpath" );
            //创建一个隐藏按钮,可以通过他到page5
            ctd.createbutton( GETSETUPSTRING( 5 ), 300, 300, PARTCALLBACK( page5 ), 85, 22, "overbutton" );
            ShowWindow( ctd.getWnd( "overbutton" ), 0 );

            //这里可以设置进度为文件数量,每次释放一个文件 +1进度
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
            //安装完毕后 桌面快捷方式/自动运行的程序
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