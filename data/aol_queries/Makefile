all: aol_data

aol_data: clean_aol_queries.o download_queries
	g++ clean_aol_queries.o -o clean_aol_queries
	./clean_aol_queries AOL-user-ct-collection/aol-user-ct-test-collection.txt

clean_aol_queries.o: clean_aol_queries.cpp
	g++ -c clean_aol_queries.cpp

download_queries:
	./dl_aol_queries.sh

clean:
	rm *.o
