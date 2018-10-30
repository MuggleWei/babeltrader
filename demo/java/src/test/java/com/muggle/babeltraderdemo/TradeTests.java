package com.muggle.babeltraderdemo;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.muggle.babeltraderdemo.component.WebsocketClient;
import com.muggle.babeltraderdemo.model.InsertOrder;
import com.muggle.babeltraderdemo.model.Order;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

import java.io.IOException;
import java.text.SimpleDateFormat;
import java.time.format.DateTimeFormatter;
import java.util.Calendar;
import java.util.Date;

@RunWith(SpringRunner.class)
@SpringBootTest
public class TradeTests {
    private final Logger logger = LoggerFactory.getLogger(this.getClass());

    @Autowired
    private WebsocketClient client;

    private String url = "ws://127.0.0.1:8001/ws";

    private void Connect() {
        try {
            client.connect2Url(url);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.fail();
        }
    }

    private void insertOrder(
            String userId,
            String market,
            String exchange,
            String type,
            String symbol,
            String contract,
            String contractId,
            String orderType,
            String orderFlag1,
            String dir,
            Double price,
            Integer amount,
            Double totalPrice,
            Long ts
    ) {
        SimpleDateFormat formatter = new SimpleDateFormat("yyyyMMdd-hhmmss");
        String dateString = formatter.format(new Date(ts));

        Order order = new Order();
        order.setUser_id(userId);
        order.setOrder_id(userId + "-" + dateString);
        order.setOutside_id("");
        order.setClient_order_id(userId + "-" + dateString);
        order.setMarket(market);
        order.setExchange(exchange);
        order.setType(type);
        order.setSymbol(symbol);
        order.setContract(contract);
        order.setContract_id(contractId);
        order.setOrder_type(orderType);
        order.setOrder_flag1(orderFlag1);
        order.setDir(dir);
        order.setPrice(price);
        order.setAmount(amount);
        order.setTotal_price(totalPrice);
        order.setTs(ts);

        InsertOrder insertOrderMsg = new InsertOrder();
        insertOrderMsg.setMsg("insert_order");
        insertOrderMsg.setData(order);

        ObjectMapper objectMapper = new ObjectMapper();
        try {
            String s = objectMapper.writeValueAsString(insertOrderMsg);
            client.sendMessage(s);
            logger.info("发送报单: {}", s);
        } catch (JsonProcessingException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Test
    public void InsertOrderTest() {
        Connect();

        long ts = System.currentTimeMillis();
        insertOrder(
                "weidaizi",
                "ctp",
                "SHFE",
                "future",
                "rb",
                "1901",
                "1901",
                "limit",
                "speculation",
                "open_long",
                4300.0,
                2,
                0.0,
                ts
        );

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
