import * as functions from 'firebase-functions';
import * as admin from 'firebase-admin';
import * as cors from 'cors';

const corsHandler = cors({origin: true});


admin.initializeApp();


// Start writing Firebase Functions
// https://firebase.google.com/docs/functions/typescript


export const sendPulse = functions.https.onRequest((request, response) => {
    corsHandler(request, response, () => {
        console.log("CORS")
    });
    if (request.method == "POST") {
        // Grabs Longitude and Latitude Fields From Body
        let deviceID = request.get("deviceID")
        let longitude = parseFloat(JSON.stringify(request.body.longitude));
        let latitude = parseFloat(JSON.stringify(request.body.latitude));
        let time = Date.now();

        admin.firestore().collection('devices').doc(deviceID).collection('coordinates').add({longitude: longitude, latitude: latitude, timestamp: time }).then((writeResult) => {
            console.log("Successfully Wrote")
            response.set('Access-Control-Allow-Origin', '*').json({
                result: `Message with ID: ${writeResult.id} added.`
            })
        }).catch((error) => {
            console.log("Unsuccessfully Wrote");
            response.set('Access-Control-Allow-Origin', '*').status(400).send(JSON.stringify({
                error: "Invalid Data Sent"
            }))
        })

    } else {
        console.log("Someone tried to do a " + request.method + " request on Send Pulse")
        response.set('Access-Control-Allow-Origin', '*').status(400).send(JSON.stringify({
            error: "Invalid Request Method"
        }))
    }
})

export const getDeviceStatus = functions.https.onRequest((request, response) => {
    corsHandler(request, response, () => {
        console.log("CORS")
    });
    const deviceID = request.get("deviceID")


        // Get Devices Location
        admin.firestore().collection('devices').doc(deviceID).collection("coordinates").orderBy("timestamp", "desc").get().then((snapshot) => {
            var counter = 0;

            snapshot.forEach(doc => {
                if (counter == 0) {
                    const data = doc.data();
                    console.log(data);
                    response.set('Access-Control-Allow-Origin', '*').status(200).send(data);
                }
                counter += 1;
            })
            
        }).catch((error) => {
            console.log("Couldn't Get Users Location")
            response.set('Access-Control-Allow-Origin', '*').status(400).send(JSON.stringify({
                error: "Device Does Not Exist"
            }))
        })

})