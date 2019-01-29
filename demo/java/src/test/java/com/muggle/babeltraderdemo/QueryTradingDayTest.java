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
public class QueryTradingDayTest {
    private final Logger logger = LoggerFactory.getLogger(this.getClass());

    @Autowired
    private Trader trader;

    private String ctp_url = "ws://127.0.0.1:8001/ws";

    @Test
    public void TestQueryTradingDay_CTP() throws Exception {
        trader.Connect(ctp_url);
        trader.QueryTradingDay(
                "1",
                "ctp"
        );

        trader.Run();
    }
}
