This is a Visual Studio 2008 SmartDevice project using the Windows Mobile 6 pro. SDK

# Introduction #

There are not much working (Visual Studio) NPAPI samples out there. Fortunately I found one easy to adopt example and managed to write a NPAPI plugin that enables Intermec HTML5 users to embed a barcode reader in HTML5 javascript pages.

# Details #

  * To test it, copy the DLL into the plugins dir of your NPAPI enabled windows mobile browser (for example zetaKey).
  * Copy MyScanner2.htm to the device or onto a webserver.
  * Start your windows mobile html5 webkit browser
  * Open the MyScanner2.htm in the browser

## Extending the example ##

The code has many DEBUGMSG lines and you will see all the events and calls in Visual Studio Output window happening during a session.