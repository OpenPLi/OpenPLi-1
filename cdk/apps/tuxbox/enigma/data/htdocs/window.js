function NewWindow(mypage, myname, w, h, scroll, timeout)
{
	if (screen.width >= 800)
	{
		var winl = (screen.width - w) / 2;
		var wint = (screen.height - h) / 2;
		winprops = 'height='+h+', width='+w+', top='+wint+', left='+winl+', scrollbars='+scroll+', resizable'
		win = window.open(mypage, myname, winprops)
		if (parseInt(navigator.appVersion) >= 4)
			win.window.focus();
		if (timeout > 0)
			win.window.setTimeout("close()", timeout);
	}
	else
		document.location = mypage;
}

function maximizeWindow()
{
	top.window.moveTo(0, 0);
	if (document.all)
	{
		top.window.resizeTo(screen.availWidth, screen.availHeight);
	}
	else
	{
		top.window.outerHeight = screen.availHeight;
		top.window.outerWidth = screen.availWidth;
	}
	top.window.focus();
}

