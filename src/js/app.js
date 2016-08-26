var WEATHER_SERVICE_GOV = 'GOV';
var WEATHER_SERVICE_OPENWEATHER = 'OW';
var WEATHER_SERVICE_WUNDERGROUND = 'WU';

// Configuration Valuables
var dataStore = {
  weather_service: WEATHER_SERVICE_GOV, 
  openWeather_apikey: "40ed40883f0964911396ea2c04020029",
  wu_apikey: "", // WUnderground is billed service, while it has $0 plan.
};

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var locationSuccess = function(pos) {
  if (dataStore.weather_service == WEATHER_SERVICE_GOV) {
    fetchWeatherGovWeather(pos.coords.latitude, pos.coords.longitude);
  } else if (dataStore.weather_service == WEATHER_SERVICE_OPENWEATHER) {
    fetchOpenWeather(pos.coords.latitude, pos.coords.longitude);
  } else if (dataStore.weather_service == WEATHER_SERVICE_WUNDERGROUND) {
    fetchWUWeather(pos.coords.latitude, pos.coords.longitude);
  }
  
};

var locationError = function(err) {
  console.log('Error requesting location!');
};

var getWeather = function() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
};
             
var fetchWeather = function(params) {
  xhrRequest(params.url, 'GET', function(responseText){
    var weather = params.parse(responseText);
      
    console.log('[fetchWeather] Temperature is ' + weather.temperature);
    console.log('[fetchWeather] Conditions are ' + weather.conditions);

    var condistions_max_length = 8;   
    // Assemble dictionary using our keys
    var dictionary = {
      'KEY_TEMPERATURE': weather.temperature, // Celcius
      'KEY_CONDITIONS': weather.conditions.substring(0, Math.min(condistions_max_length,weather.conditions.length)).toUpperCase()  
    };
    // Send to Pebble
    Pebble.sendAppMessage(dictionary, 
                          function(e) { console.log('Weather info sent to Pebble successfully!');  },
                          function(e) { console.log('Error sending weather info to Pebble!'); }
                       );    
  });
};

function fetchOpenWeather(lat, lon) {
  var params = {};
  params.url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + lat + '&lon=' + lon + '&appid=' + dataStore.openWeather_apikey;
  console.log('URL: ' + params.url);
  params.parse = function(responseText) {
       var json = JSON.parse(responseText);
       var temperature = Math.round(json.main.temp - 273.15); // returned value is Kelvin
       var conditions = json.weather[0].main;
       console.log('Temperature is ' + temperature);
       console.log('Conditions are ' + conditions);
      return {
                conditions: conditions,
                temperature: temperature
      };  
  };
  fetchWeather(params);
}
             
function fetchWUWeather(lat, lon) {
  var params = {};
  params.url = 'http://api.wunderground.com/api/' + dataStore.wu_apikey + '/conditions/astronomy/q/' + lat + ',' + lon + '.json';
  console.log('URL: ' + params.url);
  params.parse = function(responseText) {
              var json = JSON.parse(responseText);
              var temperature = json.current_observation.temp_c;
              var conditions = json.current_observation.weather;
              console.log('Temperature is ' + temperature);
              console.log('Conditions are ' + conditions); 
              return {
                conditions: conditions,
                temperature: temperature
              };          
  };
  fetchWeather(params);
}

var fetchWeatherGovWeather = function(lat, lon) {

  var params = {};
  params.url = 'http://forecast.weather.gov/MapClick.php?lat=' + lat + '&lon=' + lon + '&FcstType=json';
  console.log('URL: ' + params.url);
  params.parse = function(responseText) {
              var json = JSON.parse(responseText);
              var temperature = Math.round((json.currentobservation.Temp - 32.0) * 5 / 9 ); // F to C
              //var conditions = json.currentobservation.Weather;
              var conditions = translateIconToWeather(json.currentobservation.Weatherimage);
              console.log('Temperature is ' + temperature);
              console.log('Conditions are ' + conditions);
              return {
                conditions: conditions,
                temperature: temperature
              };          
  };    
  fetchWeather(params);
};

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    // Get the initial weather
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

// Configuration

Pebble.addEventListener('showConfiguration', function() {
  //var url = 'https://rawgit.com/yoosee/PebbleWatchObsidian/master/config/index.html'; // development
  var url = 'https://cdn.rawgit.com/yoosee/PebbleWatchObsidian/4da3aa36bba44ddb6a05417cca57afd5135799a4/config/index.html' // produuction URL in rawgit.com should be unique like commit hash.
  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));
  
  //var background_color = configData['background_color'];
  var is_fahrenheit = configData['is_fahrenheit'] ? 1 : 0; // Send a boolean as an integer
  var is_steps_enabled = configData['is_steps'] ? 1 : 0; 
  
  var dictionary = {
    'KEY_IS_FAHRENHEIT': is_fahrenheit,
    'KEY_IS_STEPS_ENABLED': is_steps_enabled,
    'KEY_COLOR_BACKGROUND': parseInt(configData['background_color']),
    'KEY_COLOR_FACE': parseInt(configData['face_color']),
    'KEY_COLOR_STEPS': parseInt(configData['steps_color']),
    'KEY_COLOR_WEATHER': parseInt(configData['weather_color']),
    'KEY_COLOR_HOURHAND': parseInt(configData['hourhand_color']),
    'KEY_COLOR_MINUTEHAND': parseInt(configData['minutehand_color']) >= 0 ? parseInt(configData['minutehand_color']) : -1
  };
  
  // Send to watchapp
  Pebble.sendAppMessage(dictionary, function() {
    console.log('Send successful: ' + JSON.stringify(dictionary));
  }, function() {
    console.log('Send failed!');
  });
});

// Conversion from Icon image name to Weather condition strings, for Weather.GOV service.
// http://w1.weather.gov/xml/current_obs/weather.php
var translateIconToWeather = function (icon) {

  iconname = icon.replace(/\.png/, "").replace(/^n/, ""); // remove suffix and prefix of 'n' represents 'Night', not necessary for weather condition strings.
  
  var cond = {
    'bkn': 'Cloud',
    'skc': 'Clear',
    'few': 'Few',
    'sct': 'Scatter',
    'ovc': 'Cloud',
    'fg': 'Fog',
    'smoke': 'Smoke',
    'fzra': 'Freeze',
    'ip': 'Ice',
    'mix': 'Ice',
    'raip': 'Rain Ice',
    'rasn': 'RainSnow',
    'shra': 'Showers',
    'tsra': 'ThunderS',
    'sn': 'Snow',
    'wind': 'Windy',
    'hi_shwrs': 'Cloud', // Showers in Vicinity
    'hi_nshwrs': 'Cloud', // Showers in Vicinity
    'fzrara': 'Freeze',
    'hi_tsra': 'Thunder', // Thunderstorm in Vicinity
    'hi_ntsra': 'Thunder', // Thunderstorm in Vicinity
    'ra1': 'Mist',
    'ra' : 'Rain',
    'nsvrtsra': 'Tornado',
    'dust': 'Dust',
    'mist': 'Haze'
  };
  return cond[iconname];
};
