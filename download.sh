# Download the data for the Ledger
BASE='http://docs.google.com/a/localdata.com/spreadsheet/ccc?output=csv&key=0Aty9maMoYSDFdDFNQk93TjUzc0haeGVjWGtORVAzYlE&gid=';
 
for SHEET in {0..20}
do
  PATH=$BASE$SHEET
  /usr/local/bin/wget -O test$SHEET.csv "$PATH"
done
