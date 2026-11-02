# Changelog
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
