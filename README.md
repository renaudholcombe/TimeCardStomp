TimeCardStomp
========
This code is an arduino project to have a physical time tracking device. The goal is to build a server component to make interacting with the device more user-friendly, but at the moment communication is done through the serial port.

In order to add projects (up to five), use the following syntax:

	syncProject@1|Project 1@2|Project 2@

At that point, you can use buttons 1 and 2 to clock in/out and switch between projects.

To get the amount of time clocked in minutes, you can execute this command:

	syncHours@

In order to clear out any time use the following command:

	timeWipe@ 

More information about this project is at http://hyperg0at.com/2014/12/timecardstomp.php.

Enjoy!
