/* FIREBASE */
firebase.initializeApp({
 	apiKey: 'AIzaSyD4Zu6YxrA9AQaeNU1WTj64mzkWnoi_ZAM',
 	authDomain: 'fleet-8b5a9.firebaseapp.com',
 	projectId: 'fleet-8b5a9'
});
var db = firebase.firestore();


/* MAP BOX */
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

//Click on the map - centers on location


/* PARAMETERS */
UPDATE_INTERVAL = 10000; // milliseconds


/* Global Data Structures and Variables */
bike_markers = {};
last_timestamp = 0;
last_touch_timestamp = 0;


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

  init_menu();

  // Disable double click zoom functionality
  map.doubleClickZoom.disable();

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

    marker_element.ondblclick = function(e) {
      center_zoom(e.path[0].dataset.id);
    }

    marker_element.ontouchstart = function(e) {
      var now = new Date().getTime()
      var time_elapsed = now - last_touch_timestamp;
      last_touch_timestamp = new Date().getTime();

      if((time_elapsed < 300) && (time_elapsed > 0)) {
        center_zoom(e.target.dataset.id);  
      }
      
      last_touch_timestamp = now
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

// Click - center 
// marker.addEventListener("click", function (e){
//   map.flyTo(this.getLatLng());
// });


function begin_update_loop() {
	console.log("Beginning update loop");
	setInterval(update_data, UPDATE_INTERVAL);
}

function update_data() {
	console.log("Periodic Update");

	// Query only entries with update location flag set
	db.collection("devices").where("timestamp", ">", last_timestamp).get().then((data) => {
    	data.forEach((doc) => {
      	console.log(doc.id, " => ", doc.data());

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
   html = '<h3>' + data.deviceID + " &nbsp;" + '<i class="fas fa-battery-three-quarters"></i>' + '</h3>'+ 
      '<h4>' + data.deviceName + '</h4>'+ 
      '<h4>' + "Loc: " + data.lastKnownLatitude + ', ' + data.lastKnownLongitude + '</h4>' +
      // '<h4>' + "Time: " + (new Date(data.timestamp).toLocaleString()) + '</h4>' +
      '<h4>' + "Last Update: " + get_last_update_string(data.timestamp) + '</h5>'+
      '<h4>' + 'Ping: ' + '<span id="ping-' + data.deviceID + '">' + data.pingFrequency + '</span>' + ' sec</h4>' + 
      '<h4><input type="range" min="5" max="60" value="' + data.pingFrequency + 
        '" class="slider" id="slider-' + data.deviceID + '" onchange="ping_slider_onchange(' + data.deviceID + ')" oninput="ping_slider_oninput(' + data.deviceID + ')"></h4>' + 
      '<h4>' + '<button class="btn" onclick="send_ring_to_firebase(' + data.deviceID + ')"> <i class="fas fa-bell"></i> &nbsp; Ring</button>'+ '</h4>';
   return html;
}

function get_last_update_string(timestamp) {
   prev_date = new Date(timestamp);
   curr_date = new Date();

   elapsed_time = (curr_date.getTime() - prev_date.getTime()) / 1000;

   if(elapsed_time < 10) 
      return "A few seconds ago";
   else if(elapsed_time < 60) 
      return Math.floor(elapsed_time) + " seconds ago";
   else if(elapsed_time < 60*60) 
      return Math.floor(elapsed_time/60) + " " + ((Math.floor(elapsed_time/60) == 1)?"minute":"minutes") + " ago";
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
    zoom: 15,
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


/* UNUSED */

function update_firebase() {
  db.collection("devices").doc("111111").updated("shouldUpdateLocation", false).then(() => {
    console.log("successfully updated!\n");
  });
}

function get_firebase() {
  db.collection("devices").get().then((querySnapshot) => {
    querySnapshot.forEach((doc) => {
      console.log(doc.id, " => ", doc.data());
    });
  });
}


