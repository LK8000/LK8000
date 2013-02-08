/**
 ****************************************************************************
 * <P> XML.c - implementation file for basic XML parser written in ANSI C++ 
 * for portability. It works by using recursion and a node tree for breaking 
 * down the elements of an XML document.  </P>
 *
 * @version     V1.08
 *
 * @author      Frank Vanden Berghen
 * based on original implementation by Martyn C Brown
 *
 * NOTE:
 * 
 *   If you add "#define APPROXIMATE_PARSING", on the first line of this file
 *   the parser will see the following XML-stream:
 *     <data name="n1"> 
 *     <data name="n2">
 *     <data name="n3" /> 
 *   as equivalent to the following XML-stream:
 *     <data name="n1" /> 
 *     <data name="n2" />
 *     <data name="n3" /> 
 *   This can be useful for badly-formed XML-streams but prevent the use
 *   of the following XML-stream:
 *     <data name="n1"> 
 *        <data name="n2">
 *            <data name="n3" /> 
 *        </data>
 *     </data>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 ****************************************************************************
 */
#include "externs.h"
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xmlParser.h"



void StartupStore(const TCHAR *Str, ...); // 100102

bool XMLNode::GlobalError = false;

XMLNode XMLNode::emptyXMLNode;
XMLClear XMLNode::emptyXMLClear={ NULL, NULL, NULL};
XMLAttribute XMLNode::emptyXMLAttribute={ NULL, NULL};

#if !defined(WIN32) && !defined(UNDER_CE)
int _tcslen(const char *c)   { return strlen(c); }
int _tcsnicmp(const char *c1, const char *c2, int l) { return strncasecmp(c1,c2,l); }
int _tcsicmp(const char *c1, const char *c2) { return strcasecmp(c1,c2); }
char *_tcsstr(const char *c1, const char *c2) { return (char*)strstr(c1,c2); }
char *_tcschr(const char *c1, int c2) { return (char*)strchr(c1,c2); }
char *_tcscpy(char *c1, const char *c2) { return (char*)strcpy(c1,c2); }
#endif

inline int mmin( const int t1, const int t2 ) { return t1 < t2 ? t1 : t2; }

// Enumeration used to decipher what type a token is
typedef enum TokenTypeTag
{
    eTokenText = 0,
    eTokenQuotedText,
    eTokenTagStart,         /* "<"            */
    eTokenTagEnd,           /* "</"           */
    eTokenCloseTag,         /* ">"            */
    eTokenEquals,           /* "="            */
    eTokenDeclaration,      /* "<?"           */
    eTokenShortHandClose,   /* "/>"           */
    eTokenClear,
    eTokenError
}TokenTypeTag;

#define INDENTCHAR    _T('\t')

typedef struct ClearTag
{
    LPCTSTR lpszOpen;
    LPCTSTR lpszClose;
} ClearTag;

// Main structure used for parsing XML
typedef struct XML
{
    LPCTSTR                lpXML;
    int                    nIndex;
    enum XMLError          error;
    LPCTSTR                lpEndTag;
    int                    cbEndTag;
    LPCTSTR                lpNewElement;
    int                    cbNewElement;
    int                    nFirst;
    ClearTag               *pClrTags;
} XML;

typedef struct 
{
    ClearTag    *pClr;
    LPCTSTR     pStr;
} NextToken;

// Enumeration used when parsing attributes
typedef enum Attrib
{
    eAttribName = 0,
    eAttribEquals,
    eAttribValue
} Attrib;

// Enumeration used when parsing elements to dictate whether we are currently
// inside a tag
typedef enum Status
{
    eInsideTag = 0,
    eOutsideTag
} Status;

// private:
LPTSTR toXMLString(LPTSTR dest,LPCTSTR source)
{
    LPTSTR dd=dest;
    while (*source)
    {
        switch (*source)
        {
        case '<' : _tcscpy(dest,_T("&lt;"  )); dest+=4; break;
        case '>' : _tcscpy(dest,_T("&gt;"  )); dest+=4; break;
        case '&' : _tcscpy(dest,_T("&amp;" )); dest+=5; break;
        case '\'': _tcscpy(dest,_T("&apos;")); dest+=6; break;
        case '"' : _tcscpy(dest,_T("&quot;")); dest+=6; break;
        default:  *dest=*source; dest++; break;
        }
        source++;
    }
    *dest=0;
    return dd;
}

// private:
int lengthXMLString(LPCTSTR source)
{
    int r=0;
    while (*source)
    {
        switch (*source)
        {
        case '<':  r+=3; break;
        case '>' : r+=3; break;
        case '&' : r+=4; break;
        case '\'': r+=5; break;
        case '"' : r+=5; break;
        }
        source++; r++;
    }
    return r;
}

LPTSTR toXMLString(LPCTSTR source)
{
    LPTSTR dest=(LPTSTR)malloc((lengthXMLString(source)+1)*sizeof(TCHAR));
    LKASSERT(dest);
    return toXMLString(dest,source);
}

LPTSTR toXMLStringFast(LPTSTR *dest,int *destSz, LPCTSTR source)
{
    int l=lengthXMLString(source)+1;
    if (l>*destSz) { *destSz=l; *dest=(LPTSTR)realloc(*dest,l*sizeof(TCHAR)); }
    return toXMLString(*dest,source);
}

// private:
LPTSTR fromXMLString(LPCTSTR s, int lo)
{
    // This function is the opposite of the function "toXMLString". It decodes the escape 
    // sequences &amp;, &quot;, &apos;, &lt;, &gt; and replace them by the characters 
    // &,",',<,>. This function is used internally by the XML Parser. All the calls to 
    // the XML library will always gives you back "decoded" strings.
    //
    // in: string (s) and length (lo) of string
    // out:  new allocated string converted from xml
    if (!s) return NULL;

    int ll=0;
    LPTSTR d;
    LPCTSTR ss=s;
    while (((lo--)>0)&&(*s))
    {
        if (*s==_T('&'))
        {
            s++;
                 if (_tcsnicmp(s,_T("lt;"  ),3)==0) { s+=2; lo-=3; }
            else if (_tcsnicmp(s,_T("gt;"  ),3)==0) { s+=2; lo-=3; }
            else if (_tcsnicmp(s,_T("amp;" ),4)==0) { s+=3; lo-=4; }
            else if (_tcsnicmp(s,_T("apos;"),5)==0) { s+=4; lo-=5; }
            else if (_tcsnicmp(s,_T("quot;"),5)==0) { s+=4; lo-=5; }
            else
            {
                ll=0; while (s[ll]&&(s[ll]!=_T(';'))&&(ll<10)) ll++; ll++;
                d=(LPTSTR)malloc((ll+1)*sizeof(TCHAR));
                LKASSERT(d);
                d[ll]=0;
                while(ll--) d[ll]=s[ll];
#ifdef DEBUG
#ifdef _UNICODE
                    printf("unknown escape character: '&%S'",d);
#else
                    printf("unknown escape character: '&%s'",d);
#endif
#endif
                free(d);
		XMLNode::GlobalError = true;
                return (LPTSTR)NULL;
            }
        };
        ll++; s++;
    }

    d=(LPTSTR)malloc((ll+1)*sizeof(TCHAR));
    LKASSERT(d);
    s=d;
    while (ll--)
    {
        if (*ss==_T('&'))
        {
            ss++;
                 if (_tcsnicmp(ss,_T("lt;"  ),3)==0) { *(d++)=_T('<' ); ss+=3; } 
            else if (_tcsnicmp(ss,_T("gt;"  ),3)==0) { *(d++)=_T('>' ); ss+=3; } 
            else if (_tcsnicmp(ss,_T("amp;" ),4)==0) { *(d++)=_T('&' ); ss+=4; }
            else if (_tcsnicmp(ss,_T("apos;"),5)==0) { *(d++)=_T('\''); ss+=5; }
            else                                     {
                *(d++)=_T('"' ); ss+=5; }
        } else { *(d++)=*ss; ss++; }
    }
    *d=0;
    return (LPTSTR)s;
}

