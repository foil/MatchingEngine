//
//  main.cpp
//  MatchingEngine
//
//  Created by Meng Zhang on 11/7/19.
//  Copyright © 2019 Meng Zhang. All rights reserved.
//

#include <iostream>
#include <memory>
#include <unordered_map>
#include <set>
#include <utility>
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
    void setPrice(int p) { price = p; }
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

class Cancel: public Operation {
public:
    Cancel(string id):
    Operation(CANCEL), id(id) {}
    
    const string& getId() const { return id; }
private:
    string id;
};

class Print: public Operation {
public:
    Print(): Operation(PRINT) {}
};

class Deal {
public:
    Deal(string id1, int p1, string id2, int p2, int q):
        id1(id1), id2(id2), price1(p1), price2(p2), qty(q) {}
    friend std::ostream & operator<<(std::ostream& os, Deal const& deal) {
        os << "TRADE "
        << deal.id1 << " " << deal.price1 << " " << deal.qty << " "
        << deal.id2 << " " << deal.price2 << " " << deal.qty;
        return os;
    }
private:
    string id1, id2;
    int price1, price2, qty;
};

class OperationFactory {
public:
    static shared_ptr<Operation> createOperation(string order, unsigned long ts) {
        size_t pos = 0;
        string type = parse(order, pos);
        if (type == "BUY")
            return createBuyOrSellOp(order, pos, ts, true);
        else if (type == "SELL")
            return createBuyOrSellOp(order, pos, ts, false);
        else if (type == "MODIFY")
            return createModifyOp(order, pos, ts);
        else if (type == "CANCEL")
            return createCancelOp(order, pos);
        else if (type == "PRINT")
            return make_shared<Print>();
        
        return nullptr;
    }
private:
    static string parse(string order, size_t& pos) {
        size_t cur = order.find(" ", pos);
        string token = order.substr(pos, cur - pos);
        pos = cur + 1;
        return token;
    }

    static shared_ptr<Operation> createBuyOrSellOp(string order, size_t& pos, unsigned long ts, bool buy) {
        try {
            auto str = parse(order, pos);
            if (str != "GFD" && str != "IOC")
                return nullptr;
            bool gfd = (str == "GFD");
            int price = stoi(parse(order, pos));
            int qty = stoi(parse(order, pos));
            if (price <= 0 || qty <= 0)
                return nullptr;
            string id = parse(order, pos);
            return make_shared<Trade>(buy, gfd, price, qty, id, ts);
        } catch (...) {
            return nullptr;
        }
    }
    
    static shared_ptr<Operation> createModifyOp(string order, size_t pos, unsigned long ts) {
        try {
            string id = parse(order, pos);
            auto str = parse(order, pos);
            if (str != "BUY" && str != "SELL")
                return nullptr;
            bool buy = (str == "BUY");
            int price = stoi(parse(order, pos));
            int qty = stoi(parse(order, pos));
            if (price <= 0 || qty <= 0)
                return nullptr;
            return make_shared<Modify>(id, buy, price, qty, ts);
        } catch (...) {
            return nullptr;
        }
    }
    
    static shared_ptr<Operation> createCancelOp(string order, size_t pos) {
        string id = parse(order, pos);
        return make_shared<Cancel>(id);
    }
};

class MatchingEngine {
public:
    void execute(shared_ptr<Operation> op) {
        if (op) {
            switch (op->getType()) {
                case BUY:
                case SELL:
                    trade(static_pointer_cast<Trade>(op));
                    break;
                case MODIFY:
                    modify(static_pointer_cast<Modify>(op));
                    break;
                case CANCEL:
                    cancel(static_pointer_cast<Cancel>(op));
                    break;
                case PRINT:
                    print();
                    break;
                default:
                    break;
            }
        }
    }

private:
    struct priceTimePriority {
        bool operator() (const shared_ptr<Trade>& lhs, const shared_ptr<Trade>& rhs) const {
            if (lhs->getPrice() != rhs->getPrice()) {
                if (lhs->getType() == BUY)
                    return lhs->getPrice() > rhs->getPrice();
                else
                    return lhs->getPrice() < rhs->getPrice();
            }
            return lhs->getTimeStamp() < rhs->getTimeStamp();
        }
    };
    set<shared_ptr<Trade>, MatchingEngine::priceTimePriority> buys;
    set<shared_ptr<Trade>, MatchingEngine::priceTimePriority> sells;
    unordered_map<string, shared_ptr<Trade>> orderBook;
    
    void trade(shared_ptr<Trade> op) {
        //  0. check if same id already existed
        if (orderBook.find(op->getId()) != orderBook.end())
            return;
        
        //  1. insert into order book
        if (op->getType() == BUY) {
            buys.insert(op);
        } else if (op->getType() == SELL) {
            sells.insert(op);
        } else
            return;
        orderBook.insert(make_pair(op->getId(), op));

        //  2. trade
        doTrade();
        
        //  3. clear IOC
        if (op && !op->isGfd()) {
            if (op->getType() == BUY)
                buys.erase(op);
            else if (op->getType() == SELL)
                sells.erase(op);
            orderBook.erase(op->getId());
        }
    }
    
