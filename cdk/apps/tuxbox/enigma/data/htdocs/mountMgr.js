function mountMountPoint(id)
{
	document.location = "/control/mountMountPoint?id=" + id;
	window.setTimeout("document.location.reload()", 12000);
}

function unmountMountPoint(id)
{
	document.location = "/control/unmountMountPoint?id=" + id;
	window.setTimeout("document.location.reload()", 3000);
}

function deleteMountPoint(id)
{
	NewWindow('/control/removeMountPoint?id='+id, 'removeMountPoint', '200', '200', 'no', '5000');
}

function changeMountPoint(id)
{
	NewWindow('/control/mountPointWindow?action=change&id='+id, 'changeMountPoint', '780', '400', 'no');
}

function addMountPoint(id)
{
	NewWindow('/control/mountPointWindow?action=add&id='+id, 'addMountPoint', '780', '400', 'no');
}
