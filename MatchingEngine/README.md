Your task is to write an exchange order matching engine. 

 

The input will come from stdin, where each line shall be comprised of several columns separated by one space.

 

The first column specifies the operation to be done. The supported operations are:

 

BUY

SELL

CANCEL

MODIFY

PRINT

 

If the first column is BUY or SELL, then this line will have five columns in total:

The second column denotes the order type, and will either be IOC or GFD.

The third column is the price you want to buy or sell, it's an integer.

The fourth column shows the quantity of that buy or sell, it's an integer. Both the price and quantity are positive numbers.

The fifth column is the Order ID, it can be arbitrary words.

 

If the first column is CANCEL, then this line has two columns in total:

The second column is the Order ID, it means the associated order needs to be deleted, it cannot be traded anymore.

If the Order ID does not exist simply do nothing.
 

If the first column is MODIFY, then this line will have five columns in total:

The second column is the Order ID, it means that specific order needs to be modified.

The third column is either BUY or SELL.

The fourth column is the new price of that order.

The fifth column is the new quantity of that order.

If the Order ID does not exist simply do nothing.

 

Note:  We cannot modify an IOC order type, as will be explained later.

 

MODIFY is a powerful operation, e.g. a BUY order can be modified to become a SELL order.

 

BUY GFD 1000 10 order1

MODIFY order1 SELL 1000 10

 

If the first column is PRINT, then there will be no following columns in this line. You're supposed to output the current price book according to our formats.

 

Output format:
SELL:
price1 qty1
price2 qty2

BUY:
price1 qty1
price2 qty2

 

The price for SELL section must be decreasing and corresponding the price for BUY section must be decreasing too.

e.g.

SELL:

1010 10

1000 20

BUY:

999 10

800 20

 

Now let's talk the order type:

The GFD order (stands for "Good For Day") will stay in the order book until it's been all traded.
the IOC order (stands for "Insert Or Cancel") means if it can't be traded immediately, it will be cancelled right away. If it's only partially traded, the non-traded part is cancelled.

 

The rule for matching is simple, if there's a price cross meaning someone is willing to buy at a price higher than or equal with the current selling price, these two orders are traded.

And you're also supposed to print out the traded information when one order is traded.

 

For example:

BUY GFD 1000 10 order1

SELL GFD 900 10 order2

 

After you process the second line, we know that these two orders can be traded, so you need to output:

TRADE order1 1000 10 order2 900 10

 

Note: The "TRADE order1 price1 qty1 order2 price2 qty2" message should be output whenever there's a trade from the matching engine, every trade must has this output, it doesn't rely on the "PRINT" operation.

 

Basically it shows two order's information, Order ID followed by its price and its traded quantity. (Real matching engine will have only one traded price, but to make things simple, we output two prices by each.)

 

The sequence for order1 and order2 is decided by who sends the order first.

 

For example:

SELL GFD 900 10 order2

BUY GFD 1000 10 order1

Then the output is:

TRADE order2 900 10 order1 1000 10

 

The overarching aim of the matching engine is to be fair. To achieve this we utilise price-time priority: Orders that are equally priced are traded out in the order they are received.

 

For example:

BUY GFD 1000 10 order1

BUY GFD 1000 10 order2

SELL GFD 900 20 order3

 

The output will be:

TRADE order1 1000 10 order3 900 10

TRADE order2 1000 10 order3 900 10

 

There's another interesting thing for MODIFY operation, "MODIFY" will lose its original place. So

BUY GFD 1000 10 order1

BUY GFD 1000 10 order2

MODIFY order1 BUY 1000 20

SELL GFD 900 20 order3

 

Because order1 is modified in the middle, now order2 is in the first place, order1 is in the second place, so the output will be:

 

TRADE order2 1000 10 order3 900 10

TRADE order1 1000 10 order3 900 10

 

We guarantee that:

Order ID will always be unique for all active orders in the matching engine, otherwise the given operation (Buy, Sell, Modify Cancel etc) should be ignored.

 

Example 1:

BUY GFD 1000 10 order1

PRINT

 

The output of above would be:

SELL:

BUY:

1000 10

 

Example 2:

BUY GFD 1000 10 order1

BUY GFD 1000 20 order2

PRINT

 

The output of above would be:

SELL:

BUY:

1000 30

 

Example 3:

BUY GFD 1000 10 order1

BUY GFD 1001 20 order2

PRINT

 

The output of above would be:

SELL:

BUY:

1001 20

1000 10

 

Example 4:

BUY GFD 1000 10 order1

SELL GFD 900 20 order2

PRINT

 

The output of above would be:

TRADE order1 1000 10 order2 900 10

SELL:

900 10

BUY:

 

Example 5:

BUY GFD 1000 10 ORDER1

BUY GFD 1010 10 ORDER2

SELL GFD 1000 15 ORDER3

TRADE ORDER2 1010 10 ORDER3 1000 10

TRADE ORDER1 1000 5 ORDER3 1000 5

 

Any input with price <= 0 or quantity <= 0 or empty order ID is invalid and should be ignored by the matching engine.

 

 

Your solution should:

 

Be entirely of your own devising and not sourced from anyone or anywhere else
Apply SOLID principles where appropriate, applicable and practical
Be clear, concise and correct  - demonstrating common sense software engineering and the application of modern C++ skills and principles.
Take into account usage in low latency scenarios
Be scalable, in short where applicable the overall performance should not degrade as the number of inputs or actions increase
 

When grading your solution all of the above criteria will be taken into account in addition to the number of tests that have passed.