    void doTrade() {
        auto buyIt = buys.begin(), sellIt = sells.begin();
        while (buyIt != buys.end() && sellIt != sells.end()) {
            auto buy = *buyIt;
            auto sell = *sellIt;
            if (buy->getPrice() < sell->getPrice())
                break;
            //  deal
            cout << createDeal(buy, sell) << endl;
            
            if (buy->getQty() == 0) {
                buyIt = buys.erase(buyIt);
                orderBook.erase(buy->getId());
            }
            if (sell->getQty() == 0) {
                sellIt = sells.erase(sellIt);
                orderBook.erase(sell->getId());
            }
        }
    }
    
    static Deal createDeal(const shared_ptr<Trade>& buy, const shared_ptr<Trade>& sell) {
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
        if (buy->getTimeStamp() < sell->getTimeStamp())
            return Deal(buy->getId(), buy->getPrice(), sell->getId(), sell->getPrice(), q);
        return Deal(sell->getId(), sell->getPrice(), buy->getId(), buy->getPrice(), q);
    }
    
    shared_ptr<Trade> removeFromPriceBook(string id) {
        auto orderIt = orderBook.find(id);
        if (orderIt == orderBook.end())
            return nullptr;
        
        auto ptr = orderIt->second;
        auto it = buys.find(ptr);
        if (it == buys.end()) {
            it = sells.find(ptr);
            if (it == sells.end())
                //  should not get here
                return nullptr;
            else
                sells.erase(it);
        } else
            buys.erase(it);
        
        return ptr;
    }
    
    void modify(shared_ptr<Modify> op) {
        auto target = removeFromPriceBook(op->getId());
        if (!target)
            return;

        target->setPrice(op->getPrice());
        target->setQty(op->getQty());
        target->setTimeStamp(op->getTimeStamp());
        if (op->isBuy()) {
            target->setType(BUY);
            buys.insert(target);
        } else {
            target->setType(SELL);
            sells.insert(target);
        }
        doTrade();
    }
    
    void cancel(shared_ptr<Cancel> op) {
        auto target = removeFromPriceBook(op->getId());
        if (target)
            orderBook.erase(target->getId());
    }
    
    void print() {
        cout << "SELL:" << endl;
        int curPrice = -1, qty = 0;
        for (auto it = sells.rbegin(); it != sells.rend(); it++) {
            auto ptr = *it;
            if (ptr->getPrice() != curPrice) {
                if (curPrice != -1)
                    cout << curPrice << " " << qty << endl;
                curPrice = ptr->getPrice();
                qty = ptr->getQty();
            } else {
                qty += ptr->getQty();
            }
        }
        if (curPrice != -1)
            cout << curPrice << " " << qty << endl;
        
        cout << "BUY:" << endl;
        curPrice = -1; qty = 0;
        for (const auto & ptr: buys) {
            if (ptr->getPrice() != curPrice) {
                if (curPrice != -1)
                    cout << curPrice << " " << qty << endl;
                curPrice = ptr->getPrice();
                qty = ptr->getQty();
            } else {
                qty += ptr->getQty();
            }
        }
        if (curPrice != -1)
            cout << curPrice << " " << qty << endl;
    }
};

int main(int argc, const char * argv[]) {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    string order;
    MatchingEngine engine;
    unsigned long ts = 0;
//    while (getline(cin, order)) {
//        engine.execute(OperationFactory::createOperation(order, ts++));
//    }
//    return 0;
//}
    string orders[] = {
//        "SELL GFD 900 10 order2",
//        "BUY GFD 1000 10 order1",
//        "PRINT",

//        "BUY GFD 1000 10 order1",
//        "BUY GFD 1000 10 order2",
//        "SELL GFD 900 20 order3",
//        "PRINT",
//
//        "BUY GFD 1000 10 order1",
//        "BUY GFD 1000 10 order2",
//        "MODIFY order1 BUY 1000 20",
//        "SELL GFD 900 20 order3",
//
//        "BUY GFD 1000 10 order1",
//        "BUY GFD 1000 20 order2",
//        "BUY GFD 1001 20 order3",
//        "SELL GFD 900 20 order4",
//        "PRINT",
//
//        "BUY GFD 1000 10 ORDER1",
//        "BUY GFD 900 10 ORDER2",
//        "SELL GFD 1010 30 ORDER3",
//        "SELL GFD 1020 30 ORDER4",
//        "PRINT",
    };
    for (const auto& order: orders)
        engine.execute(OperationFactory::createOperation(order, ts++));

    return 0;
}

