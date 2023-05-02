import { Router } from "preact-router";

import "./App.css";
import { Header } from "./Header";
import { WiFi } from "./WiFi/WiFi";
import { Display } from "./Display/Display";

const App = () => (
  <main class="app">
    <Header />
    <Router>
      <Display path="/" />
      <WiFi path="/wifi" />
    </Router>
  </main>
);

export default App;
