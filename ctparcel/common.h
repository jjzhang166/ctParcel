#pragma once

#include <windows.h>
#include <string>

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
    bool CreateDesktopShotCut( std::string shotcutName,std::string exeName, std::string strSourcePath )
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
            psl->SetPath( (strSourcePath +"\\"+ exeName).c_str() );

            //�����ݷ�ʽ������ 
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
