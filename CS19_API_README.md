# CS19 Integration API

This document is intended for third-party developers integrating with the CS19 scale over its local Wi-Fi connection.

It covers the supported public local API in firmware `1.5.0`:

- `GET /api/v1/scale`
- `GET /api/v1/device`
- `GET /api/v1/capabilities`
- `WS /api/v1/ws`

It does not cover internal compatibility routes used by CS19 browser, remote-display, or printer workflows.

## Quick Start

1. Connect your device to the CS19 Wi-Fi network.
2. Send HTTP requests to `http://192.168.1.184`.
3. Use WebSocket at `ws://192.168.1.184/api/v1/ws` for live updates.

Base URL:

```text
http://192.168.1.184
```

WebSocket URL:

```text
ws://192.168.1.184/api/v1/ws
```

## Endpoints

### `GET /api/v1/scale`

Returns the current machine-readable scale state.

Example response:

```json
{
  "weight": 12.45,
  "units": "lb",
  "unit_mode": "lb",
  "display_weight": "   12.45",
  "locked": false,
  "lock_state": "calculating",
  "stable": false,
  "status": "motion",
  "lock_sequence": 151,
  "gross_net": "gross",
  "gross": true,
  "net": false,
  "valid": false,
  "motion": true,
  "over_under": false,
  "has_signal": true,
  "recent_locked_weights": [12.45, 11.92, 10.37, 9.84, 8.71]
}
```

Field notes:

- `weight`: normalized numeric weight.
- `units`: normalized unit for the numeric `weight`. Currently `lb` or `kg`.
- `unit_mode`: current scale display mode. Currently `lb`, `kg`, or `lb_oz`.
- `display_weight`: human/display-oriented weight string.
- `locked`: `true` when the scale is in its locked state.
- `lock_state`: one of `ready`, `calculating`, or `locked`.
- `stable`: `true` when the scale is in its non-motion, non-over/under reading state.
- `status`: one of `valid`, `motion`, or `over_under`.
- `gross_net`: `gross` or `net`.
- `gross`, `net`, `valid`, `motion`, `over_under`: boolean convenience fields.
- `lock_sequence`: persistent lock counter for that scale.
- `recent_locked_weights`: recent locked weights as normalized numeric values when available.

LB/OZ note:

- When `unit_mode` is `lb_oz`, `weight` is still reported as decimal pounds.
- The conversion is `pounds + ounces / 16`.
- `display_weight` remains display-oriented, for example `12lb 4.0oz`.

### `GET /api/v1/device`

Returns device metadata.

Example response:

```json
{
  "model": "CS19",
  "firmware_version": "1.5.0",
  "api_version": "1.0",
  "device_id": "CS19-1A2B00C0FFEE"
}
```

Field notes:

- `model`: scale model name.
- `firmware_version`: installed firmware version.
- `api_version`: local API version string.
- `device_id`: deterministic device identifier derived from the ESP32 hardware identity.

### `GET /api/v1/capabilities`

Returns a machine-readable summary of currently implemented features.

Example response:

```json
{
  "local_http_api": true,
  "direct_wifi_ap_mode": true,
  "browser_interface": true,
  "legacy_api": true,
  "remote_display_endpoints": true,
  "printer_endpoints": true,
  "websocket_live_updates": true,
  "websocket_endpoint": "/api/v1/ws",
  "server_sent_events": false,
  "station_wifi": false,
  "bluetooth": false
}
```

For third-party integrations, the most useful fields are usually:

- `websocket_live_updates`
- `websocket_endpoint`
- `station_wifi`
- `bluetooth`

### `WS /api/v1/ws`

Provides live scale updates over WebSocket.

Behavior:

- Sends one full JSON snapshot immediately after connect.
- Sends later updates only when scale state changes.
- Sends no more than one broadcast every 250 ms.
- Uses the same JSON shape as `GET /api/v1/scale`.

## Test With curl

Get current scale state:

```bash
curl http://192.168.1.184/api/v1/scale
```

Get device metadata:

```bash
curl http://192.168.1.184/api/v1/device
```

Get capabilities:

```bash
curl http://192.168.1.184/api/v1/capabilities
```

Pretty-print JSON with `jq` if available:

```bash
curl -s http://192.168.1.184/api/v1/scale | jq
```

## JavaScript Examples

### Fetch Current Weight

