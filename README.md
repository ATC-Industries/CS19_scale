# CS19 Scale Firmware

Firmware and supporting files for the Pro Tournament Scales CS19 scale Wi-Fi interface.

The current CS19 system allows a phone, tablet, or computer to connect directly to the scale over Wi-Fi, view scale information in a browser, and retrieve scale data programmatically through a local JSON endpoint.

> **Important:** The ideas listed under **Future Development Ideas** are exploratory only. They are not committed features, release dates, or development plans.

## Current Functionality

The CS19 currently:

* Creates its own Wi-Fi network for local connections.
* Provides a browser-based interface for viewing scale data.
* Provides a JSON endpoint for software integrations.
* Communicates with the scale hardware through its existing serial interface.
* Allows phones, tablets, and computers to access scale data without requiring an internet connection.

The scale is currently available at:

`192.168.1.184`

## Local API

### Scale Data

Current scale data can be retrieved from:

`GET http://192.168.1.184/v1/scale/data`

The response includes information such as:

* Current weight
* Units
* Scale lock/status
* Lock counter
* Recent locked weights

Example:

```json
{
  "weight": "    0.00",
  "units": "lbs",
  "locked": "READY",
  "lockedodo": 151,
  "lastLocked": ["", "---", "---", "---", "---"]
}
```

Some values are currently returned in the same formatted form used by the CS19 display. Third-party applications should not assume every field is a normalized numeric value.

### Polling

The web server runs directly on the CS19 controller.

Applications should avoid excessive polling. The current recommended maximum request rate is approximately once every 200 ms.

Additional notes about the existing API can be found in:

`cs19/api_doc.md`

## Development Environment

The firmware is built using PlatformIO and the Arduino framework for ESP32.

Primary project configuration:

`cs19/platformio.ini`

Primary application code:

`cs19/src/main.cpp`

Scale communication and scale-state handling are primarily contained in the `Scale` class.

## Repository Layout

```text
CS19_scale/
├── README.md
└── cs19/
    ├── api_doc.md
    ├── CHANGELOG.md
    ├── platformio.ini
    ├── data/
    ├── include/
    ├── lib/
    └── src/
```

## Integration Notes

The existing API is designed for local communication with the CS19.

A third-party application can connect to the CS19 Wi-Fi network and request scale data directly without parsing the browser interface.

The current firmware does not:

* Provide a cloud-hosted API.
* Allow the CS19 to join an arbitrary customer Wi-Fi network.
* Provide a supported Bluetooth integration interface.

Bluetooth-capable hardware is present in the unit, so Bluetooth communication may be technically possible in the future, but there are currently no committed development plans for it.

# Future Development Ideas

The following are possible directions for future development.

These are being documented as ideas and areas worth investigating, not as a committed roadmap.

## Improved Local API

The existing API works, but a future revision could provide a cleaner and more formal interface for third-party developers.

Possible improvements include:

* A consistent `/api/v1/...` namespace.
* Numeric weight values instead of display-formatted strings.
* Clearly defined unit values.
* Explicit scale status fields.
* Unique scale identification.
* Firmware version information.
* API version information.
* Capability reporting.
* More consistent error responses.
* Better integration documentation and examples.

A future response could look more like:

```json
{
  "device": {
    "model": "CS19",
    "id": "CS19-XXXXXX",
    "firmware": "1.x.x"
  },
  "measurement": {
    "weight": 12.42,
    "unit": "lb",
    "stable": true,
    "locked": true
  },
  "sequence": 152
}
```

The exact API structure has not been decided.

## Live Scale Data

The current API is request-based, meaning applications poll the scale for updates.

A future version could potentially provide live scale updates using WebSockets, Server-Sent Events, or a similar mechanism.

A possible long-term design would be:

* REST/JSON for current scale state and device information.
* A live event connection for weight and lock-state changes.

This could reduce unnecessary polling and make mobile and desktop integrations easier.

## Customer Wi-Fi Support

The CS19 currently operates as its own Wi-Fi network.

A future version could potentially allow the scale to also connect to an existing network, such as:

* A home Wi-Fi network.
* A tournament venue network.
* A dedicated router.
* Another local network.

Possible goals would include:

* Preserve the existing direct CS19 Wi-Fi mode.
* Optionally connect to a customer-selected Wi-Fi network.
* Save network settings on the scale.
* Provide a simple configuration process.
* Provide a reliable recovery/fallback method if the configured network is unavailable.
* Allow applications to find the scale without requiring the user to know its IP address.

The configuration and recovery process would need to be carefully designed before this could become a production feature.

## Device Discovery

If customer Wi-Fi support is added, applications should not require users to manually determine the scale's IP address.

Future firmware could potentially support local network discovery.

For example, an application could display:

```text
Available Scales

CS19-A84F
CS19-29D3
CS19-71B2
```

The user could then select the appropriate scale.

## Unique Device Identification

A future API should expose a stable unique identifier for each physical CS19.

This would help with:

* Multiple scales on the same network.
* Third-party app integrations.
* Remembering previously connected scales.
* Device-specific settings.
* Troubleshooting and support.

## Bluetooth / BLE

The CS19 hardware includes Bluetooth capability.

A future firmware version could investigate Bluetooth or Bluetooth Low Energy as another method of communicating with mobile devices.

Potential uses could include:

* Direct iOS and Android app connections.
* Scale discovery.
* Weight updates.
* Configuration.

There are currently no committed development plans for Bluetooth support.

## Backward Compatibility

Future API or networking changes should avoid unnecessarily breaking existing CS19 installations.

Where practical, future development should preserve:

* The existing browser interface.
* Direct CS19 Wi-Fi operation.
* Existing `/v1/scale/data` integrations.

New functionality should preferably be added alongside the existing interface rather than replacing it immediately.

# Possible Development Order

If future development is pursued, a reasonable technical progression may be:

1. Define a cleaner API structure.
2. Add unique device identification.
3. Add firmware and API version information.
4. Normalize weight and scale-status data.
5. Improve developer documentation.
6. Add live scale updates.
7. Investigate customer Wi-Fi support.
8. Develop a reliable Wi-Fi setup and recovery process.
9. Add local network discovery.
10. Evaluate Bluetooth/BLE support based on demand.

This list is intended only to organize possible future work. It does not represent a committed development schedule.

# Manual Correction

Some versions of the CS19 manual contain the following sentence:

> The CS19 WiFi will work with Apple, Arduino, and Windows equipment.

`Arduino` is a typo and should read **Android**.

The intended meaning is that the CS19 Wi-Fi/browser interface can be used with common Apple, Android, and Windows devices.

# Version History

See:

`cs19/CHANGELOG.md`

for existing firmware version history and changes.
