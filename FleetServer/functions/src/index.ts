import * as functions from 'firebase-functions';
import * as admin from 'firebase-admin';
import * as cors from 'cors';

const corsHandler = cors({origin: true});

admin.initializeApp();

const firestore_device = admin.firestore().collection('devices');


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

        // Check If the Device Is Activated
        var deviceExistsPromise = checkIfDeviceExists(deviceID)

        deviceExistsPromise.then((result) => {
            // Device Does Exist
            if (result == true) {
                firestore_device.doc(deviceID).collection('coordinates').add({longitude: longitude, latitude: latitude, timestamp: time }).then((writeResult) => {
                    console.log("Successfully Pinged Location")
                    response.status(200).json({
                        result: `Coordinated with ID: ${writeResult.id} added.`
                    })
                }).catch((error) => {
                    console.log("Unsuccessfully Wrote");
                    response.status(400).send(JSON.stringify({
                        error: "Invalid Data Sent"
                    }))
                })
            } else {
                console.log('Device Does Not Exist')
                response.status(400).send(JSON.stringify({
                    error: "Device Does Not Exist"
                }))
            }
            

        }).catch((error) => {
            console.log("Firestore Request Catched")
            response.status(400).send(JSON.stringify({
                error: "Something Bad Happened When looking For Device"
            }))
        })

    } else {
        console.log("Someone tried to do a " + request.method + " request on Send Pulse")
        response.status(400).send(JSON.stringify({
            error: "Invalid Request Method"
        }))
    }
})

export const getDeviceStatus = functions.https.onRequest((request, response) => {
    corsHandler(request, response, () => {
        console.log("CORS")
    });
    const deviceID = request.get("deviceID")

    // Check if the Device Exists
    var deviceExistsPromise = checkIfDeviceExists(deviceID)

    deviceExistsPromise.then((result) => {
        if (result == true) {
            var coordinatePromise = firestore_device.doc(deviceID).collection("coordinates").orderBy("timestamp", "desc").get()

            coordinatePromise.then((snapshot) => {
                var counter = 0;
    
                snapshot.forEach(doc => {
                    if (counter == 0) {
                        const data = doc.data();
                        console.log(data);
                        response.status(200).send(data);
                    }
                    counter += 1;
                })
                
            }).catch((error) => {
                console.log("Couldn't Get Users Location")
                response.status(400).send(JSON.stringify({
                    error: "Couldn't Get A Location"
                }))
            })

        } else {
            console.log('Device Does Not Exist')
            response.status(400).send(JSON.stringify({
                error: "Device Does Not Exist"
            }))
        }
    })
.catch((error) => {
        console.log("Firestore Request Catched")
        response.status(400).send(JSON.stringify({
            error: "Something Bad Happened When looking For Device"
        }))
    })

})

export const activateNewDevice = functions.https.onRequest((request, response) => {
    corsHandler(request, response, () => {
        console.log("CORS")
    });

    const deviceID = request.get("deviceID")
    const deviceName = request.get("deviceName")
    const time = Date.now()

    firestore_device.doc(deviceID).set({deviceName: deviceName, deviceID: deviceID, shouldRequestUpdate: false}).then((activateResult) => {
        console.log("Successfully Activated " + deviceName + " With ID: " + deviceID)

        admin.firestore().collection('devices').doc(deviceID).collection('coordinates').add({longitude: 0, latitude: 0, timestamp: time }).then((writeResult) => {
            console.log("Successfully Wrote")
            response.json({
                result: `Activated Device with ID: ${deviceID} added.`
            })
        }).catch((error) => {
            console.log("Unsuccessfully Wrote");
            response.status(400).send(JSON.stringify({
                error: "Couldn't Activate New Device"
            }))
        })
    }).catch((error) => {
        console.log("Unsuccessfully Able To Activate New Device");
        response.status(400).send(JSON.stringify({
            error: "Couldn't Activate New Device"
        }))
    })
})

// Method To Check If A Device Exists Or Not
function checkIfDeviceExists(deviceID: string | undefined): Promise<boolean> {
    return new Promise((resolve, reject) => {

        var devicePromise = firestore_device.doc(deviceID).get()

        devicePromise.then((doc) => {
            if (!doc.exists) {
                resolve(false)
            } else {
                resolve(true)
            }
        }).catch((error) => {
            reject(error)
        })
    })
}

// Method To Check If the Device Has Ever Sent Coordinates