// private:
char myTagCompare(LPCTSTR cclose, LPCTSTR copen)
// !!!! WARNING strange convention&:
// return 0 if equals
// return 1 if different
{
    if (!cclose) return 1;
    int l=(int)_tcslen(cclose);
    if (_tcsnicmp(cclose, copen, l)!=0) return 1;
    const TCHAR c=copen[l];
    if ((c==_T('\n'))||
        (c==_T(' ' ))||
        (c==_T('\t'))||
        (c==_T('\r'))||
        (c==_T('/' ))||
        (c==_T('<' ))||
        (c==_T('>' ))||
        (c==_T('=' ))) return 0;
    return 1;
}

// private:
// update "order" information when deleting a content of a XMLNode
void XMLNode::removeOrderElement(XMLNodeData *d, XMLElementType t, int index)
{
    int j=(int)((index<<2)+t),i=0,n=nElement(d)+1, *o=d->pOrder;
    while ((o[i]!=j)&&(i<n)) i++;
    n--;
    memmove(o+i, o+i+1, (n-i)*sizeof(int));
    for (;i<n;i++)
        if ((o[i]&3)==(int)t) o[i]-=4;
// We should normally do:
// d->pOrder=(int)realloc(d->pOrder,n*sizeof(int));
// but we skip reallocation because it's too time consuming.
// Anyway, at the end, it will be free'd completely at once.
}

// Obtain the next character from the string. 
static inline TCHAR getNextChar(XML *pXML)
{
    TCHAR ch = pXML->lpXML[pXML->nIndex];    
    if (ch!=0) pXML->nIndex++;
    return ch;
}

// Find next non-white space character.  
static TCHAR FindNonWhiteSpace(XML *pXML)
{
    TCHAR ch=0; // VENTA3 fix initialize
    int nExit = FALSE;

    LKASSERT(pXML);

    // Iterate through characters in the string until we find a NULL or a
    // non-white space character
    while((nExit == FALSE) && ((ch = getNextChar(pXML)) != 0))
    {
        switch(ch)
        {
        // Ignore white space
        case _T('\n'):
        case _T(' '):
        case _T('\t'):
        case _T('\r'): continue;            
        default: nExit = TRUE;
        }
    }
    return ch;
}

// Find the next token in a string.
// pcbToken contains the number of characters that have been read.
static NextToken GetNextToken(XML *pXML, int *pcbToken, enum TokenTypeTag *pType)
{
    NextToken        result = { 0 };
    LPCTSTR          lpXML;
    TCHAR            ch;
    TCHAR            chTemp;
    int              nSize;
    int              nFoundMatch;
    int              nExit;
    int              n;
    LPCTSTR          lpszOpen;
    int              cbOpen;
    int              nIsText = FALSE;

    // Find next non-white space character
    ch = FindNonWhiteSpace(pXML);

    if (ch)
    {
        // Cache the current string pointer
        lpXML = pXML->lpXML;
        result.pStr = &lpXML[pXML->nIndex-1];

        // First check whether the token is in the clear tag list (meaning it 
        // does not need formatting).
        n = 0;

        while(TRUE)
        {
            // Obtain the name of the open part of the clear tag
            lpszOpen = pXML->pClrTags[n].lpszOpen;

            if (lpszOpen)
            {
                // Compare the open tag with the current token
                cbOpen = (int)_tcslen(lpszOpen);
                // if (myTagCompare(lpszOpen, result.pStr) == 0)
                if (_tcsnicmp(lpszOpen, result.pStr, cbOpen)==0)
                {
                    result.pClr = &pXML->pClrTags[n];
                    pXML->nIndex += (int)(_tcslen(lpszOpen)-1);
                    *pType  = eTokenClear;
                    return result;
                }
                n++;
            }
            else break;
        }
        // If we didn't find a clear tag then check for standard tokens
        chTemp = 0;
        lpXML = pXML->lpXML;
        
        switch(ch)
        {
        // Check for quotes
        case _T('\''):
        case _T('\"'):
            // Type of token
            *pType = eTokenQuotedText;
            chTemp = ch;
            n=pXML->nIndex;

            // Set the size
            nSize = 1;
            nFoundMatch = FALSE;

            // Search through the string to find a matching quote
            while(((ch = getNextChar(pXML))) != 0)
            {
                nSize++;
                if (ch==chTemp) { nFoundMatch = TRUE; break; }
                if (ch==_T('<')) break;
            }

            // If we failed to find a matching quote
            if (nFoundMatch == FALSE)
            {
                pXML->nIndex=n-1;
                ch=getNextChar(pXML);
                nIsText=TRUE;
                break;
            } 

            //  4.02.2002 
            if (FindNonWhiteSpace(pXML))
            {
                pXML->nIndex--;
            }

            break;

        // Equals (used with attribute values)
        case _T('='):
            nSize = 1;
            *pType = eTokenEquals;
            break;

        // Close tag
        case _T('>'):
            nSize = 1;
            *pType = eTokenCloseTag;
            break;

        // Check for tag start and tag end
        case _T('<'):

            // Peek at the next character to see if we have an end tag '</',
            // or an xml declaration '<?'
            chTemp = pXML->lpXML[pXML->nIndex];

            // If we have a tag end...
            if (chTemp == _T('/'))
            {
                // Set the type and ensure we point at the next character 
                getNextChar(pXML);
                *pType = eTokenTagEnd;
                nSize = 2;
            }

            // If we have an XML declaration tag
            else if (chTemp == _T('?'))
            {

                // Set the type and ensure we point at the next character 
                getNextChar(pXML);
                *pType = eTokenDeclaration;
                nSize = 2;
            }

            // Otherwise we must have a start tag
            else 
            {
                *pType = eTokenTagStart;
                nSize = 1;
            }
            break;

        // Check to see if we have a short hand type end tag ('/>').
        case _T('/'):

            // Peek at the next character to see if we have a short end tag '/>'
            chTemp = pXML->lpXML[pXML->nIndex];

            // If we have a short hand end tag...
            if (chTemp == _T('>'))
            {
                // Set the type and ensure we point at the next character 
                getNextChar(pXML);
                *pType = eTokenShortHandClose;
                nSize = 2;
                break;
            }

            // If we haven't found a short hand closing tag then drop into the
            // text process

        // Other characters
        default:
            nIsText = TRUE;
        }

        // If this is a TEXT node
        if (nIsText)
        {
            // Indicate we are dealing with text
            *pType = eTokenText;
            nSize = 1;
            nExit = FALSE;

            while((nExit == FALSE) && ((ch = getNextChar(pXML)) != 0))
            {
                switch(ch)
                {
                // Break when we find white space
                case _T('\n'):
                case _T(' '):
                case _T('\t'):
                case _T('\r'):
                    nExit = TRUE;
                    break;

                // If we find a slash then this maybe text or a short hand end tag.
                case _T('/'):
                    
                    // Peek at the next character to see it we have short hand end tag
                    chTemp = pXML->lpXML[pXML->nIndex];

                    // If we found a short hand end tag then we need to exit the loop
                    if (chTemp == _T('>'))
                    {
                        pXML->nIndex--; //  03.02.2002 
                        nExit = TRUE;
                    }
                    else
                    {
                        nSize++;
                    }
                    break;

                // Break when we find a terminator and decrement the index and
                // column count so that we are pointing at the right character
                // the next time we are called.
                case _T('<'):
                case _T('>'):
                case _T('='):
                    pXML->nIndex--;
                    nExit = TRUE;
                    break;

                case 0:
                    nExit = TRUE;
                    break;

                default:
                    nSize++;
                }
            }
        }
        *pcbToken = nSize;
    } else 
    {
        // If we failed to obtain a valid character
        *pcbToken = 0;
        *pType = eTokenError;
        result.pStr=NULL;
    }

    return result;
}

