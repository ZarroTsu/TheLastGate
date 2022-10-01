#!/bin/bash

while true; do
	read -p "Copy FROM live dir TO bk dir? (y/n) " yn
	case $yn in 
		[yY] ) break;;
		[nN] ) exit;;
		* ) exit;;
	esac
done

cp /home/merc/.dat/*.dat ../../datBK/*.dat