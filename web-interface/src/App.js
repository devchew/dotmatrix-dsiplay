import { Router } from "preact-router";

import "./App.css";
import { Header } from "./Header";
import { Home } from "./Home";
import { WiFi } from "./WiFi/WiFi";
import { Display } from "./Display/Display";

const App = () => (
  <main class="app">
    <Header />
    <Router>
      <Home path="/" />
      <WiFi path="/wifi" />
      <Display path="/display" />
    </Router>
  </main>
);

export default App;
