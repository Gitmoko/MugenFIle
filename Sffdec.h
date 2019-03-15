#ifndef SffdecH
#define SffdecH
#include <map>
#include <functional>
#include <iostream>
#include <fstream>
#include<iterator>

namespace{
	int place[4] = { 1, 0x100, 0x10000, 0x1000000 };
}


namespace Sffdec {

	struct Color {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t reserved;//pcx�ɂ͖����Abmp�ɂ͂���
		Color() {
			r = 0;
			g = 0;
			b = 0;
			reserved = 0;
		}
	};
	struct Palette {
		Color colors[256];

		Palette() :colors{} {}
	};

	constexpr int upperendian = 0x100;//�G���f�B�A���ɋC��t����

	template <class T, class Arr>
	int push(Arr& tempbmp, int& tempit, T data) {//data��byte��ɂ���tempbmp[tempit]�ɉ������ށB

		int size = sizeof(data);
		for (int i = 0; i < size; i++) {
			tempbmp[tempit] = (data >> 8 * i) % (0x100);
			tempit++;
		}
		return 0;
	}

	struct Pcxfileheader {
		uint8_t  Identifier;
		uint8_t  Version;
		uint8_t  Encoding;
		uint8_t  BitsPerPixel;
		uint16_t XStart;
		uint16_t YStart;
		uint16_t XEnd;//�d�v
		uint16_t YEnd;//
		uint16_t HorizontalResolution;
		uint16_t VerticalResolution;
		uint8_t  EGAPalette[48];
		uint8_t  Reserved1;
		uint8_t  NumBitPlanes;
		uint16_t BytesPerScanline;
		uint16_t PaletteType;
		uint16_t HorizontalScreenSize;
		uint16_t VerticalScreenSize;
		uint8_t  Reserved2[54];
	};

	//�w�b�_�S�̂łP�Q�W�o�C�g
	//�Ă������d�v�Ȃ̂͂������t�@�C��������

	struct BMPINFOHEADER {
		uint32_t  biSize;//40�ɂ���
		uint32_t   biWidth;//�K�X
		uint32_t   biHeight;//�K�X
		uint16_t   biPlanes;//�P�Ƃ���
		uint16_t   biBitCount;//�W�Ƃ���
		uint32_t  biCompression;//0�Ƃ���
		uint32_t  biSizeImage;//0�Ƃ���
		uint32_t   biXPelsPerMeter;//�ǂ��ł�����
		uint32_t   biYPelsPerMeter;//�ǂ��ł�����
		uint32_t  biClrUsed;//256�Ƃ���
		uint32_t  biClrImportant;//0�Ƃ���

		BMPINFOHEADER() {
			biSize = 40;


			biPlanes = 1;
			biBitCount = 8;
			biCompression = 0;
			biSizeImage = 0;
			biXPelsPerMeter = 0;
			biYPelsPerMeter = 0;
			biClrUsed = 256;
			biClrImportant = 0;
		}
	};
	struct BMPFILEHEADER {
		uint16_t    bfType;//char��'B''M'�o�C�g����
		uint32_t   bfSize;//�K�X
		uint16_t    bfReserved1;//0
		uint16_t    bfReserved2;//0
		uint32_t   bfOffBits;//0�ł�����(�{���\�t�g�ɂ���Ă͉�����̂Ō����ɂ��܂�)

		BMPFILEHEADER() {
			bfType = 'M' * 0x100 + 'B';

			bfReserved1 = 0;
			bfReserved2 = 0;

		}
	};



