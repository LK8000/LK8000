<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output methode="xml" encoding="UTF-8" indent="no" />

	<!-- Remove Comment-->
	<xsl:template match="comment()"/>

	<!-- Remove Root attribute-->
	<xsl:template match="/*">
		<xsl:element name="PMML" namespace="http://www.dmg.org/PMML-3-0">
			<xsl:apply-templates match="*"/>
		</xsl:element>
	</xsl:template>

	<!-- Remove empty attribut -->
	<xsl:template match="@*">
		<xsl:if test="string-length(.)>0">
			<xsl:copy/>
		</xsl:if>
	</xsl:template>

	<!-- copy all element -->
	<xsl:template match="node()">
		<xsl:if test="count(descendant::text()[string-length(normalize-space(.))>0] | @*[string-length(.)>0])">
			<xsl:copy>
				<xsl:apply-templates select="@*|node()"/>
			</xsl:copy>
		</xsl:if>
	</xsl:template>

	<!-- remove unneeded space -->
	<xsl:template match="text()">
		<xsl:value-of select="normalize-space()" />
	</xsl:template>

</xsl:stylesheet>
