<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output methode="xml" encoding="UTF-16LE" indent="no" />

	<!-- Remove Comment-->
	<xsl:template match="comment()"/>

	<!-- Remove Root attribute-->
	<xsl:template match="/*">
		<xsl:element name="PMML" namespace="http://www.dmg.org/PMML-3-0">
			<xsl:apply-templates match="*"/>
		</xsl:element>
	</xsl:template>

	<!-- copy all element and attribut -->
	<xsl:template match="@*|node()">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
	</xsl:template>

	<!-- remove unneeded space -->
	<xsl:template match="text()">
		<xsl:value-of select="normalize-space()" />
	</xsl:template>

</xsl:stylesheet>
