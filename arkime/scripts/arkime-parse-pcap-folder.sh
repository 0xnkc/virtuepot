#!/bin/bash
# argument parsing
TAG=""
PCAPDIR="/data/pcap"
pcapdir_set=false

help() {
  usage
  echo
  echo "  -d PCAPDIR    The directory where the .pcap files are stored."
  echo "  -t TAG        Extra tag to add to all packages. Multiple tags can be separated with a comma, e.g. '-t foo,bar'"
  echo
  exit 0
}
usage() {
  echo "Usage: $0 [ -d PCAPDIR ] [ -t TAG ]" 1>&2
}
exit_abnormal() {
  usage
  exit 1
}
while getopts ":d:t:h" options; do
  case "${options}" in
    d)
      PCAPDIR=${OPTARG}
      if [ ! -d "$PCAPDIR" ]; then
        echo "ERROR: The directory '$PCAPDIR' does not exist!" 1>&2
        exit_abnormal
      fi

      pcapdir_set=true
      ;;
    t)
      IFS=','
      TAG=($OPTARG)
      unset IFS
      ;;
    :)
      echo "Error: -${OPTARG} requires an argument."
      exit_abnormal
      ;;
    h)
      help
      ;;
    *)
      exit_abnormal
      ;;
  esac
done


# check if the default pcap directory should be used
if [ "$pcapdir_set" = false ]; then
  echo "You didn't specify a PCAP directory. The default directory '$PCAPDIR' will be used."
  read -r -p "Do you want to continue? Y/[N] " response
  case "$response" in
    [yY][eE][sS]|[yY])
      # continue after
      ;;
    *)
      echo "Aborting..."
      exit 0
      ;;
  esac
fi


# check if there are *.pcap[0-9]+ files
# these files will not be processed by capture -R
# so rename them
pcap_part_regex="(.*)\.pcap([0-9]+)"

find $PCAPDIR -type f -name '*.pcap*' | while read file; do
    if [[ $file =~ $pcap_part_regex ]]; then
        # ${BASH_REMATCH[1]} is the file name without the extension
        # ${BASH_REMATCH[2]} is the number after '.pcap'
        new_file="${BASH_REMATCH[1]}.part${BASH_REMATCH[2]}.pcap"

        # rename the file
        mv $file $new_file
    fi
done


# process the tags
tags_cmd=""
for t in ${TAG[@]}; do
  tags_cmd="$tags_cmd --tag $t"
done
# remove leading spaces
tags_cmd="${tags_cmd## }"


# the command string
CMD_STRING="$ARKIMEDIR/bin/capture --config $ARKIMEDIR/etc/config.ini --host $ARKIME_HOSTNAME --pcapdir $PCAPDIR --skip --recursive $tags_cmd"

# execute the command
eval $CMD_STRING