	inline uint8_t* Convert(uint8_t* (&pcxit), int& outputsizebuf, Palette* sharepalette, Palette* output_usedpalette, unsigned int* outputxsize, unsigned int* outputysize) {


		//�܂���pcx��ǂݍ���
		//�w�b�_�ǂݍ���
		if (uint8_t(*(pcxit++)) != 0x0A)//pcx�f�[�^�łȂ����
			return{};///////////
		for (int i = 0; i < 7; i++) {
			*(pcxit++);//�����܂ł�XEND�̐擪
		}

		int pcxwidth = uint8_t(*(pcxit++));
		pcxwidth += (uint8_t(*(pcxit++)))*upperendian + 1;//int pcxwidth = (uint8_t(*(pcxit++))) + uint8_t(*(pcxit++))*upperendian + 1;���̏ꍇ, debug�@�Ɓ@release�@�Ł@�v�Z���Ԃ��Ⴄ
		int pcxheight = (uint8_t(*(pcxit++)));
		pcxheight += uint8_t(*(pcxit++))*upperendian + 1;

		if (outputxsize != nullptr)
			*outputxsize = pcxwidth;
		if (outputysize != nullptr)
			*outputysize = pcxheight;

		for (int i = 0; i < 116; i++) {
			*(pcxit++);
		}//����Ńw�b�_�I��

		//�C���f�b�N�X�ǂݍ���rle���k�ɋC��t���悤

		std::vector<std::vector<uint8_t>> pcxindex(pcxheight, std::vector<uint8_t>(pcxwidth, 0));

		for (auto h = pcxindex.begin(); h != pcxindex.end(); h++) {
			for (auto w = (*h).begin(); w != (*h).end();) {
				int temp = uint8_t(*(pcxit++));
				if (temp > 0xC0) {//rle���k�Ȃ�
					uint8_t times = temp - 0xC0;
					uint8_t seq_index = uint8_t(*(pcxit++));
					for (int i = 0; i < times; i++) {
						(*w) = (seq_index);
						w++;
					}
				}
				else {
					(*w) = (temp);
					w++;
				}
			}
		}

		//�p���b�g�Ƃ̋��E0x0C��ǂݍ���
		if (uint8_t(*(pcxit++)) != 0x0C)//0x0C�ɂȂ��Ă�H
			return{};////////

		////�Ɨ��p���b�g�̏ꍇ,�p���b�g�ǂݍ���
		Palette* mypalette = nullptr;
		if (sharepalette == nullptr) {
			mypalette = new Palette();

			for (int i = 0; i < 256; i++) {
				if (i == 0) {//�C���f�b�N�X�O�Ԃ͓��߂Ƃ��Ĉ���.DxLib�̓f�t�H���g�ł�(0,0,0)�𓧉ߐF�ň�������
					mypalette->colors[i].r = 0;
					mypalette->colors[i].g = 0;
					mypalette->colors[i].b = 0;

					for (int i = 0; i < 3; i++)
						*(pcxit++);
				}
				else {
					mypalette->colors[i].r = uint8_t(*(pcxit++));
					mypalette->colors[i].g = uint8_t(*(pcxit++));
					mypalette->colors[i].b = uint8_t(*(pcxit++));
				}
			}
		}
		//�K�v�ɉ����Ďg�p�����p���b�g��Ԃ�
		if (output_usedpalette != nullptr) {
			if (sharepalette == nullptr) {
				for (int i = 0; i < 256; i++) {
					output_usedpalette->colors[i].r = mypalette->colors[i].r;
					output_usedpalette->colors[i].g = mypalette->colors[i].g;
					output_usedpalette->colors[i].b = mypalette->colors[i].b;
				}
			}
			else {
				for (int i = 0; i < 256; i++) {
					output_usedpalette->colors[i].r = sharepalette->colors[i].r;
					output_usedpalette->colors[i].g = sharepalette->colors[i].g;
					output_usedpalette->colors[i].b = sharepalette->colors[i].b;
				}
			}
		}

		/*if (non_comvert_ptob){
			//�C���f�b�N�X
			for (int h = bmptag.biHeight - 1; h != -1; h--){
				for (unsigned int w = 0; w < bmptag.biWidth; w++){
					push(bmpdata, it, pcxindex[h][w]);
				}
				for (int w = 0; w < resizeline; w++){
					push(bmpdata, it, uint8_t(0));
				}
			}
			return;
		}*/

		//bmp�t�@�C���w�b�_�����
		BMPFILEHEADER bmpfile;//�����őS�f�[�^�ʂ�bfSize�ɕK�v�����W�R�s�̒����ł���

		//bmp�摜�w�b�_�����
		BMPINFOHEADER bmptag;
		bmptag.biHeight = pcxheight;
		bmptag.biWidth = pcxwidth;
		//��������bmp�f�[�^��W�J���Ă���

		int resizeline = (4 - (bmptag.biWidth) % 4) % 4;//���}�X����˂΂Ȃ�Ȃ���
		int bmpline = bmptag.biWidth + resizeline;//�\�z�����bmp�̉���
		uint32_t bmpsize = (14) + (bmptag.biSize) + (bmptag.biClrUsed * 4) + (bmpline*bmptag.biHeight);//�t�@�C���w�b�_�A�摜�w�b�_�A�p���b�g�A�⊮�����摜�̃s�N�Z���� �P�ʂ�byte
		outputsizebuf = bmpsize;
		bmpfile.bfSize = bmpsize;


		uint8_t* bmpdata = new uint8_t[bmpsize];
		int it = 0;
		//�t�@�C���w�b�_
		push(bmpdata, it, bmpfile.bfType);
		push(bmpdata, it, bmpfile.bfSize);
		push(bmpdata, it, bmpfile.bfReserved1);
		push(bmpdata, it, bmpfile.bfReserved2);
		//�I�t�Z�b�g�����߂�B�S�T�C�Y����C���f�b�N�X�̃T�C�Y������ 0 1 2 3 4
		bmpfile.bfOffBits = bmpsize - (bmpline*bmptag.biHeight);//   a a a b b    5-2 = 3
		push(bmpdata, it, bmpfile.bfOffBits);
		//�摜�w�b�_
		push(bmpdata, it, bmptag.biSize);
		push(bmpdata, it, bmptag.biWidth);
		push(bmpdata, it, bmptag.biHeight);
		push(bmpdata, it, bmptag.biPlanes);
		push(bmpdata, it, bmptag.biBitCount);
		push(bmpdata, it, bmptag.biCompression);
		push(bmpdata, it, bmptag.biSizeImage);
		push(bmpdata, it, bmptag.biXPelsPerMeter);
		push(bmpdata, it, bmptag.biYPelsPerMeter);
		push(bmpdata, it, bmptag.biClrUsed);
		push(bmpdata, it, bmptag.biClrImportant);
		//�J���[�p���b�g,bmp�̃f�[�^�̕��т�b,g,r�ł��邱�Ƃɒ���

		if (sharepalette == nullptr) {
			for (int i = 0; i < 256; i++) {
				push(bmpdata, it, mypalette->colors[i].b);
				push(bmpdata, it, mypalette->colors[i].g);
				push(bmpdata, it, mypalette->colors[i].r);
				push(bmpdata, it, mypalette->colors[i].reserved);
			}
		}
		else {
			for (int i = 0; i < 256; i++) {
				if (i == 0) {
					push(bmpdata, it, (uint8_t)0);//���̃L���X�g�͋ɂ߂ďd�v
					push(bmpdata, it, (uint8_t)0);
					push(bmpdata, it, (uint8_t)0);
					push(bmpdata, it, (uint8_t)0);
				}
				else {
					push(bmpdata, it, sharepalette->colors[i].b);
					push(bmpdata, it, sharepalette->colors[i].g);
					push(bmpdata, it, sharepalette->colors[i].r);
					push(bmpdata, it, (uint8_t)0);
				}
			}
		}

		//�C���f�b�N�X
		for (int h = bmptag.biHeight - 1; h != -1; h--) {
			for (unsigned int w = 0; w < bmptag.biWidth; w++) {
				push(bmpdata, it, pcxindex[h][w]);
			}
			for (int w = 0; w < resizeline; w++) {
				push(bmpdata, it, uint8_t(0));
			}
		}

		if (mypalette != nullptr)
			delete mypalette;

		return bmpdata;
	}


