# Changelog
-
## [2.0.1-beta]
- Feature: Make `PSAV_LVL` a read-write property and implement writer (`W63:<0-2>`) to set Off/Low/High.
- Feature: Add read-write properties `PSAV_BGN` and `PSAV_END` and implement writers (`W64:<value>`, `W65:<value>`) for power-save start/end times (encoded as h*256+m).
- Fix: Simplfy Debug logging output to reduce noise.

## [2.0.0-beta]
- Rewrite SpaInterface class to be more efficient
- Add settable CLMT and sanitize cycle start time handling
- Add `ss` debug command for sending raw serial payloads to the spa controller and extended timeouts for long-running commands
- Add heat element current measurement and support W01-W07 commands in register map
- Add keyboard button press support for waking the spa
- Add CLMT & VMAX as settable properties

## [1.0.18]
- Add v2 boards
- Fix string conversion for sleep timers
   
## [1.0.17]
- Fix typo in Lock Mode autodiscovery

## [1.0.16]
- Add control panel lock mode
- Optomise code to reduce size

## [1.0.15]
- Fixed setMode 

## [1.0.14] 

### Changed
- Changed spa-base to ESP32-S3
- Disabled LED 4 on ESP32-S3 due to conflict
- Increased memory allocation
- Fix Python version rather than have it auto update to new versions
- Force 1.0.14 release

## [1.0.13] 

### Fixed
- Fixed access point address space to 169.254.1.1/24

## [1.0.12] 

### Changed
- Changed AP network to link-local address space (168.254.1.0/24)
