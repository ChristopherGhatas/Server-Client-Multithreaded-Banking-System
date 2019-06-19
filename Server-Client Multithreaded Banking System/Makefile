
All: bankingServer bankingClient

bankingServer: bankingServer.c 
	gcc -o bankingServer bankingServer.c -pthread
	
bankingClient: bankingClient.c 
	gcc -o bankingClient bankingClient.c -pthread
	
clean:
	rm bankingServer bankingClient
