import React from "react";
import { Content } from "react-bulma-components";
import QRCode from "react-qr-code";

class QuickLink extends React.Component {
    render() {
        const state = this.props;

        return (
            <Content className="has-text-centered">
                {state.hostname !== "unknown" &&
                    <QRCode value={`http://${state.hostname}/`} />
                }
            </Content>
        );
    }
};

export default QuickLink;
