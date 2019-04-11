import React, { Component } from 'react';
import logo from './logo.svg';
import './App.css';
//import { Map, Marker, Popup, TileLayer } from 'react-leaflet';
import ReactMapGL,{Marker} from 'react-map-gl';



class App extends Component {

  state = {
    viewport: {
      latitude: 38.9916541,
      longitude: -76.93770289999999,
      zoom: 8
    }
  };

  onViewportChange = viewport => { 
    const {width, height, ...etc} = viewport
    this.setState({viewport: etc})
  } 
  
  componentDidMount() {
    fetch("https://us-central1-fleet-8b5a9.cloudfunctions.net/getDeviceStatus", {
      method: "GET",
      headers: {
        "Content-Type": "application/json",
        "deviceID": "1121"
      },
    }).then(response => response.json())
      .then(json => {
        this.setState({
        })
      })
  }

  render() {
    const position = [51.505, -0.09]
    return (
      <ReactMapGL 
      width='100%'
      height='100%'
        className="map"
        mapboxApiAccessToken="pk.eyJ1IjoiNDA4cmZsZWV0IiwiYSI6ImNqdWJmeXJqdzBkNG40NG8wMXFoZDlqYncifQ.YkRrorh-PE6HVYDtZf1nAw" 
        {...this.state.viewport}
        onViewportChange={viewport => this.onViewportChange(viewport)}>

        <Marker latitude={38.9916541} longitude={-76.93770289999999} offsetLeft={0} offsetTop={0}>
          <img src="https://i.imgur.com/MK4NUzI.png" alt="Logo" />
        </Marker>
      </ReactMapGL>
    );
  }
}

export default App;