// Parse XML errors into a user friendly string.  
LPCTSTR XMLNode::getError(XMLError error)
{
    switch (error)
    {
    case eXMLErrorNone:              return _T("No error");
    case eXMLErrorEmpty:             return _T("No XML data");
    case eXMLErrorFirstNotStartTag:  return _T("First token not start tag");
    case eXMLErrorMissingTagName:    return _T("Missing start tag name");
    case eXMLErrorMissingEndTagName: return _T("Missing end tag name");
    case eXMLErrorNoMatchingQuote:   return _T("Unmatched quote");
    case eXMLErrorUnmatchedEndTag:   return _T("Unmatched end tag");
    case eXMLErrorUnexpectedToken:   return _T("Unexpected token found");
    case eXMLErrorInvalidTag:        return _T("Invalid tag found");
    case eXMLErrorNoElements:        return _T("No elements found");
    case eXMLErrorFileNotFound:      return _T("File not found");
    case eXMLErrorTagNotFound:       return _T("First Tag not found");
    };
    return _T("Unknown");
}

// private:
XMLNode::XMLNode(XMLNode *pParent, LPCTSTR lpszName, int _isDeclaration)
{
    d=(XMLNodeData*)malloc(sizeof(XMLNodeData));
    LKASSERT(d);
    d->ref_count=1;

    d->lpszName=lpszName;

    d->nChild= 0;
    d->nText = 0;
    d->nClear = 0;
    d->nAttribute = 0;

    d->isDeclaration = _isDeclaration;

    d->pParent = pParent;
    d->pChild= NULL;
    d->pText= NULL;
    d->pClear= NULL;
    d->pAttribute= NULL;
    d->pOrder= NULL;
}

const int memoryIncrease=50;

static void *myRealloc(void *p, int newsize, int memInc, int sizeofElem)
{
    int blocks = newsize / memInc+1;
    if (p==NULL) { 
      void* v = malloc(blocks*memInc*sizeofElem);
      LKASSERT(v);
      return v; 
    }
    if ((newsize % memInc)==0) {
      p=realloc(p,blocks*memInc*sizeofElem);
      LKASSERT(p);
    }
    return p;
}

void XMLNode::addToOrder(int index, int type)
{
    int n=nElement();
    d->pOrder=(int*)myRealloc(d->pOrder,n+1,memoryIncrease*3,sizeof(int));
    LKASSERT(d->pOrder);
    d->pOrder[n]=(index<<2)+type;
}

// Add a child node to the given element.
XMLNode XMLNode::AddChild(LPCTSTR lpszName, int _isDeclaration)
{
    if (!lpszName) return emptyXMLNode;
    int nc=d->nChild;
    d->pChild=(XMLNode*)myRealloc(d->pChild,(nc+1),memoryIncrease,sizeof(XMLNode));
    LKASSERT(d->pChild);
    d->pChild[nc].d=NULL;
    d->pChild[nc]=XMLNode(this,lpszName,_isDeclaration);
    addToOrder(nc,eNodeChild);
    d->nChild++;
    return d->pChild[nc];
}

// Add an attribute to an element. 
XMLAttribute *XMLNode::AddAttribute(LPCTSTR lpszName, LPCTSTR lpszValuev)
{
    if (!lpszName) return &emptyXMLAttribute;
    int na=d->nAttribute;
    d->pAttribute=(XMLAttribute*)myRealloc(d->pAttribute,(na+1),memoryIncrease,sizeof(XMLAttribute));
    XMLAttribute *pAttr=d->pAttribute+na;
    pAttr->lpszName = lpszName;
    pAttr->lpszValue = lpszValuev;
    addToOrder(na,eNodeAttribute);
    d->nAttribute++;
    return pAttr;
}

// Add text to the element.  
LPCTSTR XMLNode::AddText(LPCTSTR lpszValue)
{
    if (!lpszValue) return NULL;
    int nt=d->nText;
    d->pText=(LPCTSTR*)myRealloc(d->pText,(nt+1),memoryIncrease,sizeof(LPTSTR));
    d->pText[nt]=lpszValue;
    addToOrder(nt,eNodeText);
    d->nText++;
    return d->pText[nt];
}

// Add clear (unformatted) text to the element.
XMLClear *XMLNode::AddClear(LPCTSTR lpszValue, LPCTSTR lpszOpen, LPCTSTR lpszClose)
{
    if (!lpszValue) return &emptyXMLClear;
    int nc=d->nClear;
    d->pClear=(XMLClear *)myRealloc(d->pClear,(nc+1),memoryIncrease,sizeof(XMLClear));
    XMLClear *pNewClear=d->pClear+nc;
    pNewClear->lpszValue = lpszValue;
    pNewClear->lpszOpenTag = lpszOpen;
    pNewClear->lpszCloseTag = lpszClose;
    addToOrder(nc,eNodeClear);
    d->nClear++;
    return pNewClear;
}

// Trim the end of the text to remove white space characters.
static void FindEndOfText(LPCTSTR lpszToken, int *pcbText)
{
    TCHAR   ch;
    int     cbText;
    LKASSERT(lpszToken);
    LKASSERT(pcbText);
    cbText = (*pcbText)-1;    
    while(TRUE)
    {
        LKASSERT(cbText >= 0);
        ch = lpszToken[cbText];
        switch(ch)
        {
        case _T('\r'):
        case _T('\n'):
        case _T('\t'):
        case _T(' '): cbText--; break;
        default: *pcbText = cbText+1; return;
        }
    }
}

// Duplicate a given string. 
LPTSTR stringDup(LPCTSTR lpszData, int cbData)
{
    if (lpszData==NULL) return NULL;

    LPTSTR lpszNew;
    if (cbData==0) cbData=(int)_tcslen(lpszData);
    lpszNew = (LPTSTR)malloc((cbData+1) * sizeof(TCHAR));
    LKASSERT(lpszNew);
    if (lpszNew)
    {
        memcpy(lpszNew, lpszData, (cbData) * sizeof(TCHAR));
        lpszNew[cbData] = (TCHAR)NULL;
    }
    return lpszNew;
}

