import React from "react";

import { Columns, Content, Footer as BulmaFooter, Level } from "react-bulma-components";

class Footer extends React.Component {
    render() {
        const state = this.props;

        return <BulmaFooter className="container">
            <Columns className="is-desktop">
                <Columns.Column className="has-text-centered">
                    <dl>
                        <dt>IP Address:&nbsp;</dt>
                        <dd>{state.ipAddress}</dd>
                    </dl>
                </Columns.Column>
                <Columns.Column className="has-text-centered">
                    <dl>
                        <dt>Hostname:&nbsp;</dt>
                        <dd>{state.hostname}</dd>
                    </dl>
                </Columns.Column>
                <Columns.Column className="has-text-centered">
                    <dl>
                        <dt>MAC Address:&nbsp;</dt>
                        <dd>{state.macAddress}</dd>
                    </dl>
                </Columns.Column>
                <Columns.Column className="has-text-centered">
                    <dl>
                        <dt>Serial Number:&nbsp;</dt>
                        <dd>{state.serialNumber}</dd>
                    </dl>
                </Columns.Column>
            </Columns>
            <Level>
                <Level.Item>
                    <dl className="has-text-centered">
                        <dt>Controller Version:&nbsp;</dt>
                        <dd>{state.version}</dd>
                    </dl>
                </Level.Item>
            </Level>
            <Content className="has-text-centered">
                Copyright (c) 2022 <a href="https://www.darkdragonsastro.com">Dark Dragons Astronomy</a>
            </Content>
        </BulmaFooter>;
    }
}

export default Footer;
