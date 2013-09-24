# Download the data for the Ledger

CLASSIFICATIONS_BASE='http://docs.google.com/a/localdata.com/spreadsheet/ccc?output=csv&key=0Aty9maMoYSDFdFR0VFhKQy03cnNVVWNLR2xrdUQ3NGc&gid=';
GRANTS_BASE='http://docs.google.com/a/localdata.com/spreadsheet/ccc?output=csv&key=0Aty9maMoYSDFdDFNQk93TjUzc0haeGVjWGtORVAzYlE&gid=';

WGET=`which wget`

DESTDIR=/tmp/migrate_gnl

mkdir -p $DESTDIR

$WGET -O $DESTDIR/orgs_masterlist_NTEE.csv "${CLASSIFICATIONS_BASE}4"

$WGET -O $DESTDIR/classify_nonprofits_detail.csv "${CLASSIFICATIONS_BASE}1"

$WGET -O $DESTDIR/classify_grants.csv "${CLASSIFICATIONS_BASE}0"

for SHEET in 0 1 2 3 5 6 7 9 10 12 13 14 15 17 19 20 21 22 23 24 25
do
  URL=$GRANTS_BASE$SHEET
  $WGET -O $DESTDIR/grants$SHEET.csv "$URL"
  echo >> $DESTDIR/grants$SHEET.csv # ensure all sheets have a newline at end
done

# concatenate
cat $DESTDIR/grants*.csv | grep -v 'granter,granter_program,grantee,grantee_city,grantee_state,impact_area,impact_neighborhood,amount,year,length_years,data_source,notes,project_tags' > $DESTDIR/allgrants-raw.csv

# add empty id field
sed 's/^/,/' $DESTDIR/allgrants-raw.csv > $DESTDIR/allgrants-raw-emptyfirst.csv

# add autoincrement to empty id field
awk -F, '$1=NR' OFS=, $DESTDIR/allgrants-raw-emptyfirst.csv > $DESTDIR/allgrants-raw-incremented.csv

# add header row
echo 'id,granter,granter_program,grantee,grantee_city,grantee_state,impact_area,impact_neighborhood,amount,year,length_years,data_source,notes,project_tags' > $DESTDIR/allgrants.csv

cat $DESTDIR/allgrants-raw-incremented.csv >> $DESTDIR/allgrants.csv

