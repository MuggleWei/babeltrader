package com.muggle.babeltraderdemo.component;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import javax.annotation.PostConstruct;

@Component
public class QuoteClient {

    @Autowired
    private WebsocketClient client;

    private String url = "ws://127.0.0.1:6002/ws";

    @PostConstruct
    void postConstuct() throws Exception {
        client.connect2Url(url);
    }

}