// private:
// Parse a clear (unformatted) type node.
int XMLNode::ParseClearTag(void *px, void *pa)
{
    XML *pXML=(XML *)px;
    ClearTag *pClear=(ClearTag *)pa;
    int cbTemp = 0;
    LPTSTR lpszTemp;
    LPCTSTR lpszXML = &pXML->lpXML[pXML->nIndex];

    // Find the closing tag
    lpszTemp = _tcsstr((wchar_t*)lpszXML, pClear->lpszClose);

    // Iterate through the tokens until we find the closing tag.
    if (lpszTemp)
    {
        // Cache the size and increment the index
        cbTemp = (int)(lpszTemp - lpszXML);

        pXML->nIndex += cbTemp;
        pXML->nIndex += (int)_tcslen(pClear->lpszClose);
        
        // Add the clear node to the current element
        AddClear(stringDup(lpszXML,cbTemp), pClear->lpszOpen, pClear->lpszClose);
        return TRUE;
    }

    // If we failed to find the end tag
    pXML->error = eXMLErrorUnmatchedEndTag;
    return FALSE;
}

// private:
// Recursively parse an XML element.
int XMLNode::ParseXMLElement(void *pa)
{
    XML *pXML=(XML *)pa;
    int cbToken;
    enum TokenTypeTag type;
    NextToken token;
    LPCTSTR lpszTemp = NULL;
    int cbTemp;
    int nDeclaration;
    LPCTSTR lpszText = NULL;
    XMLNode pNew;
    enum Status status; // inside or outside a tag
    enum Attrib attrib = eAttribName;

    LKASSERT(pXML);
    if (!pXML) { // 100102
	StartupStore(_T("...... ParseXMLElem null value\n"));
	return FALSE;
    }

    // If this is the first call to the function
    if (pXML->nFirst)
    {
        // Assume we are outside of a tag definition
        pXML->nFirst = FALSE;
        status = eOutsideTag;
    } else 
    {
        // If this is not the first call then we should only be called when inside a tag.
        status = eInsideTag;
    }

 
    // Iterate through the tokens in the document
    while(TRUE)
    {
        // Obtain the next token
        token = GetNextToken(pXML, &cbToken, &type);

        if (type != eTokenError)
        {
            // Check the current status
            switch(status)
            {

            // If we are outside of a tag definition
            case eOutsideTag:

                // Check what type of token we obtained
                switch(type)
                {
                // If we have found text or quoted text
                case eTokenText:
                case eTokenQuotedText:
                case eTokenEquals:
                    if (!lpszText)
                    {
                        lpszText = token.pStr;
                    }                    

                    break;

                // If we found a start tag '<' and declarations '<?'
                case eTokenTagStart:
                case eTokenDeclaration:

                    // Cache whether this new element is a declaration or not
                    nDeclaration = type == eTokenDeclaration;

                    // If we have node text then add this to the element
                    if (lpszText)
                    {
                        cbTemp = (int)(token.pStr - lpszText);
                        FindEndOfText(lpszText, &cbTemp);
                        AddText(stringDup(lpszText,cbTemp));
                        lpszText = NULL;
                    }

                    // Find the name of the tag
                    token = GetNextToken(pXML, &cbToken, &type);

                    // Return an error if we couldn't obtain the next token or
                    // it wasnt text
                    if (type != eTokenText)
                    {
                        pXML->error = eXMLErrorMissingTagName;
                        return FALSE;
                    }

                    // If we found a new element which is the same as this 
                    // element then we need to pass this back to the caller..

#ifdef APPROXIMATE_PARSING 
                    if (d->lpszName && 
                        myTagCompare(d->lpszName, token.pStr) == 0)
                    {
                        // Indicate to the caller that it needs to create a 
                        // new element.
                        pXML->lpNewElement = token.pStr;
                        pXML->cbNewElement = cbToken;
                        return TRUE;
                    } else
#endif
                    {
                        // If the name of the new element differs from the name of
                        // the current element we need to add the new element to 
                        // the current one and recurse
                        pNew = AddChild(stringDup(token.pStr,cbToken), nDeclaration);

                        while (!pNew.isEmpty())
                        {
                            // Callself to process the new node.  If we return 
                            // FALSE this means we dont have any more 
                            // processing to do...

                            if (!pNew.ParseXMLElement(pXML))
                            {
                                d->pOrder=(int*)myRealloc(d->pOrder,
                                                          nElement(),
                                                          memoryIncrease*3,
                                                          sizeof(int));
                                d->pChild=(XMLNode*)myRealloc(d->pChild,
                                                              d->nChild,
                                                              memoryIncrease,
                                                              sizeof(XMLNode));
                                if (d->nAttribute > 0){
                                  d->pAttribute=(XMLAttribute*)
                                    myRealloc(d->pAttribute,
                                              d->nAttribute,
                                              memoryIncrease,
                                              sizeof(XMLAttribute));

                                }

                                if (d->nText > 0) {

                                  d->pText=(LPCTSTR*)myRealloc(d->pText,
                                                               d->nText,
                                                               memoryIncrease,
                                                               sizeof(LPTSTR));

                                }

                                if (d->nClear > 0) {

                                  d->pClear=(XMLClear *)myRealloc(d->pClear,
                                                                  d->nClear,
                                                                  memoryIncrease,
                                                                  sizeof(XMLClear));

                                }

                                return FALSE;
                            }
                            else
                            {
                                // If the call to recurse this function 
                                // evented in a end tag specified in XML then 
                                // we need to unwind the calls to this 
                                // function until we find the appropriate node 
                                // (the element name and end tag name must 
                                // match)
                                if (pXML->cbEndTag)
                                {
                                    // If we are back at the root node then we 
                                    // have an unmatched end tag
                                    if (!d->lpszName)
                                    {
                                        pXML->error=eXMLErrorUnmatchedEndTag;
                                        return FALSE;
                                    }

                                    // If the end tag matches the name of this 
                                    // element then we only need to unwind 
                                    // once more...

                                    if (myTagCompare(d->lpszName, pXML->lpEndTag)==0)
                                    {                                    
                                        pXML->cbEndTag = 0;
                                    }

                                    return TRUE;
                                } else 
                                    if (pXML->cbNewElement)
                                    {
                                        // If the call indicated a new element is to 
                                        // be created on THIS element.

                                        // If the name of this element matches the
                                        // name of the element we need to create 
                                        // then we need to return to the caller 
                                        // and let it process the element.

                                        if (myTagCompare(d->lpszName, pXML->lpNewElement)==0)
                                        {
                                            return TRUE;
                                        }

                                        // Add the new element and recurse
                                        pNew = AddChild(stringDup(pXML->lpNewElement,pXML->cbNewElement), FALSE);
                                        pXML->cbNewElement = 0;
                                    }
                                    else 
                                    {
                                        // If we didn't have a new element to create
                                        pNew = emptyXMLNode;

                                    }
                            }
                        }
                    }
                    break;

                // If we found an end tag
                case eTokenTagEnd:

                    // If we have node text then add this to the element
                    if (lpszText)
                    {
                        cbTemp = (int)(token.pStr - lpszText);
                        FindEndOfText(lpszText, &cbTemp);
                        AddText(fromXMLString(lpszText,cbTemp));
                        lpszText = NULL;
                    }
                    
                    // Find the name of the end tag
                    token = GetNextToken(pXML, &cbTemp, &type);                    

                    // The end tag should be text
                    if (type != eTokenText)
                    {
                        pXML->error = eXMLErrorMissingEndTagName;
                        return FALSE;
                    }
                    lpszTemp = token.pStr;

                    // After the end tag we should find a closing tag
                    token = GetNextToken(pXML, &cbToken, &type);
                    if (type != eTokenCloseTag)
                    {
                        pXML->error = eXMLErrorMissingEndTagName;
                        return FALSE;
                    }

                    // We need to return to the previous caller.  If the name 
                    // of the tag cannot be found we need to keep returning to
                    // caller until we find a match
                    if (myTagCompare(d->lpszName, lpszTemp) != 0)
                    {
                        pXML->lpEndTag = lpszTemp;
                        pXML->cbEndTag = cbTemp;
                    }

                    // Return to the caller
                    return TRUE;

                // If we found a clear (unformatted) token
                case eTokenClear:
                    // If we have node text then add this to the element
                    if (lpszText)
                    {
                        cbTemp = (int)(token.pStr - lpszText);
                        FindEndOfText(lpszText, &cbTemp);
                        AddText(stringDup(lpszText,cbTemp));
                        lpszText = NULL;
                    }

                    if (!ParseClearTag(pXML, token.pClr))
                    {
                        return FALSE;
                    }
                    break;

                // Errors...
                case eTokenCloseTag:          /* '>'         */
                case eTokenShortHandClose:    /* '/>'        */
                    pXML->error = eXMLErrorUnexpectedToken;
                    return FALSE;
                default:
                    break;
                }
                break;

            // If we are inside a tag definition we need to search for attributes
            case eInsideTag:

                // Check what part of the attribute (name, equals, value) we 
                // are looking for.
                switch(attrib) 
                {
                // If we are looking for a new attribute
                case eAttribName:

                    // Check what the current token type is
                    switch(type)
                    {
                    // If the current type is text...
                    // Eg.  'attribute'
                    case eTokenText:
                        // Cache the token then indicate that we are next to 
                        // look for the equals
                        lpszTemp = token.pStr;
                        cbTemp = cbToken;
                        attrib = eAttribEquals;
                        break;

                    // If we found a closing tag...
                    // Eg.  '>'
                    case eTokenCloseTag:
                        // We are now outside the tag
                        status = eOutsideTag;
                        break;

                    // If we found a short hand '/>' closing tag then we can 
                    // return to the caller
                    case eTokenShortHandClose:
                        return TRUE;

                    // Errors...
                    case eTokenQuotedText:    /* '"SomeText"'   */
                    case eTokenTagStart:      /* '<'            */
                    case eTokenTagEnd:        /* '</'           */
                    case eTokenEquals:        /* '='            */
                    case eTokenDeclaration:   /* '<?'           */
                    case eTokenClear:
                        pXML->error = eXMLErrorUnexpectedToken;
                        return FALSE;
                    default: break;
                    }
                    break;

                // If we are looking for an equals
                case eAttribEquals:
                    // Check what the current token type is
                    switch(type)
                    {
                    // If the current type is text...
                    // Eg.  'Attribute AnotherAttribute'
                    case eTokenText:
                        // Add the unvalued attribute to the list
                        AddAttribute(stringDup(lpszTemp,cbTemp), NULL);
                        // Cache the token then indicate.  We are next to 
                        // look for the equals attribute
                        lpszTemp = token.pStr;
                        cbTemp = cbToken;
                        break;

                    // If we found a closing tag 'Attribute >' or a short hand 
                    // closing tag 'Attribute />'
                    case eTokenShortHandClose:
                    case eTokenCloseTag:
                        // If we are a declaration element '<?' then we need
                        // to remove extra closing '?' if it exists
                        if (d->isDeclaration && 
                            (lpszTemp[cbTemp-1]) == _T('?'))
                        {
                            cbTemp--;
                        }

                        if (cbTemp)
                        {
                            // Add the unvalued attribute to the list
                            AddAttribute(stringDup(lpszTemp,cbTemp), NULL);
                        }

                        // If this is the end of the tag then return to the caller
                        if (type == eTokenShortHandClose)
                        {
                            return TRUE;
                        }

                        // We are now outside the tag
                        status = eOutsideTag;
                        break;

                    // If we found the equals token...
                    // Eg.  'Attribute ='
                    case eTokenEquals:
                        // Indicate that we next need to search for the value
                        // for the attribute
                        attrib = eAttribValue;
                        break;

                    // Errors...
                    case eTokenQuotedText:    /* 'Attribute "InvalidAttr"'*/
                    case eTokenTagStart:      /* 'Attribute <'            */
                    case eTokenTagEnd:        /* 'Attribute </'           */
                    case eTokenDeclaration:   /* 'Attribute <?'           */
                    case eTokenClear:
                        pXML->error = eXMLErrorUnexpectedToken;
                        return FALSE;
                    default: break;
                    }
                    break;

                // If we are looking for an attribute value
                case eAttribValue:
                    // Check what the current token type is
                    switch(type)
                    {
                    // If the current type is text or quoted text...
                    // Eg.  'Attribute = "Value"' or 'Attribute = Value' or
                    // 'Attribute = 'Value''.
                    case eTokenText:
                    case eTokenQuotedText:
                        // If we are a declaration element '<?' then we need
                        // to remove extra closing '?' if it exists
                        if (d->isDeclaration && 
                            (token.pStr[cbToken-1]) == _T('?'))
                        {
                            cbToken--;
                        }

                        if (cbTemp)
                        {
                            // Add the valued attribute to the list
                            if (type==eTokenQuotedText) 
                                { token.pStr++; cbToken-=2; }
                            AddAttribute(stringDup(lpszTemp,cbTemp),fromXMLString(token.pStr,cbToken));
                        }

                        // Indicate we are searching for a new attribute
                        attrib = eAttribName;
                        break;

                    // Errors...
                    case eTokenTagStart:        /* 'Attr = <'          */
                    case eTokenTagEnd:          /* 'Attr = </'         */
                    case eTokenCloseTag:        /* 'Attr = >'          */
                    case eTokenShortHandClose:  /* "Attr = />"         */
                    case eTokenEquals:          /* 'Attr = ='          */
                    case eTokenDeclaration:     /* 'Attr = <?'         */
                    case eTokenClear:
                        pXML->error = eXMLErrorUnexpectedToken;
                        return FALSE;
                        break;
                    default: break;
                    }
                }
            }
        }
        // If we failed to obtain the next token
        else 
        {
            return FALSE;    
        }
    }
}

