#ifndef TV_H
#define TV_H

#include <Arduino.h>

// Index HTML
static const char tv_html[] PROGMEM = R"rawliteral(

<!DOCTYPE HTML>
<html lang="en">

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <meta name="apple-mobile-web-app-capable" content="yes">
  <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="HandheldFriendly" content="true">
  <meta name="MobileOptimized" content="width">
  <link rel="icon" href="https://cdn11.bigcommerce.com/s-190w8a8x/product_images/favicon.png">
  <title>Pro Tournament Scales RemoteView</title>
  <!-- Bootstrap CSS -->
  <!-- Bootstrap core CSS -->
  <link href="bootstrap.min.css" rel="stylesheet">

  <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css" integrity="sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T" crossorigin="anonymous">
 <style>
    body, html {
      height: 100vh;
      margin: 0;
      background-color: #000;
      color: #fff;
    }
    .navbar, .p-3, .units, #locked {
      background-color: #000 !important;
      color: #fff !important;
    }
    #weight {
      font-size: 20vw; /* Adjust the font size as needed */
      font-family: Verdana, Arial, Helvetica, sans-serif;
      font-weight: 900;
      color: #ffffff;
    }
    .units {
      font-size: 7vw; /* Adjust the unit's font size as needed */
    }
    #locked {
      font-size: 7vw; /* Adjust according to your preference */
      color: #ffffff;
    }
   .lower-right {
    position: absolute;
    top: 90vh;
    right: 0vh;
   }
  </style>
</head>

<body>
  <!-- Image and text -->
  <nav class="navbar navbar-dark bg-dark"><span class="badge badge-pill badge-danger">BETA</span>
     <h1 class="p-3 mb-2 bg-dark text-white text-center">LIVE WEIGHT</h1>
    <a class="navbar-brand d-lg-none" href="#">
      <div style="width: 75px; float: left;"><a href="/">
          <svg id="Layer_1" data-name="Layer 1" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1577.22 1100.85">
            <title>Pro Tournament Scales</title>
            <path d="M1581.78,259.77" transform="" style="fill: #275589" />
            <path d="M1544.44,434.34" transform="" style="fill: #fff200" />
            <path d="M829.65,1104.3" transform="" style="fill: #275589" />
            <path d="M831.39,1100.59" transform="" style="fill: #275589" />
            <path d="M1538.69,242.23a27.37,27.37,0,0,0-2.84-.06l-.06-.06H1284.7l1.84-8.91,37.91-183S1331.16,0,1283.6,0H545.79c-34.51,0-40.85,33.25-40.85,33.25L467,216.27l-5.15,24.85H174.18s-33.48-1.24-40.8,33l-23.71,111L98.9,435.56l-3,14.2-94,444s-15.32,50.43,40.8,50.43H175.24s33,3.33,40.8-33l40.37-189H440.34c38.52-3.75,60.17,6.51,71.71,14.32,78.72,53.28,46.42,235.56,32.55,313.84-.25,1.41.2,2.9.33,4.36-.08.48-.19.78-.27,1.29-6.89,46.47,40.77,44.59,40.77,44.59h212s32.79,5.4,40.84-33.19l24.84-119h387.07c33.44,0,60.78-1.75,83.58-5.36,27-4.27,50.07-13.31,68.69-26.89,19-13.87,34.13-32.68,45-55.93,9.24-19.72,17.23-44.47,24.41-75.65l22.74-103.89a236.78,236.78,0,0,0,4.2-44.48c0-60.83-33.53-97.9-33.53-97.9-41.59-46.3-107.7-39.77-114.68-39a338.1,338.1,0,0,1,30.64-18.91c48-26.18,95.3-36.76,135.11-40.71v-.12a40.66,40.66,0,0,0,35.6-30.8c.31-1.42.61-2.83.92-4.25l23.71-112.08a37.6,37.6,0,0,0,.48-9.81C1575.39,262.39,1556.82,243.51,1538.69,242.23ZM568,656.23q-19.95,14.55-50.64,19.4t-77.05,4.84H222.66l-47.42,222H42.71l94.82-444H270.09L246.37,568.39h168.1a347.87,347.87,0,0,0,37.18-1.61q14.55-1.62,23.71-8.08t14-19.4q4.83-12.91,10.22-35.56l9.7-45.27q2.18-11.82,3.77-20.47a79,79,0,0,0,1.63-14c0-12.23-4.16-20.29-12.41-24.27s-25-5.9-50.09-5.9H150.47l23.71-111H524.39q74.36,0,104,14.54T658,355q0,24.81-7.54,59.28L621.38,549q-9.72,42-21.55,67.35T568,656.23Zm229.9,400.46c-6.51.46-13.56,1.21-21.06,2.18H585.43L759.54,224.74H507.86l37.93-183H1283.6l-37.91,183H971.58ZM1512.08,395.9H1203.89a347.66,347.66,0,0,0-37.17,1.61q-14.55,1.62-23.72,8.09T1128.46,425q-5.42,12.93-9.7,35.57l-3.24,14q-5.4,26.94-5.39,35.56,0,18.35,12.93,23.71t50.64,5.38h180q103.42,0,103.44,97a192.17,192.17,0,0,1-3.24,35.57l-22.63,103.44q-9.69,42-21.55,67.34t-31.78,39.89q-20,14.55-50.64,19.39t-77.06,4.85H891.39L915.1,794.6h309.26a348.13,348.13,0,0,0,37.18-1.61q14.55-1.62,23.71-8.09c6.09-4.29,10.78-10.77,14-19.39s6.64-20.47,10.24-35.55l3.22-14q5.41-26.93,5.4-36.65,0-28-36.65-28H1091.81q-58.17,0-91.58-24.79t-33.41-66.81a90.26,90.26,0,0,1,.53-10.23c.36-3.24.89-6.64,1.63-10.25L995.91,415.3q9.72-42,22.1-67.9t32.33-39.86q19.92-14,50.64-18.87t76-4.84h358.85Z" transform="" style="fill: #275589" />
          </svg></a>
      </div>
      <!--// style -->
    </a>
  </nav>

  <p class="p-3 display-3 text-center">
