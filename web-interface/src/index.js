import { render } from "preact";

import App from "./App";
import './reset.css';

document.addEventListener('DOMContentLoaded', () => {
    render(<App />, document.querySelector("#root"));
})