// Count the number of lines and columns in an XML string.
static void CountLinesAndColumns(LPCTSTR lpXML, int nUpto, XMLResults *pResults)
{
    TCHAR ch;
    int n;

    LKASSERT(lpXML);    
    LKASSERT(pResults);

    pResults->nLine = 1;
    pResults->nColumn = 1;
    for(n=0; n<nUpto; n++)
    {
        ch = lpXML[n];
        LKASSERT(ch);
        if (ch == _T('\n'))
        {
            pResults->nLine++;
            pResults->nColumn = 1;
        }
        else pResults->nColumn++;
    }
}

// Parse XML and return the root element.
XMLNode XMLNode::parseString(LPCTSTR lpszXML, LPCTSTR tag, 
			     XMLResults *pResults)
{
    if (!lpszXML) 
    {
        if (pResults)
        {
            pResults->error=eXMLErrorNoElements;
            pResults->nLine=0;
            pResults->nColumn=0;
        }
        return emptyXMLNode;
    }
    static struct ClearTag tags[] =
    {
        {    _T("<![CDATA["),    _T("]]>")       },
        {    _T("<PRE>"),        _T("</PRE>")    },
        {    _T("<Script>"),     _T("</Script>") },
        {    _T("<!--"),         _T("-->")       },
        {    _T("<!DOCTYPE"),    _T(">")         },
        {    NULL,               NULL            }
    };
    enum XMLError error;
    XMLNode xnode(NULL,NULL,FALSE);
    struct XML xml={ NULL, 0, eXMLErrorNone, NULL, 0, NULL, 0, TRUE , NULL};

    xml.lpXML = lpszXML;
    xml.pClrTags = tags;

    // Create header element
    xnode.ParseXMLElement(&xml); // 100102 CRASH here
    error = xml.error;

    // If an error occurred
    if (error == eXMLErrorNone)
    {
        if (tag&&_tcslen(tag))
        {
            XMLNode nodeTmp;
            int i=0;
            while (i<xnode.nChildNode())
            {
                nodeTmp=xnode.getChildNode(i); 
                if (_tcsicmp(nodeTmp.getName(),tag)==0) break;
                if (nodeTmp.isDeclaration()) { xnode=nodeTmp; i=0; } else i++;
            }
            if (i<0)
            {
                if (pResults)
                {
                    pResults->error=eXMLErrorTagNotFound;
                    pResults->nLine=0;
                    pResults->nColumn=0;
                }
                return emptyXMLNode;
            }
            xnode=nodeTmp;
        }
    } else
    {
        // Cleanup: this will destroy all the nodes
        xnode = emptyXMLNode;
    }


    // If we have been given somewhere to place results
    if (pResults)
    {
        pResults->error = error;

        // If we have an error
        if (error != eXMLErrorNone)
        {
            // Find which line and column it starts on.
            CountLinesAndColumns(xml.lpXML, xml.nIndex, pResults);
        }
    }
    return xnode;
}

