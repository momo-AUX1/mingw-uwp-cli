#pragma once
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/base.h>

namespace winrt {
    using namespace winrt::Windows::UI::Xaml::Controls;
}

namespace winrt::__PROJECT_NAMESPACE__::implementation {
    struct MainPage : winrt::Windows::UI::Xaml::Controls::UserControlT<MainPage> {
        MainPage() { InitializeComponent(); }
        void InitializeComponent();
    };
}
