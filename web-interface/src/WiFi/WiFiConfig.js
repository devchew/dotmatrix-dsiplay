import { useState, useEffect } from "preact/hooks";
import { wifiConfigGet, wifiConfigSet } from './../api';

const FormStatuses = {
  waiting: 0,
  saving: 1,
  ok: 2,
  error: 3,
};

const extractVal = (e) => ((e.target || {}).value || '');

export const WiFiConfig = ({ ssid }) => {
  const [formSSID, setFormSSID] = useState(ssid);
  const [formPass, setFormPass] = useState("");
  const [formStatus, setFormStatus] = useState(FormStatuses.waiting);
  useEffect(() => {
    setFormSSID(ssid);
    setFormPass('')
  }, [ssid]);

  useEffect(() => {
    wifiConfigGet().then((config) => {
            setFormSSID(config.ssid);
            setFormPass(config.passphrase)
        })
  },[])

  const submit = (e) => {
    e.preventDefault();
    const formData = new FormData(e.target);
    setFormStatus(FormStatuses.saving);
    wifiConfigSet(formData)
      .then(() => setFormStatus(FormStatuses.ok))
      .catch(() => setFormStatus(FormStatuses.error));
  };
  return (
    <section>
      <h3>Config Networks</h3>
      {formStatus !== FormStatuses.waiting && (
        <div class="notify">
          {formStatus === FormStatuses.ok && "saved"}
          {formStatus === FormStatuses.saving && "saving"}
          {formStatus === FormStatuses.error && "saving error"}
        </div>
      )}
      <form onSubmit={submit}>
        <label>
          SSID
          <input name="ssid" value={formSSID} onInput={extractVal(setFormSSID)} />
        </label>
        <label>
          Pass Phrase
          <input name="passphrase" value={formPass} onInput={extractVal(setFormPass)} />
        </label>
        <button type="submit">Save</button>
      </form>
    </section>
  );
};
