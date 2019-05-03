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
	style: 'mapbox://styles/mapbox/light-v10',
	// initial position in [long, lat] format
	center: [-77.034084142948, 38.909671288923],
	// initial zoom
	zoom: 13
}
var map = new mapboxgl.Map(MAP_STYLE);

//Click on the map - centers on location


/* PARAMETERS */
UPDATE_INTERVAL = 1000; // milliseconds


/* Global Data Structures */
bike_markers = {}




/* Initialization */
$('document').ready(function(){
	console.log("Initializing App!");
	init_map();


	console.log("Initialization Complete!");
});


// Initialize Map
function init_map() {
	// TODO

	get_initial_data();

	setTimeout(begin_update_loop, UPDATE_INTERVAL);
}


function get_initial_data() {
	db.collection("devices").get().then(populate_map);
}

function populate_map(data) {
	data.forEach((doc) => {
		// console.log(doc.id, " => ", doc.data());
		var el = document.createElement('div');

		el.className = 'marker';

		// Add new marker
		bike_markers[doc.id] = new mapboxgl.Marker(el)
			.setLngLat([doc.data().lastKnownLongitude, doc.data().lastKnownLatitude])
			.setPopup(new mapboxgl.Popup({offset: 30}) // add popups
            .setHTML('<h3>' + doc.data().deviceName + '</h3><p><h4>' + "ID: " + doc.data().deviceID + '<p>' + "Battery: " + '</h4></p>'))
			
			.addTo(map);

		// Clear update location flag
		db.collection("devices").doc(doc.id).update("shouldUpdateLocation", false);
	});

	console.log("Map populated");
}

// Click - center 
marker.addEventListener("click", function (e){
  map.flyTo(this.getLatLng());
});


function begin_update_loop() {
	console.log("Beginning update loop");

	setInterval(update_data, UPDATE_INTERVAL);
}

function update_data() {
	console.log("Periodic Update");

	// Query only entries with update location flag set
	db.collection("devices").where("shouldUpdateLocation", "==", true).get().then((data) => {
    	data.forEach((doc) => {
      	console.log(doc.id, " => ", doc.data());

      	// Update marker location
      	bike_markers[doc.id].setLngLat([doc.data().lastKnownLongitude, doc.data().lastKnownLatitude]);

      	// Clear update location flag
			db.collection("devices").doc(doc.id).update("shouldUpdateLocation", false);
    	});
  	});
}




/* UNUSED */

function update_firebase() {
  db.collection("devices").doc("111111").updated("shouldUpdateLocation", false).then(() => {
    console.log("successfully updated!\n");
  });
}

function get_firebase_conditional() {

}

function get_firebase() {
  db.collection("devices").get().then((querySnapshot) => {
    querySnapshot.forEach((doc) => {
      console.log(doc.id, " => ", doc.data());
    });
  });


// Add the data to your map as a layer
// map.addSource('bikes', { type: 'geojson', data: stores });
// map.addLayer({
//    "id": "locations",
//    "type": "symbol",
//    "source": "bikes",
//    "layout": {
//       "icon-image": "restaurant-15",
//       "icon-allow-overlap": true,
//    }
// });
}    

function update_location() {
//stores.features[0].geometry.coordinates[0] += 0.01;
//map.getSource('bikes').setData(stores);

requestAnimationFrame(animateMarker);
}






function animateMarker() {
var prev_loc = marker.getLngLat()
marker.setLngLat([prev_loc.lng + 0.005, 38.909671288923]);



// Ensure it's added to the map. This is safe to call if it's already added.

}

// This adds the data to the map
map.on('load', function (e) {


});