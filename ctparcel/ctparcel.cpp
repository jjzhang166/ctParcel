// ctparcel.cpp : 定义控制台应用程序的入口点。
//

//链接为Windows新的视觉
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "ctWizard.h"

//////////////////////////////////////////////////////////////////////////

int WINAPI WinMain( _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd )
{
    ctWin32Wizard::ctWizard ctw;
    return 0;
}

