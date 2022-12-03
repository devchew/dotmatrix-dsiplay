const apiBase = process.env.API;

const createRequestUri = (uri) => `${apiBase}${uri}`;
const after = (r) => r.json();

export const wifiConfigGet = () =>
  fetch(createRequestUri("/wifi/get")).then(after);

export const wifiConfigScan = () =>
  fetch(createRequestUri("/wifi/scan")).then(after);

export const wifiConfigSet = (formData) =>
  fetch(
    createRequestUri("/wifi/set"), {
      body: formData,
      method: "post",
    })
