//
//  main.cpp
//  MatchingEngine
//
//  Created by Meng Zhang on 11/7/19.
//  Copyright © 2019 Meng Zhang. All rights reserved.
//

#include <iostream>
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

class Buy: public Operation {
public:
    Buy(bool gfd, int p, int q, string id):
    Operation(BUY), price(p), qty(q), id(id) {}
private:
    bool gfd;   //  true: GFD; false: IOC
    int price;
    int qty;
    string id;
};

class Sell: public Operation {
public:
    Sell(bool gfd, int p, int q, string id):
    Operation(BUY), price(p), qty(q), id(id) {}
private:
    bool gfd;   //  true: GFD; false: IOC
    size_t price;
    size_t qty;
    string id;
};

class Modify: public Operation {
public:
    Modify(string id, bool buy, int p, int q):
    Operation(MODIFY), id(id), buy(buy), price(p), qty(q) {}
private:
    string id;
    bool buy;   //  true: BUY; false: SELL
    size_t price;
    size_t qty;
};

class OperationFactory {
public:
    static unique_ptr<Operation> createOperation(string order) {
        size_t pos = 0;
        string type = parse(order, pos);
        if (type == "BUY")
            return createBuyOrSellOp(order, pos, true);
        else if (type == "SELL")
            return createBuyOrSellOp(order, pos, false);
        else if (type == "MODIFY")
            return createModifyOp(order, pos);
        return unique_ptr<Operation>(nullptr);
    }
private:
    static string parse(string order, size_t& pos) {
        size_t cur = order.find(" ", pos);
        string token = order.substr(pos, cur - pos);
        pos = cur + 1;
        return token;
    }

    static unique_ptr<Operation> createBuyOrSellOp(string order, size_t& pos, bool buy) {
        bool gfd = (parse(order, pos) == "GFD");
        int price = stoi(parse(order, pos));
        int qty = stoi(parse(order, pos));
        string id = parse(order, pos);
        if (buy)
            return unique_ptr<Operation>(new Buy(gfd, price, qty, id));
        return unique_ptr<Operation>(new Sell(gfd, price, qty, id));
    }
    
    static unique_ptr<Operation> createModifyOp(string order, size_t pos) {
        string id = parse(order, pos);
        bool buy = (parse(order, pos) == "BUY");
        int price = stoi(parse(order, pos));
        int qty = stoi(parse(order, pos));
        return unique_ptr<Operation>(new Modify(id, buy, price, qty));
    }
};

class MatchingEngine {
public:
    void execute(unique_ptr<Operation> op) {
        switch (op->getType()) {
            case BUY:
            case SELL:
                trade(move(op));
                break;
            case MODIFY:
                modify(move(op));
                break;
            case CANCEL:
                cancel(move(op));
                break;
            case PRINT:
                print(move(op));
                break;
            default:
                break;
        }
    }
private:
    void trade(unique_ptr<Operation> op) {
        
    }
    void modify(unique_ptr<Operation> op) {
        
    }
    void cancel(unique_ptr<Operation> op) {
        
    }
    void print(unique_ptr<Operation> op) {
        
    }
};

int main(int argc, const char * argv[]) {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    string order;
    MatchingEngine engine;
    while (true) {
        getline(cin, order);
        engine.execute(OperationFactory::createOperation(order));
    }
    return 0;
}
