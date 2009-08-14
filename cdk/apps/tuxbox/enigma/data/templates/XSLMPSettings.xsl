<?xml version="1.0" encoding="iso-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">
<html>
	<head>
		<title>Streaming Server Settings</title>
		<link rel="stylesheet" type="text/css" href="/webif.css"/>
		<script language="javascript" type="text/javascript" src="/window.js"></script>
		<script language="javascript" type="text/javascript" src="/movieplayer.js"></script>
	</head>
	<body>
		<h2>Server Settings</h2>
		<table id="epg" width="100%" border="1" cellspacing="0" cellpadding="5">
			<tr valign="middle" align="left">
				<td width="300">Server IP Address</td>
				<td>
					<xsl:value-of select="/vlc/server/@ip"/>
				</td>
			</tr>
			<tr valign="middle" align="left">
				<td>Server Movie Directory</td>
				<td>
					<xsl:value-of select="/vlc/config/@startdir"/>
				</td>
			</tr>
			<tr valign="middle" align="left">
				<td>Server CD/DVD Drive</td>
				<td>
					<xsl:value-of select="/vlc/config/@cddrive"/>
				</td>
			</tr>
		</table>
		<p>#SERVEREDITBUTTON#</p>
		<h2>VLC Settings</h2>
		<table id="epg" width="100%" border="1" cellspacing="0" cellpadding="5">
			<tr valign="middle" align="left">
				<td width="300">VLC Webif Port</td>
				<td>
					<xsl:value-of select="/vlc/server/@webif-port"/>
				</td>
			</tr>
			<tr valign="middle" align="left">
				<td>VLC Streaming Port</td>
				<td>
					<xsl:value-of select="/vlc/server/@stream-port"/>
				</td>
			</tr>
			<tr valign="middle" align="left">
				<td>VLC Admin Userid</td>
				<td>
					<xsl:value-of select="/vlc/server/@user"/>
				</td>
			</tr>
			<tr valign="middle" align="left">
				<td>VLC Admin Password</td>
				<td>
					<xsl:value-of select="/vlc/server/@pass"/>
				</td>
			</tr>
		</table>
		<p>#VLCEDITBUTTON#</p>
		<h2>Video Settings</h2>
		<table id="epg" width="100%" border="1" cellspacing="0" cellpadding="5">
			<thead>
				<tr>
					<th>Video Type</th>
					<th>File Extension</th>
					<th>Video Transcode</th>
					<th>Video Codec</th>
					<th>Video Rate</th>
					<th>Video Ratio</th>
					<th>Audio Transcode</th>
					<th>Audio Rate</th>
					<th>FPS</th>
					<th>Sub</th>
					<th>Action</th>
					
				</tr>
			</thead>
			<tbody>
				<xsl:for-each select="vlc/setup">
					<tr valign="middle" align="center">
						<td>
							<xsl:value-of select="@name"/>
						</td>
						<td>
							<xsl:value-of select="@ext"/>
						</td>
						<td>
							<xsl:value-of select="@Videotranscode"/>
						</td>
						<td>
							<xsl:value-of select="@Videocodec"/>
						</td>
						<td>
							<xsl:value-of select="@Videorate"/>
						</td>
						<td>
							<xsl:value-of select="@Videosize"/>
						</td>
						<td>
							<xsl:value-of select="@Audiotranscode"/>
						</td>
						<td>
							<xsl:value-of select="@Audiorate"/>
						</td>
						<td>
							<xsl:value-of select="@fps"/>
						</td>
						<td>
							<xsl:value-of select="@soutadd"/>
						</td>
						<td>
							<a href="javascript:editStreamingServerVideoSettings('{@name}', '{@ext}')">
								<img src="/edit.gif" border="0"/>
							</a>
						</td>
					</tr>
				</xsl:for-each>
			</tbody>
		</table>
	</body>
</html>
</xsl:template>
</xsl:stylesheet>
