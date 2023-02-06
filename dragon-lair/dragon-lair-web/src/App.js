import Header from "./components/Header";
import "./style.sass";

import React, { useEffect, useState } from "react";
import { Container, Section } from "react-bulma-components";
import {
  BrowserRouter as Router, Route, Routes
} from "react-router-dom";
import Footer from "./components/Footer";
import Home from "./components/Home";
import QuickLink from "./components/QuickLink";
import Update from "./components/Update";

const defaultStatus = {
  "status": "unknown",
  "serialNumber": "unknown",
  "macAddress": "unknown",
  "version": "unknown",
  "ipAddress": "unknown",
  "hostname": "unknown"
};

function App() {
  const [status, setStatus] = useState(defaultStatus);

  useEffect(() => {
    setInterval(() => {
      const interval = fetch("/status")
        .then(res => res.json())
        .then(data => setStatus(data))
        .catch(err => setStatus(defaultStatus));

      return () => clearInterval(interval);
    }, 1000);
  }, []);

  return (
    <Router>
      <Header />
      <Section>
        <Container>
          <Routes>
            <Route path="/update" element={<Update {...status} />} />
            <Route path="/quicklink" element={<QuickLink {...status} />} />
            <Route path="/" element={<Home {...status} />} />
          </Routes>
        </Container>
      </Section>
      <Footer {...status} />
    </Router>
  );
}

export default App;
