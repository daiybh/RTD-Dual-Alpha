
#include <iostream>
#include <tga.h>
namespace MyRGB {

	unsigned char mapRange(unsigned char value, unsigned char oldMin, unsigned char oldMax,
		unsigned char newMin, unsigned char newMax) {
		return ((value - oldMin) * (newMax - newMin) / (oldMax - oldMin)) + newMin;
	}
	void convertAlpha(uint8_t* pSrc)
	{
		uint8_t* pY = pSrc;
		uint8_t* pU = pY + 1920 * 1080;
		uint8_t* pV = pU + 1920 * 1080 / 2;
		for (int i = 0; i < (1920 * 1080 / 2); i++)
		{
			int p1 = *pY = mapRange(*pY, 16, 235, 0, 255);		pY++;
			int p2 = *pY = mapRange(*pY, 16, 235, 0, 255);		*pY++;
			*pU = mapRange(*pU, 0, 128, 0, 255);
			*pU = (p1 + p2) / 2;
			pU++;
			*pV = mapRange(*pV, 0, 128, 0, 255);
			*pV = (p1 + p2) / 2;
			pV++;
		}
	}
	void key_Y_WithPos(unsigned char* _output, unsigned char* _inputA, unsigned char* _inputB,
		unsigned char* _key, size_t _pitch, unsigned int _pos)
	{
		for (int h = 0; h < 1080; h++)
		{
			for (int w = 0; w < _pitch; w++)
			{
				unsigned int key = _key[h * _pitch + w];
				unsigned int inputA = _inputA[h * _pitch + w];
				unsigned int inputB = _inputB[h * _pitch + w];
				unsigned int key_a = key * _pos / 255;
				unsigned int key_b = 0xff - key_a;

				_output[h * _pitch + w] = inputA * key_b / 255 + inputB * key_a / 255;
			}
		}
	}

