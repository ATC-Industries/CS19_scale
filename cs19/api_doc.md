# CS19 Local API Documentation

The CS19 exposes a small HTTP interface over its local Wi-Fi network.

The primary endpoint intended for software integrations is:

```http
GET http://192.168.1.184/v1/scale/data
```

Unless otherwise noted, the endpoints documented here are served directly by the CS19 controller over HTTP port 80.

> **Important:** Some older endpoints remain in the firmware for compatibility with existing CS19 components and integrations. New software should use `/v1/scale/data` unless there is a specific reason to use one of the legacy interfaces.

---

# Supported Integration API

## Get Scale Data

### Endpoint

```http
GET /v1/scale/data
```

### Description

Returns the current CS19 scale state as JSON.

This is the **preferred endpoint for new software integrations**.

### Parameters

None.

### Example Request

```http
GET http://192.168.1.184/v1/scale/data
```

### Example Response

```json
{
  "weight": "    0.00",
  "units": "lbs",
  "locked": "READY",
  "lockedodo": 151,
  "lastLocked": ["", "---", "---", "---", "---"]
}
```

### Response Fields

#### `weight`

Current weight as a string.

Example:

```json
"weight": "   12.45"
```

The value may contain leading spaces because the current API exposes weight using the same formatting used elsewhere by the CS19 display system.

Applications should currently treat this as a formatted string rather than assuming it is already a normalized numeric value.

---

#### `units`

Current scale units.

Possible values include:

```text
lbs
kg
""
```

An empty string is currently used when the scale is operating in pounds/ounces mode.

---

#### `locked`

Current lock state.

Known values include:

```text
READY
Calculating...
LOCKED
```

`READY` generally indicates that the scale is not locked and is at or below zero.

`Calculating...` indicates weight is present but the scale has not reached its locked condition.

`LOCKED` indicates that the scale has reached its locked/stable state.

---

#### `lockedodo`

Persistent counter representing the number of lock events recorded by that scale.

Example:

```json
"lockedodo": 151
```

The value increments when the scale enters a new locked state.

The counter is persistent across power cycles.

It should not be considered globally unique across different CS19 units.

---

#### `lastLocked`

Array containing the most recent locked weight values.

Example:

```json
"lastLocked": [
  "12.45",
  "11.92",
  "10.37",
  "9.84",
  "8.71"
]
```

The first element represents the current or most recent locked weight.

Older values follow in descending order of recency.

These values may also contain display-oriented formatting.

---

# Polling Rate

The HTTP server runs directly on the embedded controller inside the CS19.

Applications should avoid unnecessarily aggressive polling.

The current recommended maximum rate is approximately:

```text
1 request every 200 ms
```

or approximately:

```text
5 requests per second
```

Applications that do not require extremely frequent updates should poll more slowly.

---

# Deprecated API

The following endpoints remain in the firmware primarily for backward compatibility.

New integrations should avoid depending on them.

## `/v1/getJSON`

```http
GET /v1/getJSON
```

**Status: Deprecated**

This endpoint currently returns the same JSON data as:

```http
GET /v1/scale/data
```

New applications should use:

```http
/v1/scale/data
```

instead.

---

# Legacy Individual Data Endpoints

Before the combined JSON endpoint was introduced, individual pieces of scale information could be retrieved through separate HTTP requests.

These routes remain present but should generally be considered **legacy interfaces**.

New applications should use `/v1/scale/data` instead.

## `/weight`

```http
GET /weight
```

Returns the current formatted weight as plain text.

Example:

```text
   12.45
```

**Status: Legacy / Deprecated for new integrations**

Use `/v1/scale/data` instead.

---

## `/getUnits`

```http
GET /getUnits
```

Returns the current unit designation as plain text.

Possible values include:

```text
lbs
kg
```

An empty response may represent pounds/ounces mode.

**Status: Legacy / Deprecated for new integrations**

Use the `units` field from `/v1/scale/data` instead.

---