	template<class Data_t>
	struct GDATA
	{
		short revisionx;
		short revisiony;

		unsigned int  xsize;
		unsigned int  ysize;

		Data_t ghandle;
		GDATA() {

			revisionx = 0;
			revisiony = 0;

			xsize = 0;
			ysize = 0;

		}
	};

	typedef short GROUP;
	typedef unsigned short IMAGE;

	//using Data_t = int;
	template<class Data_t>
	class SSff {
	public:
		SSff() {}
		SSff(SSff<Data_t> && right) : Graphdata(std::move(right.Graphdata)) {}
		SSff(std::string filename, std::function<Data_t(uint8_t* (&bmpdata), const int& sizebuf)> func) {
			Decord(filename, func);
		}
	public:
		std::map < GROUP, std::map<IMAGE, GDATA<Data_t> > > Graphdata;
		int Decord(std::string filename, std::function<Data_t(uint8_t* (&bmpdata), const int& sizebuf)> func) {

			std::ifstream sff(filename, std::ios::binary);
			if (!sff) {
				return -1;///////////////////
			}

			sff.seekg(0, std::ifstream::end);//�t�@�C��������T��
			
			auto eofPos = sff.tellg();//�t�@�C�������C���f�N�X���擾
			sff.clear();//�擪�ɂ��ǂ邽�߂Ɉ�xclear()��������B��������Ȃ��Ǝ���seekg()�ŃR�P��Ƃ�������B
			sff.seekg(0, std::ifstream::beg);//�t�@�C���擪�ɖ߂�
			auto begPos = sff.tellg();//�t�@�C���擪�C���f�N�X���擾
			auto size = eofPos - begPos;

			auto it = new uint8_t[size];
			auto itbeg = it;

			sff.read(reinterpret_cast<char*>(it),size);
			sff.clear();//�擪�ɂ��ǂ邽�߂Ɉ�xclear()��������B��������Ȃ��Ǝ���seekg()�ŃR�P��Ƃ�������B
			sff.seekg(0, std::ifstream::beg);//�t�@�C���擪�ɖ߂�


			int numofgroupes = 0;
			int numofimages = 0;

			for (int i = 0; i < 0x10; i++)//�O���[�v���܂Ŕ��
				std::uint8_t(*(it++));

			for (int i = 0; i < sizeof(numofgroupes); i++) {//groupe
				numofgroupes += std::uint8_t(*(it++)) * place[i];
			}
			for (int i = 0; i < sizeof(numofimages); i++) {//Image
				numofimages += std::uint8_t(*(it++)) * place[i];
			}

			{
				{
					long long now = it - itbeg;
					for (int i = 0; i < 0x200 - now; i++) {//�ŏ��̉摜�w�b�_��
						std::uint8_t(*(it++));
					}
				}

				Palette firstpalette;//Group0,Image0�̉摜���p���b�g���L�̏ꍇ�A���̃p���b�g�łȂ���ԍŏ��ɂ���摜�̃p���b�g���Q�Ƃ���炵���̂ŁA�ŏ��̃p���b�g��ێ�����K�v������
				Palette sharepalette;//�Ɨ��p���b�g�摜��ǂݍ��ނ��тɕύX����
				std::vector<std::pair<Sffdec::GROUP, Sffdec::IMAGE>> order;//���ꂽ���ԂɃO���[�v�A�C���[�W�l������

				for (int i = 0; i < numofimages; i++) {
					unsigned int nextgraphheader = 0;
					bool shareflag = false;
					bool renewflag = true;
					short cloneflag = 0;
					uint8_t* bmpdata = nullptr;
					GDATA<Data_t> Data;
					GROUP Group = 0;
					IMAGE Image = 0;
					for (int j = 0; j < sizeof(nextgraphheader); j++) {
						nextgraphheader += std::uint8_t(*(it++)) * place[j];//���̉摜�w�b�_�A�h���Xoffset
					}
					for (int j = 0; j < 4; j++)//pcx�f�[�^�T�C�Y
						std::uint8_t(*(it++));
					for (int j = 0; j < sizeof(Data.revisionx); j++) {//x�␳
						Data.revisionx += std::uint8_t(*(it++))*place[j];
					}
					for (int j = 0; j < sizeof(Data.revisiony); j++) {//���␳
						Data.revisiony += std::uint8_t(*(it++))*place[j];
					}
					for (int j = 0; j < sizeof(Group); j++) {
						Group += std::uint8_t(*(it++))*place[j];
					}
					for (int j = 0; j < sizeof(Image); j++) {
						Image += std::uint8_t(*(it++))*place[j];
					}

					for (int j = 0; j < sizeof(cloneflag); j++) {/*�N���[���t���O�f�[�^��2byte�g���Ă��āA�ォ�牽�Ԗڂ̉摜�ƈ�v
																 ���Ă��邩��������B�N���[���łȂ��Ƃ��͂O�������A�O�Ԗڂ̉摜�ƈ�v���Ă���Ƃ����O������炵�����A�ǂ������
																 ��ʂ��Ă��邩�͕s��*/
						cloneflag += std::uint8_t(*(it++))*place[j];
					}

					if (i == 0) {//0���ڂ̋��L�p���b�g�t���O�͖���
						std::uint8_t(*(it++));
						shareflag = false;
						renewflag = true;
					}
					else {
						shareflag = bool(std::uint8_t(*(it++)) == 1);//���L
						if (shareflag) {
							renewflag = false;
							if (Group == 0 && Image == 0)//Group0,Image0�̋��L�t���O�������Ă���Ƃ��Asff�̐擪�摜�̃p���b�g�f�[�^���g����
								sharepalette = firstpalette;
						}
						else
							renewflag = true;
					}

					for (int j = 0; j < 0x20 - 0x13; j++) {//�g���Ă��Ȃ��̈���X���[
						std::uint8_t(*(it++));
					}

					if (cloneflag > 0) {//clone������Ƃ�
						if (nextgraphheader != static_cast<long long>(it-itbeg)) {
							return -2;//////////
						}
						else {
							auto orderpair = order[cloneflag];
							if (Graphdata.find(Group) == Graphdata.end()) {//���̃O���[�v���Ȃ����
								Graphdata.insert(std::map<GROUP, std::map<IMAGE, GDATA<Data_t>> >::value_type(Group, {}));
							}

							Graphdata[Group].insert({ Image, Graphdata[orderpair.first][orderpair.second] });

							order.push_back({ Group, Image });
						}
					}
					else {//clone���Ȃ��Ƃ�

						int sizebuf;
						bmpdata = Convert(it, sizebuf, shareflag == true ? &sharepalette : nullptr, renewflag == true ? &sharepalette : nullptr, &Data.xsize, &Data.ysize);
						if (bmpdata == 0) {
							return -3;//////////
						}
						Data.ghandle = func(bmpdata, sizebuf);
						delete[](bmpdata);

						std::pair<IMAGE, GDATA<Data_t>> temppair(Image, Data);
						if (Graphdata.find(Group) == Graphdata.end()) {//���̃O���[�v���Ȃ����
							Graphdata.insert({ Group,{} });
						}

						Graphdata[Group].insert(temppair);
						order.push_back({ Group, Image });

						if (i == 0)
							firstpalette = sharepalette;
					}
					if (nextgraphheader != static_cast<long long>(it-itbeg)) {
						return -2;//////////
					}
				}
			}
			delete[] itbeg;
			return 0;
		}

