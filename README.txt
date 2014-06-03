Name:Hongzhi Li
USCID:1322106452
DONE:ALL required
FILES:
    auctionserver.c:Handle all the login procedure and authentication.
		    Collect sellers' item and broadcast them to bidder.
		    Maximize the profit and sell bidding request to all sellers and bidders
    bidder.c: login request, propose bidding requests.      
    seller.c: login request, send all the items needed to bid
    common.h: some common functions all the files need to include
REUSED:No
FAIL CONDITION:1. all records in the text files should be line by line as the sample i 
		provide. IF all entries are located in one line and have no space to
                seperate the Name password and account#,the system can not tell them
                apart and fail.
	       2. Put all the files I submitted in one folder, otherwise they cannot 
                be found and fail.

	       
How to run:
1.start the server by typing "./auctionserver", then "./bidder", and finally "./seller".

2.If you run the files more than one time, it may occur"bind:Address alreay in use".
  That's because the port address did not realse since last execution, you only need 
  to wait about one minute for the system to release the port address.

3.Please empty the broadcastList.txt file before each execution because this file 
  is written in append method.Otherwise, there will be so many duplicated entries.

4.the auction server will wait about 5s before handling the phase 2;

Explanation:
1.The requirement to complete PHASE1 is there are two bidders and two sellers have
  successfully login.Otherwise, the sever will wait.

2.After login, all the sellers will wait for 5s.Then send the item lists to the server
  So if you run "./seller" first and then wait more than 5s to run "./bidder", you
  will get a "connection refused" from the seller client window because the sever still
  waiting bidder to login and refuse to handle the PHASE2 seller request. 