## `/isLocked`

```http
GET /isLocked
```

Returns the current scale lock state as plain text.

Known values include:

```text
READY
Calculating...
LOCKED
```

**Status: Legacy / Deprecated for new integrations**

Use the `locked` field from `/v1/scale/data` instead.

---

## `/getLastLocked`

```http
GET /getLastLocked
```

Returns the most recent locked weight as plain text.

**Status: Legacy / Deprecated for new integrations**

Use the `lastLocked` array from `/v1/scale/data` instead.

---

## `/getLockedOdo`

```http
GET /getLockedOdo
```

Returns the persistent lock counter as plain text.

**Status: Legacy / Deprecated for new integrations**

Use the `lockedodo` field from `/v1/scale/data` instead.

---

# Internal and Compatibility Endpoints

The following endpoints exist in the current firmware but are not intended to be treated as the general-purpose public CS19 integration API.

They support existing CS19 accessories, remote displays, printer behavior, or legacy hardware.

Their behavior may be tightly coupled to those systems.

Third-party applications should avoid relying on these routes unless they specifically need to emulate or integrate with the corresponding CS19 accessory.

---

## `/getlegacyweight`

```http
GET /getlegacyweight
```

Returns weight information formatted for legacy remote-display/radio compatibility.

**Status: Legacy internal interface**

Not recommended for new third-party integrations.

---

## `/getmaxmode`

```http
GET /getmaxmode
```

Returns the number of display modes supported by the remote-display interface.

Current firmware returns:

```text
4
```

**Status: Internal remote-display interface**

Not intended as part of the general-purpose CS19 API.

---

## `/remote`

```http
POST /remote
```

Used by supported remote-display hardware to request scale information based on a display mode sent in the request body.

**Status: Internal remote-display interface**

The request and response format is designed around existing remote display behavior and is not recommended for new software integrations.

---

## `/remoteMode`

```http
POST /remoteMode
```

Existing compatibility route associated with remote-display operation.

**Status: Internal / Legacy**

Not recommended for third-party integrations.

---

## `/islockedandprintpressed`

```http
GET /islockedandprintpressed
```

Used as part of existing printer/manual-print behavior.

Calling this endpoint also changes internal print-state behavior.

**Status: Internal printer interface**

Third-party applications should not use this as a general lock-status endpoint.

Use:

```http
/v1/scale/data
```

instead.

---

## `/islockedandauto`

```http
GET /islockedandauto
```

Used as part of the CS19 automatic-print workflow.

This endpoint changes internal printer behavior in addition to returning status information.

**Status: Internal printer interface**

Not intended for general application integrations.

---

# Browser Interface Endpoints

The firmware also serves the built-in CS19 browser interface.

These routes are not API endpoints.

## `/`

```http
GET /
```

Returns the main CS19 browser interface.

---

## Static Assets

The following routes supply files used by the browser interface:

```text
/bootstrap.min.css
/jquery-3.3.1.slim.min.js
/jquery-slim.min.js
/popper.min.js
/bootstrap.min.js
```

These are implementation details of the current browser interface and should not be used as part of an external application integration.

---

# HTTP Errors

Requests to unknown routes currently return:

```http
404 Not Found
```

The current firmware does not yet provide a structured JSON error format.

---

# Current API Limitations

The existing API was originally developed to expose CS19 scale information locally rather than as a complete third-party developer platform.

Known limitations include:

- Weight is returned as a formatted string rather than a normalized numeric value.
- Pounds/ounces mode is represented indirectly through the current unit and formatting conventions.
- Scale/device identity is not exposed.
- Firmware version is not exposed through the API.
- API version information is not returned in responses.
- There is no capabilities endpoint.
- There is no structured error-response format.
- Applications currently need to poll for changes.
- There is no formal event/subscription interface.
- The API is available only through the CS19's local Wi-Fi network.
- The CS19 cannot currently join an arbitrary customer Wi-Fi network.
- There is no supported Bluetooth API.

