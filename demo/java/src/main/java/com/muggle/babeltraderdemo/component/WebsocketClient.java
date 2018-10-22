package com.muggle.babeltraderdemo.component;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

import javax.websocket.*;
import java.io.IOException;
import java.net.URI;

@ClientEndpoint
@Component
public class WebsocketClient {
    private final Logger logger = LoggerFactory.getLogger(this.getClass());

    private Session session;

    public void connect2Url(String url) throws Exception {
        WebSocketContainer container = ContainerProvider.getWebSocketContainer();
        this.session = container.connectToServer(this.getClass(), new URI(url));
    }

    @OnOpen
    public void onOpen(Session session) {
        this.session = session;
        logger.info("Client on open");
    }

    @OnError
    public void onError(Throwable throwable) throws IOException {
        this.session.close();
        logger.error("client on error");
        throwable.printStackTrace();
    }

    @OnClose
    public void onClose() throws IOException {
        this.session.close();
        logger.info("client on close");
    }

    @OnMessage
    public void onMessage(String message) throws Exception {
        logger.info("client on message: " + message);
    }

    public void sendMessage(String message) throws IOException {
        session.getBasicRemote().sendText(message);
    }
}
