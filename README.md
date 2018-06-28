
# ApplicationCache-50X

Installs an ApplicationCache.db that contains a modified version of [Al-Azif's exploit host](https://github.com/Al-Azif/ps4-exploit-host) to your PS4, eliminating the need for a PC, smart device (Android/iPhone/Tablet/etc), or the ESP8266.  It also includes an option to make the browser never remember the last open page. (accessible via HTML payload)

Click [here](https://cdn.rawgit.com/HotdogWithMustard/ApplicationCache-50X/cce862db884c00efa520cf7312ecc9fe296df869/bin/Index.html) to run the HTML payload.

## COMPILING

In order to compile, you will need to modify your SDK to expose a couple sceKernelLoadStartModule handles, you can either replace the files with the code below or find the changes and apply them yourself.

	/libPS4/include/network.h: https://hastebin.com/adixecagux.cs
	/libPS4/source/network.c: https://hastebin.com/ihewitisen.cpp

After you've done that, you should be able to use ./build.sh to compile.

## EXECUTING

You can either send the .bin file or use the HTML payload, It's recommended to use the HTML payload as it gives you some additional options.

## NOTES

 - Clearing your Website Data in the browser will delete the cache.
 - Requires an internet connection to download ApplicationCache.db.
 - Uses sockets, so no SSL/TLS support. :/

If anyone knows how to get sceHttpInit working, let me know.
