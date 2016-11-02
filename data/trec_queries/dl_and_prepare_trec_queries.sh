#!/bin/bash    

################################################################################
#
# Script to download and extact the trec queries
# 
# [1] http://www.cim.mcgill.ca/~dudek/206/Logs/AOL-user-ct-collection/
#
################################################################################

echo "Starting downloading the queries"
wget http://trec.nist.gov/data/million.query/07/07-million-query-topics.1-10000.gz
wget http://trec.nist.gov/data/million.query/08/08.million-query-topics.10001-20000.gz
wget http://trec.nist.gov/data/million.query/09/09.mq.topics.20001-60000.gz
echo "Downloading complete, extracting"
gzip -d 07-million-query-topics.1-10000.gz
gzip -d 08.million-query-topics.10001-20000.gz
gzip -d 09.mq.topics.20001-60000.gz
cat 07-million-query-topics.1-10000  08.million-query-topics.10001-20000  09.mq.topics.20001-60000 >> trec_queries.txt
rm 07-million-query-topics.1-10000  08.million-query-topics.10001-20000  09.mq.topics.20001-60000
echo "Removing line numbers"
sed -i 's/[0-9]*://g' trec_queries.txt
echo "Finished"