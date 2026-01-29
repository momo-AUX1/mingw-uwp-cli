#define WIN32_MEAN_AND_LEAN
#include <windows.h>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>

#include "MainPage.h"

namespace winrt {
    using namespace Windows::Foundation;
    using namespace Windows::UI::Xaml;
}

namespace winrt::__PROJECT_NAMESPACE__::implementation {
    void MainPage::InitializeComponent() {
        Uri uri(L"ms-appx:///MainPage.xaml");
        Application::LoadComponent(*this, uri);
    }
}
