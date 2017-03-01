
function iconFromWeatherId(weatherId) {
  if (weatherId < 600) {
    return 2;
  } else if (weatherId < 700) {
    return 3;
  } else if (weatherId > 800) {
    return 1;
  } else {
    return 0;
  }
}

function fetchWeather(latitude, longitude) {
  var response;
  var req = new XMLHttpRequest();
   /*req.open('GET', "http://api.openweathermap.org/data/2.1/find/city?" +
    "lat=" + latitude + "&lon=" + longitude + "&cnt=1", true); 
	*/  
  var countryCode='';
  req.open('GET', 'http://freegeoip.net/json/', false); //synchronous
  req.send(null);
  if(req.status == 200) {
	    console.log(req.responseText);
        var response = JSON.parse(req.responseText);
	    if (response){
		    countryCode = response.country_code;
			console.log(countryCode);
		}
   } else { 
		  console.log("GeoIP Error");
	}
  
  //req = new XMLHttpRequest();
 
  req.open('GET', "http://api.openweathermap.org/data/2.5/find?lat="+latitude+"&lon="+longitude+"&cnt=1", true);

  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        var temperature, icon, city;
		
        if (response && response.list && response.list.length > 0) {
		
         var weatherResult = response.list[0]; 
		
		 //var countryCode = weatherResult.sys.country;
	
			
		/*
	
    Fahrenheit Countries:		
	Bahamas - BS
	Belize - BZ
	Cayman Islands -KY
	Palau - PW
	USA - US
	U.S Virgin Islands -VI
	Guam -GU
	Puerto Rico -PR
	*/
		 temperature = Math.round(weatherResult.main.temp - 273.15);
			
		 var fahrenheitFlag=false;
		if (countryCode =='BS' || countryCode =='BZ' || countryCode =='KY' || countryCode =='PW' || countryCode =='US'||countryCode =='VI' || countryCode =='GU' || countryCode =='PR')
		 {
			 fahrenheitFlag=true;
		}	 
        
		 //convert to Fahrenheit
	   if( fahrenheitFlag){
		  temperature = Math.round(( temperature * 9 )/ 5 + 32);
		  temperature=  temperature + "\u00B0F" ;
		}else{
		   temperature =temperature + "\u00B0C";
		}
			
					
         icon = iconFromWeatherId(weatherResult.weather[0].id);
		
		
		  //description = weatherResult.weather[0].description //fog
		 //deg = weatherResult.wind.deg //deg
	     //windspeed = weatherResult.wind.speed //wind speed.
         //humidity = weatherResult.main.humidity
	     //pressure = weatherResult.main.pressure
	     //tem_max= weatherResult.main.temp_max
	    //temp_min = weatherResult.main.temp_min
			
         city = weatherResult.name;
			
          console.log(temperature);
          console.log(icon);
          console.log(city);
		
		  
          Pebble.sendAppMessage({
            "icon":icon,
            "temperature":temperature,
            "city":city });
         }

      } else {
        console.log("Error");
		  
      }
    }
  }
  req.send(null);
}

function locationSuccess(pos) {
  var coordinates = pos.coords;
 
  fetchWeather(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    "city": "Location Error",
    "temperature":"N/A"
  });
}

var locationOptions = { "timeout": 25000, "maximumAge": 60000 }; 


Pebble.addEventListener("ready",
                        function(e) {
                          console.log("connect!" + e.ready);
                          locationWatcher = window.navigator.geolocation.watchPosition(locationSuccess, locationError, locationOptions);
                          console.log(e.type);
                        });

Pebble.addEventListener("appmessage",
                        function(e) {
                          window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
                          console.log(e.type);
                          console.log(e.payload.temperature);
                          console.log("message!");
                        });

Pebble.addEventListener("webviewclosed",
                                     function(e) {
                                     console.log("webview closed");
                                     console.log(e.type);
                                     console.log(e.response);
                                     });


