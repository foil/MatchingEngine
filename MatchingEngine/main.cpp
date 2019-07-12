//
//  main.cpp
//  MatchingEngine
//
//  Created by Meng Zhang on 11/7/19.
//  Copyright © 2019 Meng Zhang. All rights reserved.
//

#include <iostream>
#include <vector>
#include <set>
using namespace std;

enum TradeType {
    BUY, SELL, CANCEL, MODIFY, PRINT
};

class Operation {
public:
    Operation(TradeType type): type(type) {}
    virtual ~Operation() {}
    const TradeType& getType() const {
        return type;
    }
protected:
    TradeType type;
};

class Trade: public Operation {
public:
    Trade(bool buy, bool gfd, int p, int q, string id, unsigned long ts):
    Operation(buy? BUY: SELL), gfd(gfd), price(p), qty(q), id(id), timestamp(ts) {}

    void setType(TradeType t) { type = t; }
    bool isGfd() const { return gfd; }
    int getPrice() const { return price; }
    int getQty() const { return qty; }
    void setQty(int q) { qty = q; };
    const string& getId() const { return id; }
    unsigned long getTimeStamp() const { return timestamp; }
    void setTimeStamp(unsigned long ts) { timestamp = ts; }
private:
    bool gfd;   //  true: GFD; false: IOC
    int price;
    int qty;
    string id;
    unsigned long timestamp;
};

class Modify: public Operation {
public:
    Modify(string id, bool buy, int p, int q, unsigned long ts):
    Operation(MODIFY), id(id), buy(buy), price(p), qty(q), timestamp(ts) {}

    bool isBuy() const { return buy; }
    int getPrice() const { return price; }
    int getQty() const { return qty; }
    void setQty(int q) { qty = q; };
    const string& getId() const { return id; }
    unsigned long getTimeStamp() const { return timestamp; }
private:
    string id;
    bool buy;   //  true: BUY; false: SELL
    int price;
    int qty;
    unsigned long timestamp;
};

class Deal {
public:
    Deal(string id1, int p1, string id2, int p2, int q):
        id1(id1), id2(id2), price1(p1), price2(p2), qty(q) {}
    const string & getId1() const { return id1; }
    const string & getId2() const { return id2; }
    int getPrice1() const { return price1; }
    int getPrice2() const { return price2; }
private:
    string id1, id2;
    int price1, price2, qty;
};

class OperationFactory {
public:
    static Operation* createOperation(string order, unsigned long ts) {
        size_t pos = 0;
        string type = parse(order, pos);
        if (type == "BUY")
            return createBuyOrSellOp(order, pos, ts, true);
        else if (type == "SELL")
            return createBuyOrSellOp(order, pos, ts, false);
        else if (type == "MODIFY")
            return createModifyOp(order, pos, ts);
        return nullptr;
    }
private:
    static string parse(string order, size_t& pos) {
        size_t cur = order.find(" ", pos);
        string token = order.substr(pos, cur - pos);
        pos = cur + 1;
        return token;
    }

    static Operation* createBuyOrSellOp(string order, size_t& pos, unsigned long ts, bool buy) {
        bool gfd = (parse(order, pos) == "GFD");
        int price = stoi(parse(order, pos));
        int qty = stoi(parse(order, pos));
        string id = parse(order, pos);
        return new Trade(buy, gfd, price, qty, id, ts);
    }
    
    static Operation* createModifyOp(string order, size_t pos, unsigned long ts) {
        string id = parse(order, pos);
        bool buy = (parse(order, pos) == "BUY");
        int price = stoi(parse(order, pos));
        int qty = stoi(parse(order, pos));
        return new Modify(id, buy, price, qty, ts);
    }
};

class MatchingEngine {
public:
    void execute(Operation* op) {
        switch (op->getType()) {
            case BUY:
            case SELL:
                trade(static_cast<Trade*>(op));
                break;
            case MODIFY:
                modify(static_cast<Modify*>(op));
                break;
            case CANCEL:
                cancel(op);
                break;
            case PRINT:
                print(op);
                break;
            default:
                break;
        }
    }

private:
    struct comp {
        bool operator() (const Trade* lhs, const Trade* rhs) const {
            if (lhs->getPrice() != rhs->getPrice()) {
                if (lhs->getType() == BUY)
                    return lhs->getPrice() > rhs->getPrice();
                else
                    return lhs->getPrice() < rhs->getPrice();
            }
            return lhs->getTimeStamp() < rhs->getTimeStamp();
        }
    };
    set<Trade*, MatchingEngine::comp> buys;
    set<Trade*, MatchingEngine::comp> sells;
    vector<Deal*> deals;

    void trade(Trade* op) {
        //  1. insert into order book
        if (op->getType() == BUY)
            buys.insert(op);
        else if (op->getType() == SELL)
            sells.insert(op);
        else
            return;
        //  2. trade
        auto buyIt = buys.begin(), sellIt = sells.begin();
        bool curOpCleaned = false;
        while (buyIt != buys.end() && sellIt != sells.end()) {
            Trade* buy = *buyIt;
            Trade* sell = *sellIt;
            if (buy->getPrice() >= sell->getPrice()) {
                //  deal
                deals.push_back(createDeal(buy, sell));
            }
            if (buy->getQty() == 0) {
                buyIt = buys.erase(buyIt);
                if (buy == op)
                    curOpCleaned = true;
                delete buy;
            } else
                buyIt++;
            if (sell->getQty() == 0) {
                sellIt = sells.erase(sellIt);
                if (sell == op)
                    curOpCleaned = true;
                delete sell;
            } else
                sellIt++;
        }
        //  3. clear IOC
        if (!curOpCleaned && !op->isGfd()) {
            if (op->getType() == BUY)
                buys.erase(op);
            else if (op->getType() == SELL)
                sells.erase(op);
            delete op;
        }
    }
    Deal* createDeal(Trade* buy, Trade* sell) {
        int q;
        if (buy->getQty() < sell->getQty()) {
            q = buy->getQty();
            sell->setQty(sell->getQty() - q);
            buy->setQty(0);
        } else {
            q = sell->getQty();
            buy->setQty(buy->getQty() - q);
            sell->setQty(0);
        }
        string id1, id2;
        int p1, p2;
        if (buy->getTimeStamp() < sell->getTimeStamp()) {
            id1 = buy->getId();
            id2 = sell->getId();
            p1 = buy->getPrice();
            p2 = sell->getPrice();
        } else {
            id1 = sell->getId();
            id2 = buy->getId();
            p1 = sell->getPrice();
            p2 = buy->getPrice();
        }

        return new Deal(id1, p1, id2, p2, q);
    }
    void modify(Modify* op) {
        //if (buys.find(op->getId())
    }
    void cancel(Operation* op) {
        
    }
    void print(Operation* op) {
        
    }
};

int main(int argc, const char * argv[]) {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    string order;
    MatchingEngine engine;
    unsigned long ts = 0;
    while (true) {
        getline(cin, order);
        engine.execute(OperationFactory::createOperation(order, ts++));
    }
    return 0;
}