	void Kernel_mix_withMask(unsigned char* _output, unsigned char* _inputA, unsigned char* _inputB, size_t _pitch,
		unsigned char* _mask)
	{
		for (int h = 0; h < 1080; h++)
		{
			for (int w = 0; w < _pitch; w++)
			{
				unsigned int key = _mask[h * _pitch + w];
				unsigned int inputA = _inputA[h * _pitch + w];
				unsigned int inputB = _inputB[h * _pitch + w];
				unsigned int key_a = key;
				unsigned int key_b = 0xff - key_a;

				_output[h * _pitch + w] = inputA * key_b / 255 + inputB * key_a / 255;

			}
		}
	}
	void cpu_mixwithMask(uint8_t* _mask, uint8_t* AFframe, uint8_t* BFrame, uint8_t* _dest)
	{
		uint8_t* pDestY = _dest;
		uint8_t* pDestU = pDestY + 1920 * 1080;
		uint8_t* pDestV = pDestU + 1920 * 1080 / 2;

		uint8_t* pAY = AFframe;
		uint8_t* pAU = pAY + 1920 * 1080;
		uint8_t* pAV = pAU + 1920 * 1080 / 2;


		uint8_t* pBY = BFrame;
		uint8_t* pBU = pBY + 1920 * 1080;
		uint8_t* pBV = pBU + 1920 * 1080 / 2;

		uint8_t* pMaskY = _mask;
		uint8_t* pMaskU = pMaskY + 1920 * 1080;
		uint8_t* pMaskV = pMaskU + 1920 * 1080 / 2;


		Kernel_mix_withMask(pDestY, pAY, pBY, 1920, pMaskY);
		Kernel_mix_withMask(pDestU, pAU, pBU, 1920 / 2, pMaskU);
		Kernel_mix_withMask(pDestV, pAV, pBV, 1920 / 2, pMaskV);

	}
	void cpu_keywithPos(uint8_t* AFframe, uint8_t* BFrame, uint8_t* key, uint8_t* dest, int pos)
	{
		uint8_t* pDestY = dest;
		uint8_t* pDestU = pDestY + 1920 * 1080;
		uint8_t* pDestV = pDestU + 1920 * 1080 / 2;

		uint8_t* pAY = AFframe;
		uint8_t* pAU = pAY + 1920 * 1080;
		uint8_t* pAV = pAU + 1920 * 1080 / 2;


		uint8_t* pBY = BFrame;
		uint8_t* pBU = pBY + 1920 * 1080;
		uint8_t* pBV = pBU + 1920 * 1080 / 2;

		uint8_t* pKY = key;
		uint8_t* pKU = pKY + 1920 * 1080;
		uint8_t* pKV = pKU + 1920 * 1080 / 2;

		memset(pDestY, 0, 1920 * 1080 * 2);
		key_Y_WithPos(pDestY, pAY, pBY, pKY, 1920, pos);
		key_Y_WithPos(pDestU, pAU, pBU, pKU, 1920 / 2, pos);
		key_Y_WithPos(pDestV, pAV, pBV, pKV, 1920 / 2, pos);
	}
	inline void convert_TGArgb2yuv422_yuv422(unsigned char* tga, int nWidth, int nHeight, bool _filter, uint8_t* dest, uint8_t* destA)
	{
		unsigned char* source = (unsigned char*)tga;

		uint8_t* pFILL_Y = dest;
		uint8_t* pFILL_U = pFILL_Y + nWidth * nHeight;
		uint8_t* pFILL_V = pFILL_U + nWidth * nHeight / 2;

		uint8_t* pAlpha_Y = destA;
		uint8_t* pAlpha_U = pAlpha_Y + nWidth * nHeight;
		uint8_t* pAlpha_V = pAlpha_U + nWidth * nHeight / 2;


		for (int i = 0; i < (nWidth * nHeight / 2); i++)
		{
			auto pRGB_TO_YUVA = [&](uint8_t& dY, uint8_t& dU, uint8_t& dV, uint8_t& dA) {
				const float B = (float)*source++;
				const float G = (float)*source++;
				const float R = (float)*source++;
				dA = *source++;

				const float dR = R;
				const float dG = G;
				const float dB = B;

				dY = lrint(((dR * 2126 + dG * 7152 + dB * 722) / 10000 * 219) / 255 + 16);
				dU = lrint((112 * dB - 86 * dG - 26 * dR) / 255 + 128);
				dV = (112 * dR - 10 * dB - 102 * dG) / 255 + 128;

			};
			uint8_t Y1, Y2;
			uint8_t U1, U2;
			uint8_t V1, V2;
			uint8_t A1, A2;
			pRGB_TO_YUVA(Y1, U1, V1, A1);

			pRGB_TO_YUVA(Y2, U2, V2, A2);

			unsigned short A1_and_A2 = (unsigned short)A1 + (unsigned short)A2;

			*pFILL_U++ = ((unsigned short)U1 * A1 / 255 + (unsigned short)U2 * A2 / 255) / 2;
			*pFILL_Y++ = (unsigned short)Y1 * (unsigned short)A1 / 255;
			*pFILL_V++ = ((unsigned short)V1 * A1 / 255 + (unsigned short)V2 * A2 / 255) / 2;
			*pFILL_Y++ = (unsigned short)Y2 * (unsigned short)A2 / 255;

			*pAlpha_U++ = (A1_and_A2 / 2);
			*pAlpha_Y++ = A1;
			*pAlpha_V++ = (A1_and_A2 / 2);
			*pAlpha_Y++ = A2;
		}
	}
	
	void convUYVYtoYUV(const uint8_t* _src, uint8_t* &_dest, int nWidth, int nHeight)
	{
		
		uint8_t* pSrc = (uint8_t*)_src;
		const uint64_t Ysize = (uint64_t)nWidth*nHeight;
		const uint64_t UVsize = Ysize / 2;

		unsigned char* pDestY = _dest;
		unsigned char* pDestU = &pDestY[Ysize];
		unsigned char* pDestV = &pDestU[UVsize];

		for (int i = 0; i < UVsize; i++)
		{
			*pDestU++ = *pSrc++;
			*pDestY++ = *pSrc++;
			*pDestV++ = *pSrc++;
			*pDestY++ = *pSrc++;
		}
	}
}

