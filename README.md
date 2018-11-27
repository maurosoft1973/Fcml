# Firebase Cloud Message for Linux (FCML)
This tool allows you to send a notification to an android phone directly from the command line from your linux system.

## Requirements
	libcurl dev library
	unzip tool

## Compiling and Install
	make all

## Compiling and Install (only Debian)
	make all-debian

## Usage
	fbcm [options]

Options are:
* -t Notification Title or (--title=<str>)
* -m Notification Message or (--message=<str>)
* -s Token Device or (--tokendevice=<str>)
* -k Server Key or (--serverkey=<str>)
* -d Debug Mode or (--debug=<int>)
* -f Path of the Configuration File or (--configuration=<str>)

The structure of configuration file is:
	[general]
	apiserver={apiserver}
	device={device}

{apiserver} is the server key by Firebase Cloud Messaging

Server Key can be found in:

1. Firebase project settings
2. Cloud Messaging
3. then copy the server key

fbcm outputs JSON data to **stdout**.

## Example of Usage
	fbcm -f /etc/fbcm.conf -t "Hello Title" -m "Hello Message"

	or

	fbcm -t "Hello Title" -m "Hello Message" -s "TokenDevice" -k "ServerKey"
