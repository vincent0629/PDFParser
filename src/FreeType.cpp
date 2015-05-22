#include "FreeType.h"
#include "InputStream.h"

class FreeTypeHelper : public FreeType
{
public:
	FreeTypeHelper()
	{
	}

	~FreeTypeHelper()
	{
	}
};
static FreeTypeHelper g_Instance;

static unsigned long Stream_ReadFunc(FT_Stream stream, unsigned long offset, unsigned char*  buffer, unsigned long count)
{
	InputStream *pSource;

	pSource = (InputStream *)stream->descriptor.pointer;
	pSource->Seek(offset, SEEK_SET);
	return pSource->Read(buffer, count);
}

static void Stream_CloseFunc(FT_Stream stream)
{
	delete stream;
}

FreeType::FreeType()
{
	FT_Init_FreeType(&m_ft);
}

FreeType::~FreeType()
{
	FT_Done_FreeType(m_ft);
}

FT_Face FreeType::OpenFace(InputStream *pStream)
{
	FT_Stream stream;
	FT_Open_Args args;
	FT_Face face;

	stream = new FT_StreamRec;  // stream will be deleted in Stream_CloseFunc
	stream->size = pStream->Available();
	stream->descriptor.pointer = pStream;
	stream->read = Stream_ReadFunc;
	stream->close = Stream_CloseFunc;
	args.flags = FT_OPEN_STREAM;
	args.stream = stream;
	if (FT_Open_Face(g_Instance.m_ft, &args, 0, &face) == 0)
		return face;
	return NULL;
}
