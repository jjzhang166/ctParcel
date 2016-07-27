#pragma once

#include <windows.h>
#include <string>

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
    bool CreateDesktopShotCut( std::string shotcutName,std::string exeName, std::string strSourcePath )
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
            psl->SetPath( (strSourcePath +"\\"+ exeName).c_str() );

            //保存快捷方式到桌面 
            IPersistFile* ppf;
            psl->QueryInterface( IID_IPersistFile, (void**)&ppf );
            WCHAR wsz[MAX_PATH];
            MultiByteToWideChar(CP_THREAD_ACP,MB_PRECOMPOSED, (LPCTSTR)strDestDir.c_str(),-1,wsz,MAX_PATH);
            ppf->Save( wsz, TRUE ); 
            return true;
        }

        CoUninitialize();
        return false;;
    }
}
