<?xml version="1.0" encoding="UTF-8"?>
<!-- http://www.mitchy-world.jp/xml/xpath/step02.htm -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="text" encoding="UTF-8" />
    <xsl:param name="delim" select="','" />
    <xsl:param name="quote" select="'&quot;'" />
    <xsl:param name="break" select="'&#xa;'" />
    <xsl:template match="/UserSetting">
        <xsl:for-each select="UserSetting/user">
            <xsl:value-of select="@name" />
            <xsl:value-of select="$delim" />
            <xsl:value-of select="$break" />
        </xsl:for-each>
    </xsl:template>
</xsl:stylesheet>
