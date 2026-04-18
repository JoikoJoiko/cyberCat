#pragma once

#if defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L
#define APPCFG_STORAGE inline
#else
#define APPCFG_STORAGE static
#endif

namespace AppConfig
{
    APPCFG_STORAGE constexpr wchar_t kAppTitle[] = L"cyberCat";
    APPCFG_STORAGE constexpr wchar_t kMenuLabel[] = L"Menu";
    APPCFG_STORAGE constexpr wchar_t kWelcomeLine1[] = L"Welcome back,";
    APPCFG_STORAGE constexpr wchar_t kWelcomeLine2[] = L"Ms. Zarina";
    APPCFG_STORAGE constexpr wchar_t kVersionLabel[] = L"Version 0.0.2";

    APPCFG_STORAGE constexpr int kTimezoneOffsetHours = 3;

    APPCFG_STORAGE constexpr wchar_t kMenuItems[][16] = {
        L"Settings",
        L"Profiles",
        L"History"
    };

    APPCFG_STORAGE constexpr int kMenuItemCount = sizeof(kMenuItems) / sizeof(kMenuItems[0]);
}

#undef APPCFG_STORAGE