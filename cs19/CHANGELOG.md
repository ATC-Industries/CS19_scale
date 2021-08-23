# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.1.1] - 2021-05-04

### Added

- Added a CHANGELOG.md file
- HTTPClient.h locally

### Removed 

- Removed millis timer for index bashing prevention method that did not work.

### Fixed

- Fixed include file issues

## [1.1.0] - 2021-03-26

### Added

- Added `/v1/getJson` API call that returns scale information in one call and returns a JSON object.
- Add protection against mashing refresh button in browser that caused ESP to reset.  (Did not work)

### Changed

- Changed to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
- Removed all Datafiles from SPIFFS and now storing required webfiles in program memory
- Cleaned a lot of code removed old dead sections


## [1015] - No Date [YANKED]

### Changed

 - increase watchdog timer (this was returned back in next version)  No Units with 1015 were ever shipped.

## [1014] - No Date

### Changed

 - Add more last locked in table on iPhone

## [1013] - No Date

### Changed

 - Fix Units button startup problems

## [1012] - 2021-09-07

### Changed

 - Add startup check for stylesheet and flash purple led

## [1011] - No Date

### Changed

 - Outputs 0.000 to remote display instead of 0.00 when in Kg mode.

## [1010] - 2020-05-08

### Changed

 - fix weight off by one error / increased max connections from 4 to 10

## [1009] - No Date

### Changed

 - Added units button startup timeout.

## [1008] - No Date

### Changed

 - Change RX TX to Xbee to 19/21

## [1007] - 2020-02-05

### Changed

 - Add auto print or manual print option, set on printer

## [1006] - No Date

### Changed

 - SHIP - Bobo Albright 

## [1005] - No Date

### Changed

 - Add legacy remote checker, blink LED red for no Legacy, RGB for Xbee found

## [1004] - No Date

### Changed

 - Add all styles locally via data upload.

## [1001] - No Date

### Changed

 - Initial Design


[unreleased]: https://github.com/atclarkson/cs19_wifi/compare/v1.1.0...HEAD
[1.1.0]: https://github.com/atclarkson/cs19_wifi/releases/tag/v1.1.0
[1012]: https://github.com/atclarkson/cs19_wifi/releases/tag/v1012
[1010]: https://github.com/atclarkson/cs19_wifi/releases/tag/v1.0.10
[1007]: https://github.com/atclarkson/cs19_wifi/releases/tag/v1.0.7