**Updates and Fixes**

***Breaking change*** *You should deploy v1.0.9 if migrating from a version ealier version than v1.0.10, this will ensure your settings (mqtt server, spa name, etc) are migrated.*

* Resolves a bug where sometimes the user was notified of an available update when there wasn't one.
* Fixes setSpaTime from webui
* Generalise /set so all spa settings can be configured from the webui - prepartion for further updates
* Update to the latest release from home assistant (not available on ESP32)
* install a specific release from the webui (not available on ESP32)
* Small bug in configChangeCallbackString
