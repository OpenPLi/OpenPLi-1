#/bin/sh
configdir=/var/tuxbox/config/dbepg

convert=/var/bin/db_epg
configfile=$configdir/rt_uk.cfg

# Some default values, to make sure they are set..
epg=/var/lib/sqlite/epg.db
hours=72
timeOffset=0
mapfile=$configdir/rt_uk_channels.dat
genrefile=$configdir/genre.dat

OIFS="$IFS"
IFS='='
while read opt val ; do
	case "$opt" in
	"timeOffset") 
		timeOffset=$val ;
		;;
	"numOfDays") 
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
		;;
	esac;
done < $configfile

IFS="$OIFS"

echo "Retrievel $hours hours of channel data, using a timeOffset of $timeOffset seconds."
touch $mapfile

[ ! -f $epg ] && echo "Database not found" && exit 10
[ ! -f $convert ] && echo "Converter $convert not found" && exit 11
[ ! -f $mapfile ] && echo "Mapfile $mapfile not found" && exit 12

IFS='='

#Feedback for plugin
#count=`grep -v '^UNKNOWN' $mapfile | grep -v '^IGNORE' | wc -l`
#Send '#count:" with the number of files
#Send '#cur:" for each file
#echo "#count:$count"
#current=0

while read serviceRef chanId ; do
	if  [ $serviceRef != "UNKNOWN" ] && [ $serviceRef != "IGNORE" ]
	then
#		echo "#cur:$current"
#		current=$(($current+1))
		echo "wget  http://xmltv.radiotimes.com/xmltv/$chanId.dat -O /tmp/$chanId.dat"
		wget  "http://xmltv.radiotimes.com/xmltv/$chanId.dat" -O "/tmp/$chanId.dat"
		if [ -f "/tmp/$chanId.dat" ]; then
			echo "$convert -t uk_rt -d $epg -f /tmp/$chanId.dat -o $timeOffset -h $hours -r $serviceRef"
			$convert -t uk_rt -d "$epg" -f "/tmp/$chanId.dat" -o $timeOffset -h $hours -r $serviceRef --genre "$genrefile"
			rm "/tmp/$chanId.dat"
		fi
	fi
done < $mapfile;

IFS="$OIFS"

#get latest channels.dat for future references
echo "wget http://xmltv.radiotimes.com/xmltv/channels.dat -O $mapfile"
wget http://xmltv.radiotimes.com/xmltv/channels.dat -O $configdir/channels.dat

exit 0
