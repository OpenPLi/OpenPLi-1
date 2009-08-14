<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">
<html>
	<head>
		<title>EPG Overview</title>
		<link rel="stylesheet" type="text/css" href="/webif.css"/>
		<script language="javascript" type="text/javascript" src="/window.js"></script>
		<script language="javascript" type="text/javascript" src="/epg.js"></script>
	</head>
	<body onLoad="maximizeWindow()">
		<table id="epg" width="100%" border="0" cellspacing="0" cellpadding="5">
			<thead>
				<tr>
					<th colspan="3"><xsl:value-of select="/service_epg/service/name"/></th>
				</tr>
			</thead>
			<tbody>
				<xsl:for-each select="service_epg/event">
					<tr valign="middle">
						<td>
							<span class="time"><xsl:value-of select="date"/> - <xsl:value-of select="time"/></span>
						</td>
						<td>
							<a href="javascript:record('{/service_epg/service/reference},{start},{duration},{description},{/service_epg/service/name},{timerendaction}')">
								<img src="/timer.gif" border="0"/>
							</a>
						</td>
						<td class="{genrecategory}">
							<span class="event"><xsl:value-of select="description"/></span>
							<br/>
							Genre: <xsl:value-of select="genre"/>
							<br/>
							<span class="description"><xsl:value-of select="details"/></span>
						</td>
					</tr>
				</xsl:for-each>
			</tbody>
		</table>
	</body>
</html>
</xsl:template>
</xsl:stylesheet>
