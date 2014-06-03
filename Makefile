CC            =  gcc
OPTIONS       = -lsocket -lnsl -lresolv -o
all  :  auctionserver bidder seller
auctionserver: auctionserver.c common.h
	$(CC) $(OPTIONS) auctionserver auctionserver.c 
bidder: bidder.c common.h
	$(CC) $(OPTIONS) bidder bidder.c 
seller: seller.c  common.h
	$(CC) $(OPTIONS) seller seller.c