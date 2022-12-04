import './Header.css';
import { Link } from "preact-router";

export const Header = () => (
	<header>
		<nav>
			<Link activeClassName="active" href="/">Home</Link>
			<Link activeClassName="active" href="/wifi">WiFi</Link>
			<Link activeClassName="active" href="/display">Display</Link>
		</nav>
	</header>
);