package com.muggle.babeltraderdemo.component;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.muggle.babeltraderdemo.model.CommonMsg;
import com.muggle.babeltraderdemo.model.QueryMsg;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

@Component
public class Trader {
    private final Logger logger = LoggerFactory.getLogger(this.getClass());

    @Autowired
    private WebsocketClient client;

    public void Run() {
        while (true) {
            try {
                String msg = client.getMessage();
                Dispatch(msg);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    private void Dispatch(String msg) {
        logger.info(msg);
    }

    public void Connect(String addr) throws Exception {
        client.connect2Url(addr);
    }

    public void InsertOrder(
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
    ) throws IOException {
        SimpleDateFormat formatter = new SimpleDateFormat("yyyyMMdd-hhmmss");
        String dateString = formatter.format(new Date(ts));

        QueryMsg order = new QueryMsg();
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

        CommonMsg orderMsg = new CommonMsg();
        orderMsg.setMsg("insert_order");
        orderMsg.setData(order);

        ObjectMapper objectMapper = new ObjectMapper();
        String s = objectMapper.writeValueAsString(orderMsg);
        client.sendMessage(s);
        logger.info("报单: {}", s);
    }

    public void CancelOrder(
            String user_id,
            String outside_id,
            String market,
            String exchange,
            String type,
            String symbol,
            String contract,
            String contract_id
    ) throws IOException {
        QueryMsg order = new QueryMsg();
        order.setUser_id(user_id);
        order.setOutside_id(outside_id);
        order.setMarket(market);
        order.setExchange(exchange);
        order.setType(type);
        order.setSymbol(symbol);
        order.setContract(contract);
        order.setContract_id(contract_id);

        CommonMsg orderMsg = new CommonMsg();
        orderMsg.setMsg("cancel_order");
        orderMsg.setData(order);

        ObjectMapper objectMapper = new ObjectMapper();
        String s = objectMapper.writeValueAsString(orderMsg);
        client.sendMessage(s);
        logger.info("撤单: {}", s);
    }

    public void QueryOrder(
            String qry_id,
            String user_id,
            String outside_id,
            String market,
            String exchange,
            String type,
            String symbol,
            String contract,
            String contract_id
    ) throws IOException {
        QueryMsg order = new QueryMsg();
        order.setQry_id(qry_id);
        order.setUser_id(user_id);
        order.setOutside_id(outside_id);
        order.setMarket(market);
        order.setExchange(exchange);
        order.setType(type);
        order.setSymbol(symbol);
        order.setContract(contract);
        order.setContract_id(contract_id);

        CommonMsg orderMsg = new CommonMsg();
        orderMsg.setMsg("query_order");
        orderMsg.setData(order);

        ObjectMapper objectMapper = new ObjectMapper();
        String s = objectMapper.writeValueAsString(orderMsg);
        client.sendMessage(s);
        logger.info("查询订单: {}", s);
    }

    public void QueryTrade(
            String qry_id,
            String user_id,
            String trade_id,
            String market,
            String exchange,
            String type,
            String symbol,
            String contract,
            String contract_id
    ) throws IOException {
        QueryMsg order = new QueryMsg();
        order.setQry_id(qry_id);
        order.setUser_id(user_id);
        order.setTrade_id(trade_id);
        order.setMarket(market);
        order.setExchange(exchange);
        order.setType(type);
        order.setSymbol(symbol);
        order.setContract(contract);
        order.setContract_id(contract_id);

        CommonMsg orderMsg = new CommonMsg();
        orderMsg.setMsg("query_trade");
        orderMsg.setData(order);

        ObjectMapper objectMapper = new ObjectMapper();
        String s = objectMapper.writeValueAsString(orderMsg);
        client.sendMessage(s);
        logger.info("查询成交: {}", s);
    }

    public void QueryPosition(
            String qry_id,
            String user_id,
            String market,
            String exchange,
            String type,
            String symbol,
            String contract,
            String contract_id
    ) throws IOException {
        QueryMsg order = new QueryMsg();
        order.setQry_id(qry_id);
        order.setUser_id(user_id);
        order.setMarket(market);
        order.setExchange(exchange);
        order.setType(type);
        order.setSymbol(symbol);
        order.setContract(contract);
        order.setContract_id(contract_id);

        CommonMsg orderMsg = new CommonMsg();
        orderMsg.setMsg("query_position");
        orderMsg.setData(order);

        ObjectMapper objectMapper = new ObjectMapper();
        String s = objectMapper.writeValueAsString(orderMsg);
        client.sendMessage(s);
        logger.info("查询持仓: {}", s);
    }

    public void QueryPositionDetail(
            String qry_id,
            String user_id,
            String market,
            String exchange,
            String type,
            String symbol,
            String contract,
            String contract_id
    ) throws IOException {
        QueryMsg order = new QueryMsg();
        order.setQry_id(qry_id);
        order.setUser_id(user_id);
        order.setMarket(market);
        order.setExchange(exchange);
        order.setType(type);
        order.setSymbol(symbol);
        order.setContract(contract);
        order.setContract_id(contract_id);

        CommonMsg orderMsg = new CommonMsg();
        orderMsg.setMsg("query_positiondetail");
        orderMsg.setData(order);

        ObjectMapper objectMapper = new ObjectMapper();
        String s = objectMapper.writeValueAsString(orderMsg);
        client.sendMessage(s);
        logger.info("查询持仓明细: {}", s);
    }

    public void QueryTradeAccount(
            String qry_id,
            String user_id,
            String market
    ) throws IOException {
        QueryMsg order = new QueryMsg();
        order.setQry_id(qry_id);
        order.setUser_id(user_id);
        order.setMarket(market);

        CommonMsg orderMsg = new CommonMsg();
        orderMsg.setMsg("query_tradeaccount");
        orderMsg.setData(order);

        ObjectMapper objectMapper = new ObjectMapper();
        String s = objectMapper.writeValueAsString(orderMsg);
        client.sendMessage(s);
        logger.info("查询交易账户: {}", s);
    }

    public void QueryProduct(
            String qry_id,
            String user_id,
            String market,
            String exchange,
            String type,
            String symbol,
            String contract
    ) throws IOException {
        QueryMsg order = new QueryMsg();
        order.setQry_id(qry_id);
        order.setUser_id(user_id);
        order.setMarket(market);
        order.setExchange(exchange);
        order.setType(type);
        order.setSymbol(symbol);
        order.setContract(contract);

        CommonMsg orderMsg = new CommonMsg();
        orderMsg.setMsg("query_product");
        orderMsg.setData(order);

        ObjectMapper objectMapper = new ObjectMapper();
        String s = objectMapper.writeValueAsString(orderMsg);
        client.sendMessage(s);
        logger.info("查询产品信息: {}", s);
    }

    public void QueryTradingDay(
            String qry_id,
            String market
    ) throws IOException {
        QueryMsg qry = new QueryMsg();
        qry.setQry_id(qry_id);
        qry.setMarket(market);

        CommonMsg commonMsg = new CommonMsg();
        commonMsg.setMsg("query_tradingday");
        commonMsg.setData(qry);

        ObjectMapper objectMapper = new ObjectMapper();
        String s = objectMapper.writeValueAsString(commonMsg);
        client.sendMessage(s);
        logger.info("查询交易日信息: {}", s);
    }
}