XMLNode XMLNode::parseFile(const TCHAR *filename, LPCTSTR tag, XMLResults *pResults)
{
    FILE *f=_tfopen(filename, _T("rb"));
    if (f==NULL)
    {
        if (pResults)
        {
            pResults->error=eXMLErrorFileNotFound;
            pResults->nLine=0;
            pResults->nColumn=0;
        }
        return emptyXMLNode;
    }
    fseek(f,0,SEEK_END);
    int l=ftell(f);
    fseek(f,0,SEEK_SET);
    char *buf=(char*)malloc(l+1);
    LKASSERT(buf);
    fread(buf,l,1,f);
    fclose(f);
    buf[l]=0;
    l++;     // 20060515:sgi make shure MultiByteToWideChar also translate the terminating char!!!
#if defined(WIN32) || defined(UNDER_CE)
#ifdef _UNICODE
#if !defined(UNDER_CE)
    if (!IsTextUnicode(buf,mmin(l,10000),NULL))
    {
#endif
        LPTSTR b2=(LPTSTR)malloc(l*2+2);
        LKASSERT(b2);
        MultiByteToWideChar(CP_ACP,          // code page
                            MB_PRECOMPOSED,  // character-type options
                            buf,             // string to map
                            l,               // number of bytes in string
                            b2,              // wide-character buffer
                            l*2+2);          // size of buffer
        free(buf);
        buf=(char*)b2;
#if !defined(UNDER_CE)
    }
#endif
#else
    if (IsTextUnicode(buf,mmin(l,10000),NULL))
    {
        l>>=1;
        LPTSTR b2=(LPTSTR)malloc(l+2);
        LKASSERT(b2);
        WideCharToMultiByte(CP_ACP,                      // code page
                            0,                           // performance and mapping flags
                            (const WCHAR*)buf,           // wide-character string
                            l,                           // number of chars in string
                            b2,                          // buffer for new string
                            l+2,                         // size of buffer
                            NULL,                        // default for unmappable chars
                            NULL                         // set when default char used
                           );
        free(buf);
        buf=(char*)b2;
    }
#endif
#endif

    XMLNode x=parseString((LPTSTR)buf,tag,pResults);
    free(buf);
    return x;
}

XMLNode XMLNode::openFileHelper(const TCHAR *lpszXML, LPCTSTR tag)
{
    XMLResults pResults;
    XMLNode::GlobalError = false;
    XMLNode xnode=XMLNode::parseFile(lpszXML, tag, &pResults);
    if (pResults.error != eXMLErrorNone)
    {
#ifdef DEBUG
        _tprintf(
            _T("XML Parsing error inside file '%s'.\nError: %s\nAt line %i, column %i.\n"),
            lpszXML, XMLNode::getError(pResults.error),pResults.nLine,pResults.nColumn);
        if (pResults.error==eXMLErrorTagNotFound)
        {
            _tprintf(_T("Tag is '%s'.\n"), tag);
        }
#endif
	XMLNode::GlobalError = true;
	// was exit(255);
    }
    return xnode;
}


XMLNodeContents XMLNode::enumContents(int i)
{ 
    XMLNodeContents c;
    if (!d) { c.type=eNodeNULL; return c; }
    c.type=(XMLElementType)(d->pOrder[i]&3);
    i=(d->pOrder[i])>>2;
    switch (c.type)
    {
        case eNodeChild:     c.child = d->pChild[i];      break;
        case eNodeAttribute: c.attrib= d->pAttribute[i];  break;
        case eNodeText:      c.text  = d->pText[i];       break;
        case eNodeClear:     c.clear = d->pClear[i];      break;
        default: break;
    }
    return c;
}

// private:
void *XMLNode::enumContent(XMLNodeData *pEntry, int i, XMLElementType *nodeType)
{
    XMLElementType j=(XMLElementType)(pEntry->pOrder[i]&3);
    *nodeType=j;
    i=(pEntry->pOrder[i])>>2;
    switch (j)
    {
    case eNodeChild:      return pEntry->pChild[i].d;
    case eNodeAttribute:  return pEntry->pAttribute+i;
    case eNodeText:       return (void*)(pEntry->pText[i]);
    case eNodeClear:      return pEntry->pClear+i;
    default: break;
    }
    return NULL;
}

// private:
int XMLNode::nElement(XMLNodeData *pEntry)
{
    return pEntry->nChild+pEntry->nText+pEntry->nClear+pEntry->nAttribute;
}

