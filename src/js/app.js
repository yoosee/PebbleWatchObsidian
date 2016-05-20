var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var myAPIKey = "40ed40883f0964911396ea2c04020029";

function locationSuccess(pos) {
  // Construct URL
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + myAPIKey;
  
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET',
    function(responseText) {
       // responseText contains a JSON object with weather info
       var json = JSON.parse(responseText);
        
       // Temperature in Kelvin requires adjustment
       var temperature = Math.round(json.main.temp - 273.15);
       console.log('Temperature is ' + temperature);
           
       // Conditions
       var conditions = json.weather[0].main;
       console.log('Conditions are ' + conditions);
      
      var condistions_max_length = 8;
      
      // Assemble dictionary using our keys
      var dictionary = {
          'KEY_TEMPERATURE': temperature,
          'KEY_CONDITIONS': conditions.substring(0, Math.min(condistions_max_length,conditions.length)).toUpperCase()
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary, 
                            function(e) {
                              console.log('Weather info sent to Pebble successfully!');
                            },
                            function(e) {
                              console.log('Error sending weather info to Pebble!');
                            }
                           );
    }
            );
}

function locationError(err) {
  console.log('Error requesting location!');
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

function hashCode(str) { // java String#hashCode
    var hash = 0;
    for (var i = 0; i < str.length; i++) {
       hash = str.charCodeAt(i) + ((hash << 5) - hash);
    }
    return hash;
} 

function intToRGB(i){
    var c = (i & 0x00FFFFFF)
        .toString(16)
        .toUpperCase();

    return "00000".substring(0, 6 - c.length) + c;
}

Pebble.addEventListener('showConfiguration', function() {
  var url = 'https://rawgit.com/yoosee/PebbleWatchObsidian/master/config/index.html'; // development
  // var url = 'https://cdn.rawgit.com/yoosee/PebbleWatchObsidian/master/config/index.html' // produuction
  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));
  
  var backgroundColor = configData['background_color'];
  var is_fahrenheit = configData['is_fahrenheit'] ? 1 : 0; // Send a boolean as an integer
  
  var dictionary = {
    'KEY_IS_FAHRENHEIT': is_fahrenheit,
    'KEY_BACKGROUND_COLOR': intToRGB(hashCode(backgroundColor))
  };
  
  // Send to watchapp
  Pebble.sendAppMessage(dictionary, function() {
    console.log('Send successful: ' + JSON.stringify(dictionary));
  }, function() {
    console.log('Send failed!');
  });
});

