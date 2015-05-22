#ifndef _PDF_FREETYPE_H_
#define _PDF_FREETYPE_H_

#include <ft2build.h>
#include FT_FREETYPE_H

class InputStream;

class FreeType
{
public:
	static FT_Face OpenFace(InputStream *pStream);

protected:
	FT_Library m_ft;

	FreeType();
	~FreeType();
};

#endif
