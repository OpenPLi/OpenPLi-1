function record(ref, start, duration, description, channel, timerendaction)
{
	NewWindow('/addTimerEvent?ref=' + ref + '&start=' + start + '&duration=' + duration + '&descr=' + description + '&channel=' + channel + '&after_event=' + timerendaction, 'record', '200', '200', 'no', '5000');
}

function switchChannel(path)
{
	document.location = "/cgi-bin/zapTo?path=" + path;
}
