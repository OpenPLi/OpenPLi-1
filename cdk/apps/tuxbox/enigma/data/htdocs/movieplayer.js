function editStreamingServerSettings()
{
	NewWindow('/cgi-bin/editStreamingServerSettings', 'server', '400', '200', 'no');
}

function editStreamingServerVideoSettings(name, extension)
{
	NewWindow('/cgi-bin/editStreamingServerVideoSettings?name=' + name + '&extension=' + extension, 'video', '400', '400', 'no');
}

function editStreamingServerVLCSettings()
{
	NewWindow('/cgi-bin/editStreamingServerVLCSettings', 'vlc', '400', '200', 'no');
}