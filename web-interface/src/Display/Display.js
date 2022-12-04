import { useEffect, useState } from "preact/hooks";
import { displayConfigGet, displayConfigSet } from "../api";

const FormStatuses = {
  waiting: 0,
  saving: 1,
  ok: 2,
  error: 3,
};

const pad = (val) => String(val).padStart(2, '0');

const stringDateToEpochTime = (date) =>
  Math.round(new Date(date).getTime() / 1000);
const epoxToString = (dateepoch) =>{
    const date = new Date(dateepoch * 1000);
    return `${date.getFullYear()}-${pad(date.getMonth() +1)}-${pad(date.getDate())}T${pad(date.getHours())}:${pad(date.getMinutes())}`;
}

export const Display = () => {
  const [formStatus, setFormStatus] = useState(FormStatuses.waiting);
  const [mode, setMode] = useState(0);
  const [until, setUntil] = useState(0);

  const changeMode = (e) => setMode(e.target.value);
  const untilChange = (e) => setUntil(stringDateToEpochTime(e.target.value));

  const submit = (e) => {
    e.preventDefault();
    const formData = new FormData();
    formData.set("until", until);
    formData.set("mode", mode);
    setFormStatus(FormStatuses.saving);
    displayConfigSet(formData)
      .then(() => setFormStatus(FormStatuses.ok))
      .catch(() => setFormStatus(FormStatuses.error));
  };

  useEffect(() => {
    displayConfigGet().then((config) => {
      setMode(config.mode);
      setUntil(config.until);
    });
  }, []);
  return (
    <section>
      <h1>Display</h1>
      <section>
        <h3>Mode</h3>
        {formStatus !== FormStatuses.waiting && (
          <div class="notify">
            {formStatus === FormStatuses.ok && "saved"}
            {formStatus === FormStatuses.saving && "saving"}
            {formStatus === FormStatuses.error && "saving error"}
          </div>
        )}
        <form onSubmit={submit}>
          <label>
            Mode
            <select name="mode" onChange={changeMode} value={mode}>
              <option value="0">Countdown</option>
              <option value="1">Clock</option>
            </select>
          </label>
          <br />
          <label>
            Countdown until
            <input
              name="until"
              type="datetime-local"
              value={epoxToString(until)}
              onChange={untilChange}
            />
          </label>
          <br />
          <button type="submit">Save</button>
        </form>
      </section>
    </section>
  );
};
