package com.muggle.babeltraderdemo;

import com.muggle.babeltraderdemo.component.Trader;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

@RunWith(SpringRunner.class)
@SpringBootTest
public class InsertOrderTests {
    private final Logger logger = LoggerFactory.getLogger(this.getClass());

    @Autowired
    private Trader trader;

    private String ctp_url = "ws://127.0.0.1:8001/ws";
    private String xtp_url = "ws://127.0.0.1:8002/ws";

    private void TestInsertOrder_CTP() throws Exception {
        trader.Connect(ctp_url);
        long ts = System.currentTimeMillis();
        trader.InsertOrder(
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
                3900.0,
                1,
                0.0,
                ts
        );
    }

    private void TestInsertOrder_XTP() throws Exception {
        trader.Connect(xtp_url);
        long ts = System.currentTimeMillis();
        trader.InsertOrder(
                "weidaizi",
                "xtp",
                "SSE",
                "spot",
                "600519",
                "",
                "",
                "limit",
                "",
                "buy",
                580.0,
                600,
                0.0,
                ts
        );
    }

    @Test
    public void TestInsertOrder() throws Exception {
        TestInsertOrder_CTP();
//        TestInsertOrder_XTP();

        trader.Run();
    }
}