<!--     <span id="weight">100.00</span>
    <sup class="units" id="units">lb</sup> -->
        <span id="weight">%WEIGHT%</span>
    <sup class="units" id="units">%UNIT%</sup>
  </p>
  <div id="locked">%LOCKED%</div>

  
<footer class="d-flex flex-wrap justify-content-between align-items-center py-3 my-4 border-top fixed-bottom bg-dark">
    <p class="col-md-3 mb-0 text-muted">&copy; 2024 Pro Tournament Scales</p>
    <p class="col-md-3 mb-0 text-muted text-center"><small>FW: <span id="version">%VERSION%</span> </small></p>
    <p class="col-md-3 mb-0 text-muted text-right"><small>LC: <span id="lockedCounter">%LOCKODO%</span> </small></p>
</footer>


  <!-- Bootstrap core JavaScript
    ================================================== -->
  <!-- Placed at the end of the document so the pages load faster -->
  <script src="jquery-3.3.1.slim.min.js"></script>
  <script>
    window.jQuery || document.write('<script src="jquery-slim.min.js"><\/script>')
  </script>
  <script src="popper.min.js"></script>
  <script src="bootstrap.min.js"></script>

  <!-- Optional JavaScript -->
  <!-- jQuery first, then Popper.js, then Bootstrap JS -->
  <script src="https://code.jquery.com/jquery-3.3.1.slim.min.js" integrity="sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo" crossorigin="anonymous"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.7/umd/popper.min.js" integrity="sha384-UO2eT0CpHqdSJQ6hJty5KVphtPhzWj9WO1clHTMGa3JDZwrnQq4sF86dIHNDz0W1" crossorigin="anonymous"></script>
  <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/js/bootstrap.min.js" integrity="sha384-JjSmVgyd0p3pXB1rRibZUAYoIIy6OrQ6VrjIEaFf/nJGzIxFDsf4x0xIM+B07jRM" crossorigin="anonymous"></script>

