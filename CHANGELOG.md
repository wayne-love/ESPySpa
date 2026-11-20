# Changelog
## [2.0.5]
### Breaking Changes
eSpa v2 devices shipped in or before October 2026, have an error in the partition table. You have two options:
1. OTA update - requires a careful sequence of steps (outlined below)
2. Remove the eSpa and flash 2.0.5 using flash.espa.diy

This firmware will flash without issues on an eSpa v1 or ESP32 with 8MB of flash storage.

### Features:
- Implement LittleFS and include internet dependencies on flash memory
- Enable gzip compression

### eSpa v2 OTA Update Process
The below steps must followed very carefully. If anything goes wrong during the Over The Air (OTA) update process, you always have the option of removing the eSpa board from the spa and flashing it manually.

Prior to starting download the following files:
1. firmware_migrator.bin from my repo: https://github.com/lucasnz/espa-partition-migration/releases

1. Ensure version firmware version 2.0.4 is installed. The currently installed version is displayed on the eSpa web interface at: http://espa.local/.
2. From the eSpa web interface, flash firmware_migrator.bin using the application update file dialog (leave the file system blank).
3. The eSpa will automatically reboot into a new eSpa Partition Migrator web interface. You should see "Settings backup ready" is yes. In the diaglog "1. Stage bridge firmware", select firmware_bridge.bin, and click "Upload bridge image".
4. After the firmware upload completes, you should see "Bridge image staged" is yes. Then click "Commit migration", and then ok.
5. Now you should see "Settings restored" is yes. In the "4. Install filesystem (optional)" dialog, select littlefs_espa-v2.bin (from the 2.0.5 eSpa release page), and click "Upload filesystem".
6. Now you should see "Final filesystem installed" is yes. In the "5. Install final eSpa firmware" dialog, select firmware_espa-v2_ota.bin (from the 2.0.5 eSpa release page), and click "Upload final firmware", and then ok.

The eSpa should reboot and you should now be running firmware version 2.0.5.

## [2.0.4]
### Features:
- Implement spa name changes without reboot
- Removed internet dependencies from /fota
- Back off mqtt when trying to reconnect
- Add redirect to /fota if file system is corrupt
- Add 404 Not Found response for files that don't exist
- Add sanitise button support
- Creation of a new RemoteDebug library that includes the Web Debug pages natively 
### Dependency updates:
- Update PubSubClient to more modern and maintained libraries
### Bug fixes:
- Implement safe handling of millis()
- Improved handling of date/time in espa.js
- Json tidy ups in WebUI.cpp
- Tidy up logging in SpaInterface.cpp
- Fixes for github build process
