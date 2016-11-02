#!/bin/bash    

################################################################################
#
# Script to download, extract and process the AOL-User Queries [1] such that
# they can be used as queries for the dpt
# 
# [1] http://www.cim.mcgill.ca/~dudek/206/Logs/AOL-user-ct-collection/
#
################################################################################

echo "Starting downloading the queries"
wget http://www.cim.mcgill.ca/~dudek/206/Logs/AOL-user-ct-collection/aol-data.tar.gz
echo "Downloading complete, extracting main archive"
tar -xzvf aol-data.tar.gz
echo "Extracting complete, deleting archive and entering 'AOL-user-ct-collection'"
rm aol-data.tar.gz
cd AOL-user-ct-collection
echo "Entered 'AOL-user-ct-collection', creating text file"
find . -name "user-ct-test-collection-*.gz" | xargs cat | gunzip >> aol-user-ct-test-collection.txt
echo "Created text file, deleting archives"
rm aol-data.tar.gz
echo "finished"