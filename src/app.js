var myAPIKey = 'e4584a043dedbb5903f71d99cc204c11';

function iconFromWeatherId(weatherId) {
  if (weatherId < 300) { // Thunder
    return 4;
  } else if (weatherId < 600 || weatherId == 701) { // Rain
    return 3;
  } else if (weatherId < 700) { // Snow
    return 5;
  } else if (weatherId < 800) { // Atmospheric (N/A)
    return 0;
  } else if (weatherId == 800) { // Clear (Sunny)
    return 1;
  } else if (weatherId < 803) { // Partly Cloudy
    return 6;
  } else if (weatherId < 805) { // Cloudy
    return 2;
  } else {
    return 0;
  }
}

function fetchWeather(latitude, longitude) {
  var req = new XMLHttpRequest();
  req.open('GET', 'http://api.openweathermap.org/data/2.5/weather?' +
    'lat=' + latitude + '&lon=' + longitude + '&cnt=1&units=imperial&appid=' + myAPIKey, true);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        console.log(req.responseText);
        var response = JSON.parse(req.responseText);
        var temp_max = Math.round(response.main.temp_max);
        var temp_min = Math.round(response.main.temp_min);
        var icon = iconFromWeatherId(response.weather[0].id);
        console.log("Max: " + temp_max);
        console.log("Min: " + temp_min);
        console.log("Weather: " + icon);
        Pebble.sendAppMessage({
          'KEY_ICON': icon,
          'KEY_TEMPMAX': temp_max,
          'KEY_TEMPMIN': temp_min
        },
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        });
      } else {
        console.log('Error');
      }
    }
  };
  req.send(null);
}

function locationSuccess(pos) {
  var coordinates = pos.coords;
  fetchWeather(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    'KEY_ICON': 0,
    'KEY_TEMPMAX': '',
    'KEY_TEMPMIN': ''
  },
  function(e) {
    console.log("Weather info sent to Pebble successfully!");
  },
  function(e) {
    console.log("Error sending weather info to Pebble!");
  });
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log('PebbleKit JS ready!');
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
  }
);