static inline void charmemset(LPTSTR dest,TCHAR c,int l) { while (l--) *(dest++)=c; }
// private:
// Creates an user friendly XML string from a given element with 
// appropriate white space and carriage returns.
//
// This recurses through all subnodes then adds contents of the nodes to the
// string.
int XMLNode::CreateXMLStringR(XMLNodeData *pEntry, LPTSTR lpszMarker, int nFormat)
{
    int nResult = 0;
    int cb;
    int cbElement;
    int nIndex;
    int nChildFormat=-1;
    int bHasChildren=FALSE;
    int i;
    XMLAttribute * pAttr;

    LKASSERT(pEntry);

#define LENSTR(lpsz) (lpsz ? _tcslen(lpsz) : 0)

    // If the element has no name then assume this is the head node.
    cbElement = (int)LENSTR(pEntry->lpszName);

    if (cbElement)
    {
        // "<elementname "
        cb = nFormat == -1 ? 0 : nFormat;

        if (lpszMarker)
        {
            if (cb) charmemset(lpszMarker, INDENTCHAR, sizeof(TCHAR)*cb);
            nResult = cb;
            lpszMarker[nResult++]=_T('<');
            if (pEntry->isDeclaration) lpszMarker[nResult++]=_T('?');
            _tcscpy(&lpszMarker[nResult], pEntry->lpszName);
            nResult+=cbElement;
            lpszMarker[nResult++]=_T(' ');

        } else 
        {
            nResult+=cbElement+2+cb;
            if (pEntry->isDeclaration) nResult++;
        }
        
        // Enumerate attributes and add them to the string
        nIndex = pEntry->nAttribute; pAttr=pEntry->pAttribute;
        for (i=0; i<nIndex; i++)
        {
            // "Attrib
            cb = (int)LENSTR(pAttr->lpszName);
            if (cb)
            {
                if (lpszMarker) _tcscpy(&lpszMarker[nResult], pAttr->lpszName);
                nResult += cb;
                // "Attrib=Value "
                cb = (int)lengthXMLString(pAttr->lpszValue);
                if (cb)
                {
                    if (lpszMarker)
                    {
                        lpszMarker[nResult]=_T('=');
                        lpszMarker[nResult+1]=_T('"');
                        toXMLString(&lpszMarker[nResult+2],pAttr->lpszValue);
                        lpszMarker[nResult+cb+2]=_T('"');
                    }
                    nResult+=cb+3;
                }
                if (lpszMarker) lpszMarker[nResult] = _T(' ');
                nResult++;              
            }
            pAttr++;
        }
        
        bHasChildren=(pEntry->nAttribute!=nElement(pEntry));
        if (pEntry->isDeclaration)
        {
            if (lpszMarker) 
            {
                lpszMarker[nResult-1]=_T('?');
                lpszMarker[nResult]=_T('>');
            }
            nResult++;
            if (nFormat!=-1)
            {
                if (lpszMarker) lpszMarker[nResult]=_T('\n');
                nResult++;
            }
        } else
            // If there are child nodes we need to terminate the start tag
            if (bHasChildren)
            {   
                if (lpszMarker) lpszMarker[nResult-1]=_T('>');
                if (nFormat!=-1)
                {
                    if (lpszMarker) lpszMarker[nResult]=_T('\n');
                    nResult++;
                }
            } else nResult--;
    }

    // Calculate the child format for when we recurse.  This is used to 
    // determine the number of spaces used for prefixes.
    if (nFormat!=-1)
    {
        if (cbElement) nChildFormat=nFormat+1;
        else nChildFormat=nFormat;
    }

    // Enumerate through remaining children
    nIndex = nElement(pEntry);
    XMLElementType nodeType;
    void *pChild;
    for (i=0; i<nIndex; i++)
    {
        pChild=enumContent(pEntry, i, &nodeType);
        switch(nodeType)
        {
        // Text nodes
        case eNodeText:
            // "Text"
            cb = (int)lengthXMLString((LPTSTR)pChild);
            if (cb)
            {
                if (nFormat!=-1)
                {
                    if (lpszMarker)
                    {
                        charmemset(&lpszMarker[nResult],INDENTCHAR,sizeof(TCHAR)*(nFormat + 1));                       
                        toXMLString(&lpszMarker[nResult+nFormat+1],(LPTSTR)pChild);
                        lpszMarker[nResult+nFormat+1+cb]=_T('\n');
                    }
                    nResult+=cb+nFormat+2;
                } else
                {
                    if (lpszMarker) toXMLString(&lpszMarker[nResult], (LPTSTR)pChild);
                    nResult += cb;
                }
            }
            break;


        // Clear type nodes
        case eNodeClear:
            // "OpenTag"
            cb = (int)LENSTR(((XMLClear*)pChild)->lpszOpenTag);
            if (cb)
            {
                if (nFormat!=-1)
                {
                    if (lpszMarker)
                    {
                        charmemset(&lpszMarker[nResult], INDENTCHAR, sizeof(TCHAR)*(nFormat + 1));
                        _tcscpy(&lpszMarker[nResult+nFormat+1], ((XMLClear*)pChild)->lpszOpenTag);
                    }
                    nResult+=cb+nFormat+1;
                }
                else
                {
                    if (lpszMarker)_tcscpy(&lpszMarker[nResult], ((XMLClear*)pChild)->lpszOpenTag);
                    nResult += cb;
                }               
            }

            // "OpenTag Value"
            cb = (int)LENSTR(((XMLClear*)pChild)->lpszValue);
            if (cb)
            {
                if (lpszMarker) _tcscpy(&lpszMarker[nResult], ((XMLClear*)pChild)->lpszValue);
                nResult += cb;
            }

            // "OpenTag Value CloseTag"
            cb = (int)LENSTR(((XMLClear*)pChild)->lpszCloseTag);
            if (cb)
            {
                if (lpszMarker) _tcscpy(&lpszMarker[nResult], ((XMLClear*)pChild)->lpszCloseTag);
                nResult += cb;
            }

            if (nFormat!=-1)
            {
                if (lpszMarker) lpszMarker[nResult] = _T('\n');
                nResult++;
            }
            break;

        // Element nodes
        case eNodeChild:

            // Recursively add child nodes
            nResult += CreateXMLStringR((XMLNodeData*)pChild, 
                lpszMarker ? lpszMarker + nResult : 0, nChildFormat);
            break;
        default: break;
        }
    }

    if ((cbElement)&&(!pEntry->isDeclaration))
    {
        // If we have child entries we need to use long XML notation for 
        // closing the element - "<elementname>blah blah blah</elementname>"
        if (bHasChildren)
        {
            // "</elementname>\0"
            if (lpszMarker)
            {
                if (nFormat != -1)
                {
                    if (nFormat)
                    {
                        charmemset(&lpszMarker[nResult], INDENTCHAR,sizeof(TCHAR)*nFormat);
                        nResult+=nFormat;
                    }
                }       

                _tcscpy(&lpszMarker[nResult], _T("</"));
                nResult += 2;
                _tcscpy(&lpszMarker[nResult], pEntry->lpszName);
                nResult += cbElement;
                
                if (nFormat == -1)
                {
                    _tcscpy(&lpszMarker[nResult], _T(">"));
                    nResult++;
                } else
                {
                    _tcscpy(&lpszMarker[nResult], _T(">\n"));
                    nResult+=2;
                }
            } else
            {
                if (nFormat != -1) nResult+=cbElement+4+nFormat;
                else nResult+=cbElement+3;
            }       
        } else 
        {
            // If there are no children we can use shorthand XML notation - 
            // "<elementname/>"
            // "/>\0"
            if (lpszMarker)
            {
                if (nFormat == -1)
                {
                    _tcscpy(&lpszMarker[nResult], _T("/>")); 
                    nResult += 2;
                }
                else
                {
                    _tcscpy(&lpszMarker[nResult], _T("/>\n")); 
                    nResult += 3;
                }
            }
            else
            {
                nResult += nFormat == -1 ? 2 : 3;
            }
        }
    }

    return nResult;
}

