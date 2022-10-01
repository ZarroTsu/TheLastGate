#!/bin/bash

while true; do
	read -p "Copy FROM bk dir TO live dir? (y/n) " yn
	case $yn in 
		[yY] ) break;;
		[nN] ) exit;;
		* ) exit;;
	esac
done

cp ../../datBK/*.dat /home/merc/.dat/*.dat