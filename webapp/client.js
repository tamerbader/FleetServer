/* FIREBASE API */
firebase.initializeApp({
 	apiKey: 'AIzaSyD4Zu6YxrA9AQaeNU1WTj64mzkWnoi_ZAM',
 	authDomain: 'fleet-8b5a9.firebaseapp.com',
 	projectId: 'fleet-8b5a9'
});
var db = firebase.firestore();


/* MAP BOX API */
mapboxgl.accessToken = 'pk.eyJ1IjoiNDA4cmZsZWV0IiwiYSI6ImNqdWJmeXJqdzBkNG40NG8wMXFoZDlqYncifQ.YkRrorh-PE6HVYDtZf1nAw';

// Map Style
MAP_STYLE = {
	// container id specified in the HTML
	container: 'map',
	// style URL
	style: 'mapbox://styles/mapbox/streets-v11',
	// initial position in [long, lat] format
	center: [-76.937498, 38.992132],
	// initial zoom
	zoom: 15
}
var map = new mapboxgl.Map(MAP_STYLE);


/* PARAMETERS */
UPDATE_INTERVAL = 2000; // milliseconds


/* Global Data Structures and Variables */
bike_markers = {};
last_timestamp = 0;


/* Initialization */
$('document').ready(function() {
	init_map();
	console.log("Initialization Complete!");
});

// Initialize Map
function init_map() {
  // Add geolocate control to the map.
  map.addControl(new mapboxgl.GeolocateControl({
    positionOptions: { enableHighAccuracy: true },
    trackUserLocation: true
  }));

  map.doubleClickZoom.disable();

  init_menu();

  // Populate map with data
	get_initial_data();

	setTimeout(begin_update_loop, UPDATE_INTERVAL);
}

// Initialize menu
function init_menu() {
  var layerList = document.getElementById('menu');
  var inputs = layerList.getElementsByTagName('input');
   
  function switchLayer(layer) {
    var layerId = layer.target.id;
    map.setStyle('mapbox://styles/mapbox/' + layerId);
  }
   
  for (var i = 0; i < inputs.length; i++) {
    inputs[i].onclick = switchLayer;
  }
}

function get_initial_data() {
	db.collection("devices").get().then(populate_map);
}

function populate_map(data) {
	data.forEach((doc) => {
		// create a HTML element for marker
		var marker_element = document.createElement('div');
		marker_element.className = 'marker';
    marker_element.dataset.id = doc.id;
    marker_element.dataset.timestamp = doc.data().timestamp;

    marker_element.ondblclick = function(e) {
      center_zoom(e.path[0].dataset.id);
    }

    // Add new marker
    var marker = new mapboxgl.Marker(marker_element)
       .setLngLat([doc.data().lastKnownLongitude, doc.data().lastKnownLatitude])
       .setPopup(new mapboxgl.Popup({offset: 30}).setHTML(generate_popup_HTML(doc.data())))
       .addTo(map);

		// Save marker in data structure
		bike_markers[doc.id] = marker;

    // Update last timestamp
    update_last_timestamp(doc.data().timestamp);
	});

	console.log("Map populated");
}


function begin_update_loop() {
	console.log("Beginning update loop");
	setInterval(update_data, UPDATE_INTERVAL);
}

function update_data() {
	console.log("Periodic Update");

	// Query only entries with update location flag set
	db.collection("devices").get().then((data) => {
    	data.forEach((doc) => {
      	// console.log(doc.id, " => ", doc.data());

      	// Update marker
      	bike_markers[doc.id].setLngLat([doc.data().lastKnownLongitude, doc.data().lastKnownLatitude]);
        bike_markers[doc.id].getPopup().setHTML(generate_popup_HTML(doc.data()));

        // Update global last timestamp
        update_last_timestamp(doc.data().timestamp);
    	});
  	});
}


function update_last_timestamp(candidate_timestamp) {
   if(candidate_timestamp > last_timestamp) {
      last_timestamp = candidate_timestamp;
   }
}

function generate_popup_HTML(data) {
   html = '<h3>' + data.deviceID + '<i class="' + get_battery_class(data.battery) + '"></i>' + '</h3>'+ 
      '<h4>' + '<b>Name: </b>' + data.deviceName + '</h4>'+ 
      '<h4>' + '<b>Loc: </b>(' + data.lastKnownLatitude.toFixed(5) + ', ' + data.lastKnownLongitude.toFixed(5) + ')</h4>' +
      // '<h4>' + "Time: " + (new Date(data.timestamp).toLocaleString()) + '</h4>' +
      '<h4>' + "<b>Last Update: </b>" + get_last_update_string(data.timestamp) + '</h5>'+
      '<h4>' + '<b>Ping: </b>' + '<span id="ping-' + data.deviceID + '">' + (data.pingFrequency + 5) + '</span>' + ' sec</h4>' + 
      '<h4><input type="range" min="5" max="60" value="' + (data.pingFrequency + 5) +
        '" class="slider" id="slider-' + data.deviceID + '" onchange="ping_slider_onchange(' + data.deviceID + ')" oninput="ping_slider_oninput(' + data.deviceID + ')"></h4>' + 
      '<h4>' + '<button class="btn" onclick="send_ring_to_firebase(' + data.deviceID + ')"> <i class="fas fa-bell"></i> &nbsp; Ring</button>'+ '</h4>';
   return html;
}

function get_battery_class(battery) {
  if(battery == 0) {
    return "fas fa-battery fa-battery-slash";
  } else if (battery <= 25) {
    return "fas fa-battery fa-battery-quarter";
  } else if (battery <= 50) {
    return "fas fa-battery fa-battery-half";
  } else if (battery <= 75) {
    return "fas fa-battery fa-battery-three-quarters";
  } else if (battery <= 100) {
    return "fas fa-battery fa-battery-full";
  } else {
    console.log("Invalid battery level:", battery);
    return "";
  }
}

function get_last_update_string(timestamp) {
   prev_date = new Date(timestamp);
   curr_date = new Date();

   elapsed_time = (curr_date.getTime() - prev_date.getTime()) / 1000;

   if(elapsed_time < 10) 
      return "A few sec ago";
   else if(elapsed_time < 60) 
      return Math.floor(elapsed_time) + " sec ago";
   else if(elapsed_time < 60*60) 
      return Math.floor(elapsed_time/60) + " min ago";
   else if(elapsed_time < 60*60*24) 
      return Math.floor(elapsed_time/60/60) + " " + ((Math.floor(elapsed_time/60/60) == 1)?"hour":"hours") + " ago";
   else 
      return Math.floor(elapsed_time/60/60/24) + " " + ((Math.floor(elapsed_time/60/60/24) == 1)?"day":"days") + " ago";
}

function center_zoom(id) {
  var marker = bike_markers[id];
  if(!marker.getPopup().isOpen()) {
    marker.togglePopup();
  }

  var loc = marker.getLngLat();

  map.flyTo({
    center: [loc.lng, loc.lat],
    zoom: 18,
  });
}

function send_ring_to_firebase(id) {
  db.collection("devices").doc(String(id)).update("alarmEnabled", true);
}

function ping_slider_onchange(id) {
  db.collection("devices").doc(String(id)).update("pingFrequency", document.getElementById("slider-" + id).value-5);
}

function ping_slider_oninput(id) {
  document.getElementById("ping-" + id).innerHTML = document.getElementById("slider-" + id).value;
}


