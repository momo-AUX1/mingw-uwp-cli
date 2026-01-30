# MinGW UWP Studio

This project generates MinGW-based UWP templates and provides a GUI/CLI for:
- Creating projects (console/xaml/corewindow/library).
- Registering and running loose builds.
- Packaging MSIX with `makemsix`.
- Signing MSIX with `osslsigncode`.
- Deploying to Windows Device Portal (including Xbox).

## Requirements
- Python 3
- `makemsix` (pack-capable) or `makeappx.exe`. Windows SDK is not required if you install standalone (though it does include whats needed its windows only):
  - `https://github.com/microsoft/msix-packaging`
- `osslsigncode` for signing:
  - `https://github.com/mtrojnar/osslsigncode`
- OpenSSL if you want to create a PFX from the GUI.

## Quick Start (GUI)
```bash
python scripts/mingw_uwp_cli.py
```
The GUI includes tabs for Project Creation, Runner/Installer, Packaging, and Signing.

## CLI Usage
### Create a project
```bash
python scripts/mingw_uwp_cli.py xaml MyApp --output-dir out --arch x64 --publisher "CN=MyPublisher"
```

### Register & run a loose build
```bash
python scripts/mingw_uwp_cli.py run out/MyApp/build
```

### Register without launching, then collect logs
```bash
python scripts/mingw_uwp_cli.py run out/MyApp/build --no-launch --logs --debug --crashes --minutes 15
```

### Install a local MSIX/AppX
```bash
python scripts/mingw_uwp_cli.py run out/MyApp/MyApp.msix
```

### Remote deploy (Device Portal)
Remote deploy **requires** a JSON config with credentials.

Loose folder deploy (UNC path):
```bash
python scripts/mingw_uwp_cli.py run \\\\HOST\\share\\MyApp --remote --json out/MyApp/mingw_winrt.json
```

MSIX/AppX deploy:
```bash
python scripts/mingw_uwp_cli.py run out/MyApp/MyApp.msix --remote --json out/MyApp/mingw_winrt.json
```

#### `mingw_winrt.json` portal section
```json
{
  "devicePortal": {
    "ip": "192.168.1.50",
    "username": "dev",
    "password": "pass"
  }
}
```

### CLI Options (summary)
#### `run` / `install` (loose + MSIX)
```
python scripts/mingw_uwp_cli.py run <build_dir | package.msix> [--remote] [--json path] [--no-launch]
                                     [--logs] [--debug] [--crashes] [--minutes N]
```
- `<build_dir>` must contain `AppxManifest.xml` for loose registration.
- `<package.msix>` can be `.msix`, `.msixbundle`, `.appx`, `.appxbundle`.
- `--remote` requires `--json` and deploys to Device Portal.
- `--no-launch` skips launching; combine with `--logs/--debug/--crashes` to inspect a running app.

#### `makemsix`
```
python scripts/mingw_uwp_cli.py makemsix <build_dir> [-o output.msix] [--makemsix PATH]
```
- Requires a `makemsix` build that supports `pack` or `makeappx.exe`.

### Package with makemsix
```bash
python scripts/mingw_uwp_cli.py makemsix out/MyApp/build -o out/MyApp/MyApp.msix --makemsix C:\\Path\\To\\makemsix.exe
```

### Edit project config
```bash
python scripts/mingw_uwp_cli.py out/MyApp/mingw_winrt.json
```

## Notes
- Runtime logs are pulled from AppModel-Runtime/Admin. Debug output relies on Windows debug logs; enable them if you see “No recent debug output found.”
- MSIX remote deploy will automatically include `.appx` dependencies found in a `Dependencies/` folder next to the package.
- Tool paths for `makemsix` and `osslsigncode` are cached in `scripts/db.json`.
