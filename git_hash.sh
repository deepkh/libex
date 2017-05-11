#!/bin/bash
hash=`git log --pretty=oneline | cut -d ' ' -f1 | cut -d$'\n' -f1`
hash6=${hash:0:4}
echo ${hash6}
