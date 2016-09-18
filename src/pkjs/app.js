var owmAPIKey = 'e4584a043dedbb5903f71d99cc204c11';

function iconFromWeatherId_owm(weatherId) {
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

function iconFromWeatherId_yahoo(weatherId) {
  if (weatherId <= 4) { // Thunder
    return 4;
  } else if (weatherId <= 12 || weatherId == 40) { // Rain
    return 3;
  } else if (weatherId <= 18) { // Snow
    return 5;
  } else if (weatherId < 26) { // Cloudy
    return 2;
  } else if (weatherId < 30 || weatherId == 44) { // Partly Cloudy
    return 6;
  } else if (weatherId <= 34 || weatherId == 36) { // Clear (Sunny)
    return 1;
  } else if (weatherId <= 39 || weatherId == 45 || weatherId == 47) { // Thunder
    return 4;
  } else if (weatherId <= 43 || weatherId == 46) { // Snow
    return 5;
  } else {
    return 0;
  }
}

function fetchWeather(latitude, longitude) {
  var req = new XMLHttpRequest();
  req.open('GET', 'https://query.yahooapis.com/v1/public/yql?q=select%20*%20from%20' +
           'weather.forecast%20where%20woeid%20in%20(select%20woeid%20from%20geo.' +
           'places(1)%20where%20text%3D%22(' + latitude + ',' + longitude +
           ')%22)&u=f&format=json', true);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        console.log(req.responseText);
        var response = JSON.parse(req.responseText);
        response = response.query.results.channel.item;
        var temp_max = Math.round(response.forecast[0].high);
        var temp_min = Math.round(response.condition.temp);
        var icon = iconFromWeatherId_yahoo(response.forecast[0].code);
        console.log("Max: " + temp_max);
        console.log("Current: " + temp_min);
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