void loadTga( int i, uint8_t* frameTGA, int width, int height, uint8_t* frameMASKYUV, uint8_t* frameTemp, 
	uint8_t* frameFILLYUV, uint8_t* frameKEYYUV)
{

	const int tgaFrameNum = 35;
	std::string rootPath_FILL = R"(E:\TGA\DualAlpha\KKD_FILL_)";
	std::string rootPath_KEY = R"(E:\TGA\DualAlpha\KKD_KEY_)";
	std::string rootPath_MASK = R"(E:\TGA\DualAlpha\KKD_MASK_)";
	//std::string rootPath_FILL = R"(E:\TGA\221 volet ralenti\L2_221_FILL_)";
	//std::string rootPath_KEY = R"(E:\TGA\221 volet ralenti\L2_221_KEY_)";
	//std::string rootPath_MASK = R"(E:\TGA\221 volet ralenti\L2_221_MASK_)";
	char tgaPath[1024];
	{//load tga and convert to yuv
		//load mask
		sprintf_s(tgaPath, R"(%s%05d.tga)", rootPath_MASK.data(), i %tgaFrameNum);
		printf("\n %d  %s", i, tgaPath);
		tgaTool::loadFrameFromTGA(tgaPath, frameTGA);
		MyRGB::convert_TGArgb2yuv422_yuv422(frameTGA, width, height, false, frameMASKYUV, frameTemp);

		//load fill
		sprintf_s(tgaPath, R"(%s%05d.tga)", rootPath_FILL.data(), i % tgaFrameNum);
		tgaTool::loadFrameFromTGA(tgaPath, frameTGA);
		MyRGB::convert_TGArgb2yuv422_yuv422(frameTGA, width, height, false, frameFILLYUV, frameTemp);
		//load key
		sprintf_s(tgaPath, R"(%s%05d.tga)", rootPath_KEY.data(), i % tgaFrameNum);
		tgaTool::loadFrameFromTGA(tgaPath, frameTGA);
		MyRGB::convert_TGArgb2yuv422_yuv422(frameTGA, width, height, false, frameKEYYUV, frameTemp);
	}
}

void doRTD() {
	char tgaPath[1024];
	const int tgaFrameNum = 35;
	int width = 1920;
	int height = 1080;
	uint8_t* frameTGA = new uint8_t[1920 * 1080 * 4]; 
	uint8_t* frameMASKYUV = new uint8_t[1920 * 1080 * 4];
	uint8_t* frameFILLYUV = new uint8_t[1920 * 1080 * 4];
	uint8_t* frameKEYYUV = new uint8_t[1920 * 1080 * 4];
	uint8_t* frameTemp = new uint8_t[1920 * 1080 * 4];
	uint8_t* frameDestYUV = new uint8_t[1920 * 1080 * 4];
	uint8_t* frameYUV1 = new uint8_t[1920 * 1080 * 4];
	uint8_t* frameYUV2 = new uint8_t[1920 * 1080 * 4];
	uint8_t* pUyvy1 = new uint8_t[1920 * 1080 * 4];

	FILE* fp1, * fp2, * fp_dest;
	fopen_s(&fp_dest, "c:\\logs\\mixALL1_1920x1080.yuv", "wb");
	fopen_s(&fp1, "d:\\1080i50.yuv", "rb");
	fopen_s(&fp2, "d:\\1080p5994.yuv", "rb");

	uint8_t* pU = frameYUV1 + width * height;
	for (int i = 1; i < tgaFrameNum*2; i++)
	{
		*pU = i;
		if (fread(pUyvy1, width*height*2, 1, fp1) != 1)
		{
			rewind(fp1);
			fread(pUyvy1, width * height * 2, 1, fp1);
		}
		MyRGB::convUYVYtoYUV(pUyvy1, frameYUV1, width, height);

		if (fread(pUyvy1, width * height * 2, 1, fp2) != 1)
		{
			rewind(fp2);
			fread(pUyvy1, width * height * 2, 1, fp2);
		}
		MyRGB::convUYVYtoYUV(pUyvy1, frameYUV2,width,height);

		//cpu ²Ù×÷
		loadTga(i, frameTGA, width, height, frameMASKYUV, frameTemp, frameFILLYUV, frameKEYYUV);

		MyRGB::convertAlpha(frameKEYYUV);		
		MyRGB::convertAlpha(frameMASKYUV);

		//mixwithMask
		MyRGB::cpu_mixwithMask(frameMASKYUV, frameYUV1, frameYUV2, frameTemp);
		MyRGB::cpu_keywithPos(frameTemp, frameFILLYUV, frameKEYYUV, frameDestYUV, 255);
		//fwrite(frameYUV1, width * height * 2, 1, fp_dest);                    //1
		//fwrite(frameYUV2, width * height * 2, 1, fp_dest);                    //1
		//fwrite(frameFILLYUV, width * height * 2, 1, fp_dest);                    //1
		//fwrite(frameKEYYUV, width * height * 2, 1, fp_dest);                    //1
		//fwrite(frameMASKYUV, width * height * 2, 1, fp_dest);                    //1
		//fwrite(frameTemp, width * height * 2, 1, fp_dest);                    //1		
		fwrite(frameDestYUV, width*height*2, 1, fp_dest);                    //1
		//if (i > 3)return;	
	}
}
int main(){
    std::cout << "Hello, World!" << std::endl;
	doRTD();

	printf("\n######OVER################\n");
    return 0;
}