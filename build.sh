# prepare the parsed data
../prepare-data/ptb -c -n 10 -x 9 /export/common/data/corpora/LDC/LDC99T42/treebank_3/parsed/mrg/wsj/00/*mrg \
    | head -100 \
    | perl -pe 's/^<s.*?> |<\/s>//g' \
    | java -jar /home/hltcoe/ccallison/bin/berkeleyParser/berkeleyParser.jar -gr /home/hltcoe/ccallison/bin/berkeleyParser/eng_sm6.gr -confidence \
    | perl -pe 's/\t/\n/; print "1\t" . $val++ . $/;' \
    > top100

# prepare the gold data
(echo 100;
    ../prepare-data/ptb -g -n 10 -x 9 /export/common/data/corpora/LDC/LDC99T42/treebank_3/parsed/mrg/wsj/00/*mrg \
        | head -101 | tail -100 \
        | perl -pe 'BEGIN { $val = 0 } s/^.*?\t/$val\t/; $val++') \
        > top100.gold
    
