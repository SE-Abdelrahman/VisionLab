// OpenCV_Template.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "MyForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]
int main(array<String^>^ args)
{
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    VisionLab::MyForm form;
    Application::Run(% form);

    return 0;
}