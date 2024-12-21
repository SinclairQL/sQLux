# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## 1.0.10 - 2024-12-21
### Added
- workflows now generate a PDF manual
- workflows add an aarch64 macos build

## 1.0.9 - 2024-12-21
### Changed
- Rewrote CI (Github actions)
  See [README.md](README.md#releases-anchor)
  Creates automatically Github Releases for:
  - Linux (x86-64, armv7, aarch) on Ubuntu and Debian
  - MacOS (x86-64 Intel)
  - Windows (MSYS2 based on x86-64 and i686)
- Removed `.vscode`

### Added
- CHANGELOG.md
- Renamed/added some `cmake` arch. specific `Toolchain*` definitions
- .gitkeep to keep `build/` folder
- QL SW content included in artifacts:
  - `mdv*` folders (moved to `examples/`)
  - New `examples/sqlux.ini`
  - New `examples/mdv3` : SQLMISE_TestChart2.zip from Dilwyn SW repo
- Added `cmake` spec files `Toolchain-*.cmake` for `arm64` and `armhf` (used in CI scripts)

### Fixed
- Build was failing on `arm` due to missing CC flag `-lm` on `SDL2`

## [1.0.6] - 2022-11-22

## [1.0.5] - 2022-05-02

[Unreleased]: https://github.com/SinclairQL/sQLux/
[1.0.6]: https://github.com/SinclairQL/sQLux/compare/v1.0.5...v1.0.6
[1.0.5]: https://github.com/SinclairQL/sQLux/releases/tag/v1.0.5
