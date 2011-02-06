#!/bin/sh


rev_number=$(git log --pretty=format:'' | wc -l)
rev_number=$(printf "%d" $rev_number)

ofile="gracing-$rev_number.tar.bz2"

if [ -f $ofile ] ; then
  echo "Output file '$ofile' already exists! Doing nothing"
  exit
fi

echo "Archiving repository with rev_number: $rev_number (file: $ofile)"

git archive master | bzip2  > $ofile

