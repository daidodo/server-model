#!/bin/bash

printUsage()
{
  echo "Usage:"
  echo "    $0 -alloc FILES        replace allocator strings"
  echo "    $0 -common FILES       replace common dir strings"
}

if [ $# -lt 1 ] ; then
  printUsage
  exit 1
fi

rep_alloc()
{
  file="$1"
  echo "process $file"
  sed -r -e 's/__DZ_VECTOR\(([^\)]+)\)/std::vector<\1>/g' $file > $file.tmp.1
  sed -r -e 's/__DZ_MAP\(([^\)]+)\)/std::map<\1>/g' $file.tmp.1 > $file.tmp.2
  sed -e 's/__DZ_STRING/std::string/g' $file.tmp.2 > $file.tmp.3
  sed -e 's/__DZ_ALLOC/std::allocator/g' $file.tmp.3 > $file.tmp.4
  sed -e 's/__DZ_OSTRINGSTREAM/std::ostringstream/g' $file.tmp.4 > $file.tmp.5
  sed -e 's/__DZ_ISTRINGSTREAM/std::istringstream/g' $file.tmp.5 > $file.tmp.6
  sed -r -e 's/__DZ_LIST\(([^\)]+)\)/std::list<\1>/g' $file.tmp.6 > $file.tmp.7
  sed -r -e 's/__DZ_SET1\(([^\)]+)\)/std::set<\1>/g' $file.tmp.7 > $file.tmp.8
  sed -r -e 's/__DZ_MAP1\(([^\)]+)\)/std::map<\1>/g' $file.tmp.8 > $file.tmp.9
  sed -r -e 's/__DZ_SET\(([^\)]+)\)/std::set<\1>/g' $file.tmp.9 > $file.tmp.10
  mv $file.tmp.10 $file
  rm -f $file.tmp*
}

rep_common()
{
  file="$1"
  echo "process $file"
  sed -e 's$include <common/$include <$g' $file > $file.tmp.1
  mv $file.tmp.1 $file
  rm -f $file.tmp*
}

OPT="$1"
shift

for file in $* ; do
  case "$OPT" in
    "-alloc" )
      rep_alloc $file
      ;;
    "-common" )
      rep_common $file
      ;;
    * )
      printUsage
      exit 1
      ;;
  esac
done
