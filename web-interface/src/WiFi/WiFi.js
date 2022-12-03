import { useState } from "preact/hooks";
import { NetScan } from "./NetScan";
import { WiFiConfig } from "./WiFiConfig";

export const WiFi = () => {
  const [selectedSSID, setSelectedSSID] = useState();

  return (
    <section>
      <h1>WiFi</h1>
      <NetScan onSelect={setSelectedSSID} />
      <WiFiConfig ssid={selectedSSID}/>
    </section>
  );
};