#undef LENSTR

// Create an XML string from the head element.
// @param       XMLElement * pHead      - head element
// @param       int nFormat             - 0 if no formatting is required 
//                                        otherwise nonzero for formatted text
//                                        with carriage returns and indentation.
// @param       int *pnSize             - [out] pointer to the size of the 
//                                        returned string not including the 
//                                        NULL terminator.
//
// @return      LPTSTR                  - Allocated XML string, you must free
//                                        this with free().
LPTSTR XMLNode::createXMLString(int nFormat, int *pnSize)
{
    if (!d) { if (pnSize) *pnSize=0; return NULL; }

    LPTSTR lpszResult = NULL;
    int cbStr;

    // Recursively Calculate the size of the XML string
    nFormat = nFormat ? 0 : -1;
    cbStr = CreateXMLStringR(d, 0, nFormat);
    LKASSERT(cbStr);
    // Alllocate memory for the XML string + the NULL terminator and
    // create the recursively XML string.
    lpszResult=(LPTSTR)malloc((cbStr+1)*sizeof(TCHAR));
    LKASSERT(lpszResult);
    CreateXMLStringR(d, lpszResult, nFormat);
    if (pnSize) *pnSize = cbStr;
    return lpszResult;
}

XMLNode::~XMLNode()
{
    destroyCurrentBuffer(d);
};

void XMLNode::destroyCurrentBuffer(XMLNodeData *d)
{
    if (!d) return;
    (d->ref_count) --;
    if (d->ref_count==0)
    {
        int i=0;

        if (d->pParent)
        {
            XMLNode *pa=d->pParent->d->pChild;
            while (((void*)(pa[i].d))!=((void*)d)) i++; 
            d->pParent->d->nChild--;
            memmove(pa+i,pa+i+1,(d->pParent->d->nChild-i)*sizeof(XMLNode));
            removeOrderElement(d->pParent->d,eNodeChild,i);
        }

        for(i=0; i<d->nChild; i++) { 
          d->pChild[i].d->pParent=NULL; 
          destroyCurrentBuffer(d->pChild[i].d); 
        }
        free(d->pChild);
        for(i=0; i<d->nText; i++) free((void*)d->pText[i]);
        free(d->pText);
        for(i=0; i<d->nClear; i++) free((void*)d->pClear[i].lpszValue);
        free(d->pClear);
        for(i=0; i<d->nAttribute; i++) 
        {
            free((void*)d->pAttribute[i].lpszName);
            if (d->pAttribute[i].lpszValue) free((void*)d->pAttribute[i].lpszValue);
        }
        free(d->pAttribute);
        free(d->pOrder);
        free((void*)d->lpszName);
        free(d);
    }
}

XMLNode& XMLNode::operator=( const XMLNode& A )
{
    // shallow copy
    if (this != &A)
    {
        destroyCurrentBuffer(d);
        d=A.d;
        if (d) (d->ref_count) ++ ;
    }
    return *this;
}

XMLNode::XMLNode(const XMLNode &A)
{
    // shallow copy
    d=A.d;
    if (d) (d->ref_count)++ ;
}

int XMLNode::nChildNode(LPCTSTR name)
{
    if (!d) return 0; 
    int i,j=0,n=d->nChild;
    XMLNode *pc=d->pChild;
    for (i=0; i<n; i++)
    {
        if (_tcsicmp(pc->d->lpszName, name)==0) j++;
        pc++;
    }
    return j;
}

XMLNode XMLNode::getChildNode(LPCTSTR name, int *j)
{
    if (!d) return emptyXMLNode; 
    int i=0,n=d->nChild;
    if (j) i=*j;
    XMLNode *pc=d->pChild+i;
    for (; i<n; i++)
    {
        if (_tcsicmp(pc->d->lpszName, name)==0) 
        {
            if (j) *j=i+1;
            return *pc;
        }
        pc++;
    }
    return emptyXMLNode;
}

XMLNode XMLNode::getChildNode(LPCTSTR name, int j)
{
    if (!d) return emptyXMLNode; 
    int i=0; 
    while (j-->0) getChildNode(name,&i);
    return getChildNode(name,&i);
}

// Find an attribute on an node.
LPCTSTR XMLNode::getAttribute(LPCTSTR lpszAttrib, int *j)
{
    if (!d) return NULL; 
    int i=0,n=d->nAttribute;
    if (j) i=*j; 
    XMLAttribute *pAttr=d->pAttribute+i;
    for (; i<n; i++)
    {
        if (_tcsicmp(pAttr->lpszName, lpszAttrib)==0) 
        {
            if (j) *j=i+1;
            return pAttr->lpszValue;
        }
        pAttr++;
    }
    return NULL;
}

char XMLNode::isAttributeSet(LPCTSTR lpszAttrib)
{
    if (!d) return FALSE; 
    int i,n=d->nAttribute;
    XMLAttribute *pAttr=d->pAttribute;
    for (i=0; i<n; i++)
    {
        if (_tcsicmp(pAttr->lpszName, lpszAttrib)==0) 
        {
            return TRUE;
        }
        pAttr++;
    }
    return FALSE;
}

LPCTSTR XMLNode::getAttribute(LPCTSTR name, int j)
{
    if (!d) return NULL; 
    int i=0; 
    while (j-->0) getAttribute(name,&i);
    return getAttribute(name,&i);
}

LPCTSTR XMLNode::getName(){ if (!d) return NULL; return d->lpszName;   }
int XMLNode::nText()      { if (!d) return 0; return d->nText;      }
int XMLNode::nChildNode() { if (!d) return 0; return d->nChild;     }
int XMLNode::nAttribute() { if (!d) return 0; return d->nAttribute; }
int XMLNode::nClear()     { if (!d) return 0; return d->nClear;     }
XMLClear     XMLNode::getClear     (int i) { if (!d) return emptyXMLClear;     if (i>=d->nClear    ) return emptyXMLClear;     return d->pClear[i];     }
XMLAttribute XMLNode::getAttribute (int i) { if (!d) return emptyXMLAttribute; if (i>=d->nAttribute) return emptyXMLAttribute; return d->pAttribute[i]; }
LPCTSTR      XMLNode::getText      (int i) { if (!d) return NULL;              if (i>=d->nText     ) return NULL;              return d->pText[i];      }
XMLNode      XMLNode::getChildNode (int i) { if (!d) return emptyXMLNode;      if (i>=d->nChild    ) return emptyXMLNode;      return d->pChild[i];     }
char         XMLNode::isDeclaration(     ) { if (!d) return (char)0;                 return d->isDeclaration; }
char         XMLNode::isEmpty      (     ) { return (d==NULL); }
int          XMLNode::nElement     (     ) { if (!d) return 0; return d->nChild+d->nText+d->nClear+d->nAttribute; }


