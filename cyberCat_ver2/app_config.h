#pragma once

namespace AppConfig
{
    inline constexpr wchar_t kAppTitle[] = L"cyberCat";
    inline constexpr wchar_t kMenuLabel[] = L"Menu";
    inline constexpr wchar_t kWelcomeLine1[] = L"Welcome back,";
    inline constexpr wchar_t kWelcomeLine2[] = L"Ms. Zarina";
    inline constexpr wchar_t kVersionLabel[] = L"Version 0.0.2";

    inline constexpr int kTimezoneOffsetHours = 3;

    inline constexpr wchar_t kMenuItems[][16] = {
        L"Settings",
        L"Profiles",
        L"History"
    };

    inline constexpr int kMenuItemCount = sizeof(kMenuItems) / sizeof(kMenuItems[0]);
}