---

# Future Improvement Ideas

The following items are possible areas for future development.

They are **not committed features or a development schedule**.

## More Formal Versioned API

A future API revision could establish a consistent namespace such as:

```text
/api/v1/
```

while retaining existing endpoints for backward compatibility where practical.

Potential endpoints could include:

```http
GET /api/v1/scale
GET /api/v1/device
GET /api/v1/capabilities
```

The exact structure has not been determined.

---

## Normalized Measurement Data

Future responses could provide numeric values separately from human-readable/display formatting.

For example:

```json
{
  "measurement": {
    "weight": 12.45,
    "unit": "lb"
  }
}
```

This would remove the need for third-party applications to parse leading spaces or display-formatted weight strings.

For pounds/ounces operation, the API could provide explicit structured values rather than relying on presentation formatting.

---

## Improved Scale Status

A future API could separate scale state into explicit machine-readable properties instead of requiring applications to interpret strings such as:

```text
READY
Calculating...
LOCKED
```

For example:

```json
{
  "status": {
    "stable": true,
    "locked": true
  }
}
```

Additional scale status such as motion, valid reading, over-range, under-range, gross/net state, or signal availability could potentially be exposed where useful.

---

## Device Identification

Each CS19 should eventually expose a stable unique device identifier.

For example:

```json
{
  "device": {
    "id": "CS19-XXXXXX",
    "model": "CS19"
  }
}
```

This becomes particularly important when an application can communicate with multiple scales.

---

## Firmware and API Version Information

Future responses could expose both firmware and API versions.

Example:

```json
{
  "device": {
    "firmware": "1.3.0",
    "api": "1.0"
  }
}
```

This would allow client applications to account for differences between firmware releases.

---

## Capabilities

A future capabilities endpoint could allow applications to determine which functionality a particular scale supports.

Example concept:

```http
GET /api/v1/capabilities
```

Possible information could include:

```json
{
  "wifi": true,
  "stationMode": false,
  "bluetooth": false,
  "liveEvents": false
}
```

The exact structure has not been defined.

---

## Live Updates

The current integration model requires polling.

A future firmware version could potentially support event-driven updates using technologies such as:

- WebSockets
- Server-Sent Events

This could allow applications to receive weight and lock-state changes as they occur rather than requesting `/v1/scale/data` repeatedly.

A possible architecture would use:

```text
REST/JSON
```

for device information and current state, with:

```text
WebSocket or event stream
```

for live scale updates.

---

## Customer Wi-Fi Support

The CS19 currently creates its own Wi-Fi network.

A future firmware revision could investigate allowing the unit to optionally join an existing customer or venue Wi-Fi network.

Such a system would need to include a reliable configuration and recovery mechanism before being considered suitable for production use.

The existing direct-connect CS19 Wi-Fi behavior should preferably remain available for backward compatibility and recovery.

---

## Local Device Discovery

If the CS19 gains the ability to join another Wi-Fi network, applications should not require users to manually determine the scale's dynamically assigned IP address.

Future development could investigate local device discovery using a hostname, mDNS, service discovery, or a similar mechanism.

This could allow an application to discover available CS19 units automatically.

---

## Bluetooth / BLE

The hardware platform includes Bluetooth capability.

Bluetooth or Bluetooth Low Energy could potentially be investigated as another local communication method for future mobile integrations.

There is currently no supported Bluetooth API and no committed Bluetooth development plan.

---

## Backward Compatibility

Future API development should attempt to preserve existing integrations where practical.

In particular, changes should avoid unnecessarily breaking applications already using:

```http
/v1/scale/data
```

New functionality should preferably be introduced through new versioned interfaces rather than silently changing the meaning or format of an existing response.

---

# Recommended Endpoint for New Integrations

For current CS19 firmware, new applications should use:

```http
GET http://192.168.1.184/v1/scale/data
```

All other endpoints should be considered either legacy, internal, browser-related, or specialized unless otherwise documented.
