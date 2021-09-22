#!/bin/bash

################################################################################
# Author: Omar Abdelmotaleb & Benjamin Singleton
# Date:	02/15/21
# Pledge:	I pledge my honor that I have abided by the Stevens Honor System.
# Description:  Moves files into junk directory. If -h is supplied, displays 
#		help message. If -l is supplied, lists files in junk directly. 
#		And if -p is supplied, purges files in help directory.
###############################################################################

readonly DIR="/home/user/.junk/"

print_help() {
	cat <<- End-of-message
	Usage: $(basename $0) [-hlp] [list of files] 
	   -h: Display help.	
	   -l: List junked files.
	   -p: Purge all files.
	   [list of files] with no other arguements to junk those files.
	End-of-message
}


#if no arguements are given, prints help message
if [ $# -eq 0 ]
	then
	print_help
	exit 1
fi


while getopts ":hlp" option; do
	#Checks to make sure only one flag has been provided
	if [ $# -gt 1 ]
		then
		echo "Error: Too many options enabled."
		print_help
		exit 1
	fi
	#checks given flag
	case "$option" in
		h) print_help
		   exit 1
		   ;;
		l) listings=$(ls -lAF "$DIR")
		   echo "$listings"
		   exit 1
		   ;;
		p) if [ "$(ls -A $DIR)" ]; then
			pDIR="${DIR}*" 
			phDIR="${DIR}.*"
		   	rm -rf $pDIR 2> /dev/null
		   	rm -rf $phDIR 2> /dev/null
		   fi
		   exit 1
		   ;;
		?) printf "Error: Unknown option '-%s'.\n" $OPTARG >&2
		   print_help
		   exit 1
		   ;;
	esac
done

#Checks for existence of .junk, if it doens't 
#exist, .junk is created
if [ ! -d "$DIR" ]; then
	mkdir $DIR
fi

#moves input into the .junk directory
#shoots warning if given input not found
index=0
for f in "$@"; do
	if [ -f "$f" ] || [ -d "$f" ]; then
		mv $f $DIR
		(( ++index ))
	else
		echo "Warning: '$f' not found."
	fi
done


exit 0


