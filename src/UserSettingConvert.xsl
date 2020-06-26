<?xml version="1.0" encoding="UTF-8"?>
<stylesheet version="1.0" 
    xmlns="http://www.w3.org/1999/XSL/Transform">
    <!--<xsl:output method="html" encoding="UTF-8" indent="no"/>-->
    <template match="/">
        <for-each select="UserSetting/user">
            "<value-of select="user" />","","",""
        </for-each>
    </template>
</stylesheet>
