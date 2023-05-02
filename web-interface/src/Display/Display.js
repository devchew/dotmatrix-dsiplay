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

const offsets = [
  {
      "label": "-12:00",
      "offset": -43200
  },
  {
      "label": "-11:00",
      "offset": -39600
  },
  {
      "label": "-10:00",
      "offset": -36000
  },
  {
      "label": "-09:00",
      "offset": -32400
  },
  {
      "label": "-08:00",
      "offset": -28800
  },
  {
      "label": "-07:00",
      "offset": -25200
  },
  {
      "label": "-06:00",
      "offset": -21600
  },
  {
      "label": "-05:00",
      "offset": -18000
  },
  {
      "label": "-04:30",
      "offset": -16200
  },
  {
      "label": "-04:00",
      "offset": -14400
  },
  {
      "label": "-03:00",
      "offset": -10800
  },
  {
      "label": "-03:30",
      "offset": -12600
  },
  {
      "label": "-02:00",
      "offset": -7200
  },
  {
      "label": "-01:00",
      "offset": -3600
  },
  {
      "label": "UTC",
      "offset": 0
  },
  {
      "label": "+01:00",
      "offset": 3600
  },
  {
      "label": "+02:00",
      "offset": 7200
  },
  {
      "label": "+03:00",
      "offset": 10800
  },
  {
      "label": "+03:30",
      "offset": 12600
  },
  {
      "label": "+04:00",
      "offset": 14400
  },
  {
      "label": "+04:30",
      "offset": 16200
  },
  {
      "label": "+05:00",
      "offset": 18000
  },
  {
      "label": "+05:30",
      "offset": 19800
  },
  {
      "label": "+05:45",
      "offset": 20700
  },
  {
      "label": "+06:00",
      "offset": 21600
  },
  {
      "label": "+06:30",
      "offset": 23400
  },
  {
      "label": "+07:00",
      "offset": 25200
  },
  {
      "label": "+08:00",
      "offset": 28800
  },
  {
      "label": "+08:45",
      "offset": 31500
  },
  {
      "label": "+09:00",
      "offset": 32400
  },
  {
      "label": "+09:30",
      "offset": 34200
  },
  {
      "label": "+10:00",
      "offset": 36000
  },
  {
      "label": "+10:30",
      "offset": 37800
  },
  {
      "label": "+11:00",
      "offset": 39600
  },
  {
      "label": "+11:30",
      "offset": 41400
  },
  {
      "label": "+12:00",
      "offset": 43200
  },
  {
      "label": "+12:45",
      "offset": 45900
  },
  {
      "label": "+13:00",
      "offset": 46800
  },
  {
      "label": "+14:00",
      "offset": 50400
  }
]

export const Display = () => {
  const [formStatus, setFormStatus] = useState(FormStatuses.waiting);
  const [mode, setMode] = useState(0);
  const [until, setUntil] = useState(0);
  const [intensity, setIntensity] = useState(0);
  const [offset, setOffset] = useState(0);

  const changeMode = (e) => setMode(e.target.value);
  const untilChange = (e) => setUntil(stringDateToEpochTime(e.target.value));
  const intensityChange = (e) => setIntensity(e.target.value);
  const offsetChange = (e) => setOffset(e.target.value);

  const submit = (e) => {
    e.preventDefault();
    const formData = new FormData();
    formData.set("until", until);
    formData.set("mode", mode);
    formData.set("intensity", intensity);
    formData.set("offset", offset);
    setFormStatus(FormStatuses.saving);
    displayConfigSet(formData)
      .then(() => setFormStatus(FormStatuses.ok))
      .catch(() => setFormStatus(FormStatuses.error));
  };


  useEffect(() => {
    displayConfigGet().then((config) => {
      setMode(config.mode);
      setUntil(config.until);
      setIntensity(config.intensity);
      setOffset(config.offset);
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
          <label>
          Intensity
            <input
              name="intensity"
              type="range"
              min="0"
              max="15"
              value={intensity}
              onChange={intensityChange}
            />
          </label>  
          <br />
          <label>
          Timezone
            <select onChange={offsetChange}>
              {offsets.map(option => (<option value={option.offset} selected={option.offset === offset}>{option.label}</option>))}
            </select>
          </label>         
          <br />
          <button type="submit">Save</button>
        </form>
      </section>
    </section>
  );
};
