#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <thread>
#include <chrono>

using namespace winrt;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;

namespace winrt::__PROJECT_NAMESPACE__ {
    struct App : implements<App, IFrameworkViewSource, IFrameworkView> {
        CoreWindow m_window{nullptr};

        IFrameworkView CreateView() { return *this; }

        void Initialize(CoreApplicationView const&) {}

        void SetWindow(CoreWindow const& window) {
            m_window = window;
            m_window.Activate();
        }

        void Load(hstring const&) {}

        void Run() {
            auto dispatcher = m_window.Dispatcher();
            while (true) {
                dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        void Uninitialize() {}
    };
} // namespace winrt::__PROJECT_NAMESPACE__

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    init_apartment();
    CoreApplication::Run(make<winrt::__PROJECT_NAMESPACE__::App>());
    return 0;
}
