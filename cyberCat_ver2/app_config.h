#pragma once

namespace AppConfig
{
    static constexpr wchar_t kAppTitle[] = L"cyberCat";
    static constexpr wchar_t kMenuLabel[] = L"Menu";
    static constexpr wchar_t kWelcomeLine1[] = L"Welcome back,";
    static constexpr wchar_t kWelcomeLine2[] = L"Ms. Zarina";
    static constexpr wchar_t kVersionLabel[] = L"Version 0.0.2";

    static constexpr int kTimezoneOffsetHours = 3;

    static constexpr wchar_t kMenuItems[][16] = {
        L"Settings",
        L"Profiles",
        L"History"
    };

    static constexpr int kMenuItemCount = sizeof(kMenuItems) / sizeof(kMenuItems[0]);
}
