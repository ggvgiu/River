#include "SeedDialog.h"
#include "SeedValue.h"
#include "Resource.h"
#include <cstdio>

namespace
{
    INT_PTR CALLBACK SeedDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_INITDIALOG)
        {
            auto* seed = reinterpret_cast<uint16_t*>(lParam);
            SetWindowLongPtrW(dialog, DWLP_USER, lParam);
            wchar_t value[16];
            swprintf_s(value, L"0x%04X", unsigned(*seed));
            SetDlgItemTextW(dialog, IDC_SEED_VALUE, value);
            SendDlgItemMessageW(dialog, IDC_SEED_VALUE, EM_SETLIMITTEXT, 16, 0);
            SetFocus(GetDlgItem(dialog, IDC_SEED_VALUE));
            SendDlgItemMessageW(dialog, IDC_SEED_VALUE, EM_SETSEL, 0, -1);
            return FALSE;
        }
        if (message == WM_COMMAND)
        {
            if (LOWORD(wParam) == IDCANCEL) { EndDialog(dialog, IDCANCEL); return TRUE; }
            if (LOWORD(wParam) == IDOK)
            {
                wchar_t text[32]{};
                GetDlgItemTextW(dialog, IDC_SEED_VALUE, text, 32);
                const auto value = RiverRaid::ParseSeed(text);
                if (!value)
                {
                    MessageBoxW(dialog, L"Enter a seed from 1 to 65535.\n"
                        L"Use decimal (43028), hex (0xA814), or Atari hex ($A814).\n"
                        L"Zero is not allowed because it stops the random generator.",
                        L"Invalid seed", MB_OK | MB_ICONWARNING);
                    SetFocus(GetDlgItem(dialog, IDC_SEED_VALUE));
                    SendDlgItemMessageW(dialog, IDC_SEED_VALUE, EM_SETSEL, 0, -1);
                    return TRUE;
                }
                auto* seed = reinterpret_cast<uint16_t*>(GetWindowLongPtrW(dialog, DWLP_USER));
                *seed = *value;
                EndDialog(dialog, IDOK);
                return TRUE;
            }
        }
        if (message == WM_CLOSE) { EndDialog(dialog, IDCANCEL); return TRUE; }
        return FALSE;
    }
}

bool RequestSeed(HWND owner, HINSTANCE instance, uint16_t& seed)
{
    return DialogBoxParamW(instance, MAKEINTRESOURCEW(IDD_SEED), owner,
        SeedDialogProc, reinterpret_cast<LPARAM>(&seed)) == IDOK;
}