</body>
<script>
  // Get Weight
  setInterval(function() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        var data = JSON.parse(this.responseText);
        processData(data);
        //console.log(data);
        //document.getElementById("weight").innerHTML = data.weight;
      }
    };
    xhttp.open("GET", "/v1/getJSON", true);
    xhttp.send();
  }, 200);

  function processData(data) {
    document.getElementById("weight").innerHTML = data.weight;

    document.getElementById("units").innerHTML = data.units;
    document.getElementById("locked").innerHTML = data.locked;
    if (data.locked == "LOCKED") {
      document.getElementById("locked").className = "p-3 mb-2 bg-danger text-white display-4 text-center font-weight-bold";
    } else if (data.locked == "READY") {
      document.getElementById("locked").className = "p-3 mb-2 bg-primary text-white display-4 text-center font-weight-bold";
    } else if (data.locked == "No Signal") {
      document.getElementById("locked").className = "p-3 mb-2 bg-warning text-dark display-4 text-center font-weight-bold";
    } else {
      document.getElementById("locked").className = "p-3 mb-2 bg-success text-white display-4 text-center font-weight-bold";
    }
    document.getElementById("lockedCounter").innerHTML = data.lockedodo;
  }
</script>

</html>


)rawliteral";

/*--------------------------------------------------------------------------------------------------------------------------------


<!-- REM out for now -->
<!--FAQ -->
<h3 class="p-3 mb-2 bg-dark text-white text-center">FAQ</h3>
<div class="accordion" id="accordionExample">
  <div class="card">
    <div class="card-header" id="headingOne">
      <h2 class="mb-0">
        <button class="btn btn-link collapsed" type="button" data-toggle="collapse" data-target="#collapseOne" aria-expanded="false" aria-controls="collapseOne">
          I can't get on the internet/I don't have internet at my event/the screen look a little funny
        </button>
      </h2>
    </div>

    <div id="collapseOne" class="collapse show" aria-labelledby="headingOne" data-parent="#accordionExample">
      <div class="card-body">
        The CS19 Scale creates a local Wi-Fi Access point that allows the scale, peripherals and mobile devices to communicate.  Since it relys on Wi-Fi it will you your devices Wi-Fi connection.  If you have a mobile device that also has celluar date you should be able to use that for all your other internet needs.  Rememeber Wi-Fi is not the same thing as the internet.  It's for this reason we don not reccomend using a PC, Laptop or tablet as you will not be able to connect to the internet while connected to the CS19.
      </div>
    </div>
  </div>
  <div class="card">
    <div class="card-header" id="headingTwo">
      <h2 class="mb-0">
        <button class="btn btn-link collapsed" type="button" data-toggle="collapse" data-target="#collapseTwo" aria-expanded="false" aria-controls="collapseTwo">
          Can I save a link to this page on my phones homescreen?
        </button>
      </h2>
    </div>
    <div id="collapseTwo" class="collapse" aria-labelledby="headingTwo" data-parent="#accordionExample">
      <div class="card-body">
        <p>You sure can!</p>
        <h4>On iPhone</h4>
        <ol>
        <li>Open Safari. It's a blue app with a compass icon on it.</li>
        <li>Make sure you are on this page.</li>
        <li>Tap the "Share" button.</li>
        <li>Scroll right and tap Add to Home Screen.</li>
        <li>Type in a new name for your shortcut if you wish.</li>
        <li>Tap Add.</li>
        </ol>
        <h4>On Android</h4>
        <ol>
        <li>Navigate to your Android web browser. Locate the icon that looks like a globe and tap on it to open.</li>
        <li>Make sure you are on this page.</li>
        <li>Enter the name of the website in the text bar and press "Enter" or "Go."</li>
        <li>Tap on the Create Bookmark icon.</li>
        <li>Tap on the drop-down menu.</li>
        <li>Tap "Home Screen."</li>
        </ol>
      </div>
    </div>
  </div>
  <div class="card">
    <div class="card-header" id="headingThree">
      <h2 class="mb-0">
        <button class="btn btn-link collapsed" type="button" data-toggle="collapse" data-target="#collapseThree" aria-expanded="false" aria-controls="collapseThree">
          I have an idea for a new feature.<br>Will you be updating this with more features.
        </button>
      </h2>
    </div>
    <div id="collapseThree" class="collapse" aria-labelledby="headingThree" data-parent="#accordionExample">
      <div class="card-body">
        Great! and Yes!  Please email adam@tournamentscales.com with all your great ideas.  This is a system for YOU! We want to make it what you want.<br> currently this is the first beta release and we wanted to have all the core functionality perfected before we started adding awesome new things.  So please email us your ideas and if you see any problems please let us know that too!      </div>
    </div>
  </div>
</div>
-->-----------------------------------------------------------------------------------------------------------------------------*/

#endif