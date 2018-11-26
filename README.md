# Command Line Firebase Cloud Message
This tool allows you to send a notification to an android phone directly from the command line.

## Compiling and Install
make all

## Usage
	fbcm [options]

Options are:
* -t Notification Title or (--title=<str>)
* -m Notification Message or (--message=<str>)
* -s Token Device or (--tokendevice=<str>)
* -k Server Key or (--serverkey=<str>)
* -d Debug Mode or (--debug=<int>)
* -f Path of the Configuration File or (--configuration=<str>)
  
fbcm outputs JSON data to **stdout**.
