import './Header.css';
import { Link } from "preact-router";

export const Header = () => (
	<header>
		<nav class="menu">
			<Link activeClassName="menu__link--active" class="menu__link winUp" href="/">Home</Link>
			<Link activeClass="menu__link--active" class="menu__link winUp" href="/wifi">WiFi</Link>
		</nav>
	</header>
);