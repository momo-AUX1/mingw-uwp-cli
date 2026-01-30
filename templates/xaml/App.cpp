#include <windows.h>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/base.h>

#include "MainPage.h"

namespace winrt {
    using namespace Windows::UI::Xaml;
    using namespace Windows::ApplicationModel::Activation;
    using namespace Windows::UI::Xaml::Controls;
} // namespace winrt

namespace winrt::__PROJECT_NAMESPACE__::implementation {
    struct App : ApplicationT<App> {
        void OnLaunched(LaunchActivatedEventArgs const &args) {
            auto window = Window::Current();
            auto page = winrt::make<MainPage>();
            window.Content(page);
            window.Activate();
        }
    };
} // namespace winrt::__PROJECT_NAMESPACE__::implementation

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    winrt::init_apartment();
    winrt::Application::Start(
        [](auto &&) { winrt::make<winrt::__PROJECT_NAMESPACE__::implementation::App>(); });
    return 0;
}