```js
async function getScale() {
  const response = await fetch("http://192.168.1.184/api/v1/scale");
  if (!response.ok) {
    throw new Error(`HTTP ${response.status}`);
  }

  const data = await response.json();
  console.log("Weight:", data.weight, data.units);
  console.log("Display:", data.display_weight);
  console.log("Locked:", data.locked);
}

getScale().catch(console.error);
```

### Poll Every 500 ms

```js
async function pollScale() {
  try {
    const response = await fetch("http://192.168.1.184/api/v1/scale");
    const data = await response.json();
    console.log(
      `[${new Date().toISOString()}] ${data.weight} ${data.units} locked=${data.locked}`
    );
  } catch (error) {
    console.error("Scale poll failed:", error);
  }
}

setInterval(pollScale, 500);
pollScale();
```

### WebSocket Live Updates

```js
const ws = new WebSocket("ws://192.168.1.184/api/v1/ws");

ws.onopen = () => {
  console.log("Connected to CS19");
};

ws.onmessage = (event) => {
  const data = JSON.parse(event.data);
  console.log("Live update:", data.weight, data.units, data.lock_state);
};

ws.onerror = (error) => {
  console.error("WebSocket error:", error);
};

ws.onclose = () => {
  console.log("Disconnected from CS19");
};
```

## Python Examples

### Read Current Scale State

```python
import requests

response = requests.get("http://192.168.1.184/api/v1/scale", timeout=2)
response.raise_for_status()

data = response.json()
print("Weight:", data["weight"], data["units"])
print("Display:", data["display_weight"])
print("Locked:", data["locked"])
```

Install dependency:

```bash
pip install requests
```

### WebSocket Live Updates

```python
import json
from websocket import WebSocketApp


def on_open(ws):
    print("Connected to CS19")


def on_message(ws, message):
    data = json.loads(message)
    print("Live update:", data["weight"], data["units"], data["lock_state"])


def on_error(ws, error):
    print("WebSocket error:", error)


def on_close(ws, close_status_code, close_msg):
    print("Disconnected from CS19")


ws = WebSocketApp(
    "ws://192.168.1.184/api/v1/ws",
    on_open=on_open,
    on_message=on_message,
    on_error=on_error,
    on_close=on_close,
)

ws.run_forever()
```

Install dependency:

```bash
pip install websocket-client
```

## Typical Integration Pattern

For most apps:

1. Call `GET /api/v1/device` once when connecting.
2. Call `GET /api/v1/capabilities` once to discover supported features.
3. Use `WS /api/v1/ws` for live updates when available.
4. Fall back to polling `GET /api/v1/scale` if WebSocket is not suitable for your client environment.

Recommended polling rate if you do poll:

```text
5 requests per second or slower
```

That is roughly one request every:

```text
200 ms
```

## Common Integration Notes

- The CS19 local API is available over the scale's own Wi-Fi network.
- The scale does not currently provide a cloud API.
- The scale does not currently join customer/home Wi-Fi networks through this API.
- Bluetooth is not currently available as a supported third-party integration path.
- `display_weight` is for display. Use `weight` for calculations.
- `lock_sequence` is useful for detecting new lock events.
- `recent_locked_weights` is useful for history displays or quick operator review.

## Minimal End-to-End Example

This JavaScript example fetches device info once, then listens for live scale updates:

```js
async function start() {
  const deviceResponse = await fetch("http://192.168.1.184/api/v1/device");
  const device = await deviceResponse.json();
  console.log("Connected to", device.model, device.device_id);

  const ws = new WebSocket("ws://192.168.1.184/api/v1/ws");
  ws.onmessage = (event) => {
    const scale = JSON.parse(event.data);
    console.log(
      `${scale.weight} ${scale.units} | ${scale.lock_state} | seq=${scale.lock_sequence}`
    );
  };
}

start().catch(console.error);
```

## Support Checklist

If you are not getting data:

1. Confirm you are connected to the CS19 Wi-Fi network.
2. Confirm `http://192.168.1.184/api/v1/device` responds in a browser or with `curl`.
3. Confirm your client can make plain local HTTP requests to `192.168.1.184`.
4. Confirm your client can open `ws://192.168.1.184/api/v1/ws`.
5. If WebSocket is unavailable in your environment, fall back to polling `GET /api/v1/scale`.
