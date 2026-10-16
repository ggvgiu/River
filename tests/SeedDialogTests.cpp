#include "app/SeedDialog.h"
#include "Resource.h"
#include <iostream>
#include <vector>

namespace {
 int mode=0, ticks=0; bool initialized=false, rejected=false;
 void CALLBACK DriveDialog(HWND,UINT,UINT_PTR timer,DWORD) {
  if(++ticks>100) {auto h=FindWindowW(L"#32770",L"River - Base random seed");if(h)EndDialog(h,IDCANCEL);KillTimer(nullptr,timer);return;}
  auto warning=FindWindowW(L"#32770",L"Invalid seed");
  if(warning) {rejected=true;PostMessageW(warning,WM_COMMAND,IDOK,0);return;}
  auto dialog=FindWindowW(L"#32770",L"River - Base random seed");
  if(!dialog)return;
  if(rejected) {PostMessageW(dialog,WM_COMMAND,IDCANCEL,0);return;}
  if(initialized)return;
  wchar_t value[32]{};GetDlgItemTextW(dialog,IDC_SEED_VALUE,value,32);
  if(!value[0])return;
  initialized=true;
  SetDlgItemTextW(dialog,IDC_SEED_VALUE,mode==3?L"65536":mode==2?L"0":L"0x1234");
  PostMessageW(dialog,WM_COMMAND,mode==1?IDCANCEL:IDOK,0);
 }
}
int main() {
 auto instance=GetModuleHandleW(nullptr);
 auto accelerators=LoadAcceleratorsW(instance,MAKEINTRESOURCEW(IDC_RIVER));
 const int count=CopyAcceleratorTableW(accelerators,nullptr,0);
 std::vector<ACCEL> bindings(count);
 CopyAcceleratorTableW(accelerators,bindings.data(),count);
 int f5Bindings=0,f4Bindings=0;
 for(const auto& binding:bindings)if(binding.key==VK_F4) {
  if(binding.cmd!=IDM_SEED_RANDOM)return 5;
  ++f4Bindings;
 }
 if(f4Bindings!=1)return 6;
 for(const auto& binding:bindings)if(binding.key==VK_F5) {
  if(binding.cmd!=IDM_VIEW_3D)return 2;
  ++f5Bindings;
 }
 if(f5Bindings!=1)return 3;
 auto menu=LoadMenuW(instance,MAKEINTRESOURCEW(IDC_RIVER));
 if(!menu || GetMenuItemCount(GetSubMenu(menu,0))!=5)return 4;
 DestroyMenu(menu);
 for(mode=0;mode<4;++mode) {
  uint16_t seed=0xA814;ticks=0;initialized=rejected=false;
  auto timer=SetTimer(nullptr,0,50,DriveDialog);
  bool applied=RequestSeed(nullptr,GetModuleHandleW(nullptr),seed);
  KillTimer(nullptr,timer);
  if(!initialized || (mode==0 && (!applied || seed!=0x1234)) ||
     (mode==2 && (!applied || seed!=0 || rejected)) ||
     ((mode==1 || mode==3) && (applied || seed!=0xA814)) || (mode==3 && !rejected)) {
   std::cerr<<"FAIL: dialog case "<<mode<<" initialized "<<initialized<<" applied "<<applied<<" seed "<<seed<<'\n';return 1;
  }
 }
 std::cout<<"PASS: seed dialog initialization, Apply, Cancel, zero acceptance and invalid entry rejection\n";
}