		//pfunc: bmpdata�ɂ͂�����bmp�摜�̃f�[�^����摜�𐶐�����֐��Bbmpdata�͏����delete���Ȃ���.
		/*DxLib���������̎�����
		**************************************************
		int user_func(const char* data, int&size){
		int ghandle = CreateGraphFromMem((char*)bmpdata, size);
		return ghandle;
		}

		int main(){
		SSff sff("Data/filename.sff",user_func);
		return 0;
		}
		***************************************************


		*/
	};
	template<class Data_t>
	class CSffmgr {

		static constexpr unsigned int bufsize = 50000000;
		std::map<std::string, SSff<Data_t>> data;
	private:

		CSffmgr() {}
		CSffmgr(CSffmgr<Data_t>&) {}
		CSffmgr(CSffmgr<Data_t>&&) {}
	public:

		static CSffmgr& GetSffmgr() {
			static CSffmgr<Data_t> stdata;
			return stdata;
		}
		~CSffmgr() {}
	public:
		int insertsff(std::string filename, std::function<Data_t(uint8_t* (&bmpdata), const int& sizebuf)> func) {
			if (data.count(filename) > 0)
				return -1;

			data.emplace(std::pair < std::string, SSff<Data_t> > {filename, SSff<Data_t>{filename, func}});
			return 0;
		}
		int insertsff(std::string filename) {
			if (data.count(filename) > 0)
				return -1;
			data.insert(decltype(data)::value_type{ filename,SSff<Data_t>{} });
			return 0;
		}
		int erasesff(std::string filename) {
			if (data.count(filename) > 0)
				data.erase(filename);

			return 0;
		}
		int eraseall() {
			data.clear();
			return 0;
		}
		void aplly(std::function<void(SSff<Data_t>&)>func) {
			for (auto&e : data) {
				func(e.second);
			}
		}
		bool count(std::string filename) {
			auto ret = data.count(filename) > 0;
			return ret;
		}
		SSff<Data_t>& operator[](std::string filename) {
			return data[filename];
		}


	};




}



#endif