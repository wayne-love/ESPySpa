**v1.0.12 Alpha**

* Fixed check for update in Web UI.
* Firmware updates via HA for eSpa board only
* eSpa can set spa controller day of week
* Added support for V2 controllers
* When spa controller read fails, try again (once), then back off

Code tweaks
* Move code out of WebUI class
* Remark out unused variables to minimise complied size
* Add validation for spa set temperature
* Optomize readStatus function by copying and modifying port.readUntil()
