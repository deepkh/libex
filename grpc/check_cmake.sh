#!/bin/bash

vercomp () {
	if [[ $1 == $2 ]]
	then
			return 0
	fi
	local IFS=.
	local i ver1=($1) ver2=($2)
	# fill empty fields in ver1 with zeros
	for ((i=${#ver1[@]}; i<${#ver2[@]}; i++))
	do
			ver1[i]=0
	done
	for ((i=0; i<${#ver1[@]}; i++))
	do
			if [[ -z ${ver2[i]} ]]
			then
					# fill empty fields in ver2 with zeros
					ver2[i]=0
			fi
			if ((10#${ver1[i]} > 10#${ver2[i]}))
			then
					return 1
			fi
			if ((10#${ver1[i]} < 10#${ver2[i]}))
			then
					return 2
			fi
	done
	echo X0
	return 0
}

testvercomp () {
	vercomp $1 $2
	case $? in
			0) op='=';;
			1) op='>';;
			2) op='<';;
	esac
	if [[ $op != $3 ]]
	then
			echo "FAIL: Expected '$3', Actual '$op', Arg1 '$1', Arg2 '$2'"
			return 1
	else
			echo "Pass: '$1 $op $2'"
	fi
	return 0
}

install_cmake() {
	echo "=== install cmake  ==="
	sudo mv /usr/bin/cmake /usr/bin/cmake.old
	wget -q -O cmake-linux.sh https://github.com/Kitware/CMake/releases/download/v3.16.1/cmake-3.16.1-Linux-x86_64.sh
	sudo sh cmake-linux.sh -- --skip-license --prefix=/usr/
}

check_cmake_version() {
	echo "=== check cmake version ==="
	testvercomp "`cmake --version | grep version | awk '{print $3}'`" "3.16.0" '>'
	if [ $? = 1 ];then
	echo $?
		install_cmake
	fi
}

