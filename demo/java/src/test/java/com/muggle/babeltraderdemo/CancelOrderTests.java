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
public class CancelOrderTests {
    private final Logger logger = LoggerFactory.getLogger(this.getClass());

    @Autowired
    private Trader trader;

    private String ctp_url = "ws://127.0.0.1:8001/ws";
    private String xtp_url = "ws://127.0.0.1:8002/ws";

    private void TestCancelOrder_CTP() throws Exception {
        trader.Connect(ctp_url);
        trader.CancelOrder(
                "weidaizi",
                "104027_20181112_      244440",
                "ctp",
                "SHFE",
                "future",
                "rb",
                "1901",
                "1901"

        );
    }

    private void TestCancelOrder_XTP() throws Exception {
        trader.Connect(xtp_url);
        trader.CancelOrder(
                "weidaizi",
                "15033731_20181112_36567580332262377",
                "ctp",
                "SHFE",
                "future",
                "rb",
                "1901",
                "1901"

        );
    }

    @Test
    public void TestCancelOrder() throws Exception {
        // NOTE: change input arguments in the tests
        TestCancelOrder_CTP();
//        TestCancelOrder_XTP();

        trader.Run();
    }
}
