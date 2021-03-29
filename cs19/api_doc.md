# Get Scale Data

Returns json data about the current state of the scale

* **URL**

  /v1/getJSON

* **Method:**

  `GET`
  
*  **URL Params**

   None

* **Data Params**

   None

* **Success Response:**

  * **Code:** 200 <br />
    **Content:** `{"weight":"    0.00","units":"lbs","locked":"READY","lockedodo":151,"lastLocked":["","---","---","---","---"]}`
 
* **Error Response:**

  * **Code:** 404 NOT FOUND 

* **Sample Call:**

   ```jquery
   $.ajax({
    url: "/v1/getJSON",
    dataType: "json",
    type : "GET",
    success : function(r) {
      console.log(r);
    }
  });
  ```

## Notes:

Max rate of query is every 200ms.  Rememember this "server" is running on a microprocessor, we don't have the power of AWS behind us.

- __weight__ will return a string with spaces. 
  - if in LBs or Kg mode, no Units designator will be present.  
  - In Lb/Oz mode units will be present.
- __units__ will be "lbs", "kg", or "" for Pounds, Kilograms and Lbs/Oz respectivly.
- __locked__ will show status of scale, "READY", "Calculating...", or "LOCKED".
- __lockedodo__ will be a persistent counter for total number of times scale is locked.  This INT value can be considered unique on this scale (other scales will use same values).
- __lastLocked__ will be an array of the last locked values where the 0th element is the current or most recent locked value.
  - The 0th Element corrosponds with lockedodo.

## Future Improvements:

- Save lastLocked values in eeprom for recovory even after scale is powered down.
- Send unique scale information eg. mac address, to uniquly identify this scale.


