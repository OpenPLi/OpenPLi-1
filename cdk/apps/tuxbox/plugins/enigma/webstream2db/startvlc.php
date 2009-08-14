<?php
// Let's see if the machine is Linux or Windows
if (ereg("Linux", $_SERVER["SERVER_SOFTWARE"]))
{	
	//This is a Linux server
	exec("killall vlc"); // First we stop the ongoing stream
	sleep(5); //lets wait a few seconds so everything kills well --> send me info if you have better settings, I took 5 seconds to be safe, but could be less
	$url = "/usr/local/bin/vlc /-vvv --daemon ".$_GET['url']." :sout='#transcode{acodec=mpga,vcodec=mpgv,vb=1000,width=704,height=576,ab=192,channels=2}:std{access=http,mux=ts,url=:".$_GET['port']."/dboxstream}'";
	#$url = "/usr/bin/vlc /-vvv --daemon ".$_GET['url']." :sout='#transcode{acodec=mpga,vcodec=mpgv,vb=1000,width=704,height=576,ab=192,channels=2}:std{access=http,mux=ts,url=:".$_GET['port']."/dboxstream}'";
	exec($url); // execute the correct URL
	$dbstart = "wget -O - 'http://".$_GET['ip']."/cgi-bin/movieplayer.m3u?command=start&mrl=dummy.ts'";
	sleep(10); //lets wait a few seconds to start the streaming --> send me info if you have better settings, I took 10 seconds to be safe, but could be less
	exec($dbstart);
}
else
{ 
	//This is a Windows server
	// First kill ongoing signals --> you need pskill.exe for this!
	exec("c:\phpdev\pskill.exe vlc.exe");
	sleep(2); //lets wait a few seconds so everything kills well --> send me info if you have better settings, I took 2 seconds to be safe, but could be less
	$url = "C:\phpdev\vlc\vlc.exe ".$_GET['url']." :sout=#transcode{vcodec=mpgv,vb=1000,width=704,height=576,acodec=mpga,ab=192,channels=2}:duplicate{dst=std{access=http,mux=ts,url=:".$_GET['port']."/dboxstream}}";
	$dbstart = "C:\Progra~1\GnuWin32\bin\wget.exe -O - http://".$_GET['ip']."/cgi-bin/movieplayer.m3u?command=start&mrl=dummy.ts";	//exec("net start \"VLC\""); // start buffering on the PC
	exec($dbstart); // now prepare the DreamBox to accept incoming streams --> I start this because vlc causes the script to pause (how about programming skills ;) but hey it works)
	exec($url); // add the service, so you can continue using the PC
	#Please send me a working script with VLC as service. I couldn't get that work properly!
} 
	
?>
