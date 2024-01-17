#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string>
class tgaTool
{
public:

#pragma pack(1)
	typedef struct {
		char  idlength;
		char  colourmaptype;
		char  datatypecode;
		short int colourmaporigin;
		short int colourmaplength;
		char  colourmapdepth;
		short int x_origin;
		short int y_origin;
		short width;
		short height;
		char  bitsperpixel;
		char  imagedescriptor;
	} header;
#pragma pack()

	static inline int loadFrameFromTGA(const char * _fileName, uint8_t *pDestBuffer)
	{
		FILE* fp = nullptr;
		fopen_s(&fp, _fileName, "rb");
		if (fp == nullptr)
			return -1;

		tgaTool::header h;
		if (fread(&h, sizeof(tgaTool::header), 1, fp) != 1)
		{
			fclose(fp);
			return -2;
		}

		//checkHeader(Config->getVideoWidth(), Config->getVideoHeight(), &header);
		
		unsigned char* buffer = pDestBuffer;
		if (fread(buffer, h.width * h.height * 4, 1, fp) != 1)
		{
			fclose(fp);
			return -3;
		}

		if (h.imagedescriptor == 0x08)
		{
			const int lineSize = h.width * 4;
			unsigned char * tmp = new unsigned char(3840 * 4);

			for (int i = 0; i < (h.height / 2); i++)
			{
				memcpy(tmp, &buffer[i * lineSize], lineSize);
				memcpy(&buffer[i * lineSize], &buffer[(h.height - 1 - i) * lineSize], lineSize);
				memcpy(&buffer[(h.height - 1 - i) * lineSize], tmp, lineSize);
			}
			free(tmp);
		}

		fclose(fp);
		return 0;
	}
};
