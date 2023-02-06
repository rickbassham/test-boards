import React from "react";
import { Button, Level, Message } from "react-bulma-components";

class Home extends React.Component {
    render() {
        const state = this.props;

        return (
            <div>
                <Message>
                    <Message.Header>
                        <p>Status</p>
                    </Message.Header>
                    <Message.Body className="has-text-centered">
                        <p id="status" className="is-size-1">{state.status.toUpperCase()}</p>
                    </Message.Body>
                </Message>
                <Level className="buttons">
                    <Button className="is-large level-item is-flex-grow-1" disabled={state.status.toUpperCase() === "OPEN" || state.status.toUpperCase() === "UNKNOWN"}>
                        Open
                    </Button>
                    <Button className="is-large level-item is-flex-grow-1" disabled={state.status.toUpperCase() === "CLOSED" || state.status.toUpperCase() === "UNKNOWN"}>
                        Close
                    </Button>
                    <Button className="is-large level-item is-flex-grow-1 is-primary" disabled={state.status.toUpperCase() === "CLOSED" || state.status.toUpperCase() === "OPEN" || state.status.toUpperCase() === "UNKNOWN"}>
                        Abort
                    </Button>
                </Level>
            </div>
        );
    }
};

export default Home;
