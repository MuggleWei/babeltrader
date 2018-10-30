package com.muggle.babeltraderdemo;

import com.muggle.babeltraderdemo.component.WebsocketClient;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

@RunWith(SpringRunner.class)
@SpringBootTest
public class QuoteTests {
    private final Logger logger = LoggerFactory.getLogger(this.getClass());

    @Autowired
    private WebsocketClient client;

    private String url = "ws://127.0.0.1:6002/ws";

    private void Connect() {
        try {
            client.connect2Url(url);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.fail();
        }
    }

    @Test
    public void RecQuoteTest() {
        Connect();

        while (true) {
            try {
                String msg = client.getMessage();
                logger.info(msg);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}
