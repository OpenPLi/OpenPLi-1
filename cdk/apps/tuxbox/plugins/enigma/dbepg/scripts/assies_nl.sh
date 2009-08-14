#/bin/sh
configdir=/var/tuxbox/config/dbepg
configfile=$configdir/assies_nl.cfg
epg=/var/lib/sqlite/epg.db
convert=/var/bin/db_epg
genrefile=$configdir/genre.dat
datadir=/media/cf

#Some default values, to make sure they are set..
hours=72
timeOffset=0
numofdays=0


OIFS=$IFS
IFS='='
while read opt val
do
	case "$opt" in
	"timeOffset") 
		timeOffset=$val ;
		;;
	"numOfDays") 
		numofdays=$val 
		hours=$((24*$val)) ;
		;;
	"mapfile")
		mapfile=$configdir/$val ;
		;;
	"genreMap")
		genrefile=$configdir/$val ;
		;;
	"epgLocation")
		epg=$val ;
		datadir=`dirname $val` ;
		;;
	esac;
done < $configfile
IFS=$OIFS

echo Using a timeOffset of $timeOffset seconds.
echo Retrieving $hours hours of data for each channel
touch $mapfile

if [ -f $epg ] && [ -f $convert ] && [ -f $mapfile ]; then
	count=$numofdays
	echo "#count:$count"
	current=0
	while [ $current -lt $count ]; do
		tmp=`awk "BEGIN { format = \"%Y%m%d\"; print strftime(format, systime()+$((3600*24*$current))) }"`
		echo $tmp
		datafile=tv-$tmp.xmltv
                datafilegz=tv-$tmp.xmltv.gz
		wget  "http://xmltv.assies.info/$datafilegz" -O $datadir/$datafilegz
	#download data to $data
		echo "#cur:$current"
		current=$(($current+1))
		if [ -f $datadir/$datafilegz ]; then
                        gunzip $datadir/$datafilegz
			echo "$convert -t xmltv -d $epg -f $datadir/$datafile -o $timeOffset -h $hours -m $mapfile -g $genrefile -e -c 1"
			$convert -t xmltv -d $epg -f $datadir/$datafile -o $timeOffset -h $hours -m $mapfile -g $genrefile -e -c 1
       		        rm $datadir/$datafile
		else
			echo "Data $datadir/$datafilegz not found"
		fi
	done
else
	if [ ! -f $epg ]; then echo "Database not found" ; fi;
	if [ ! -f $convert ]; then echo "Converter $convert not found" ; fi;
	if [ ! -f $mapfile ]; then echo "Mapfile $mapfile not found" ; fi;
fi;

