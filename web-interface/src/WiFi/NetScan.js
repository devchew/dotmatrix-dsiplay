import {useState} from 'preact/hooks';
import { wifiConfigScan } from './../api';

export const NetScan = ({onSelect}) => {
    const [scanning, setScanning] = useState(false);
    const [ssids, setSsids] = useState([]);

    const runScan = () => {
        setScanning(true);
        wifiConfigScan()
            .then((networks) => {
                setSsids(networks.ssids)
            })
            .finally(() => {
                setScanning(false);
            })
    }
  return <section>
    <h3>Scan Networks</h3>
    <button onClick={runScan} >{scanning ? 'scanning...' : 'Scan'}</button>
    {ssids.length > 0 && <section><ul>
        {ssids.map(ssid => (<li>{ssid} <button onClick={() => onSelect(ssid)}>use</button></li>))}
    </ul></section>}
  </section>;
};
