#ifndef AnimH
#define AnimH
#include <vector>
#include <map>
#include<array>
namespace Coordinate {
    struct SCoordinate {

        std::array<int, 4> coordinate;//x1,y1,x2,y2

        SCoordinate() {
            for (int i = 0; i < 4; i++) {
                coordinate[i] = 0;
            }

        }
        SCoordinate(int x1, int y1, int x2, int y2) {

            coordinate[0] = x1;
            coordinate[1] = y1;
            coordinate[2] = x2;
            coordinate[3] = y2;

            diagonalchange(coordinate);
        }
        template<class Arr>
        SCoordinate(Arr argarr) {
            for (int i = 0; i < 4; i++) {
                coordinate[i] = argarr[i];
            }
            diagonalchange(coordinate);
        }
        SCoordinate operator*(double arg_enlarge_rate) {
            SCoordinate tmp;
            for (int i = 0; i < 4; i++) {
                tmp.coordinate[i] = coordinate[i] * arg_enlarge_rate;
            }
            return tmp;
        }
        SCoordinate operator*(std::pair<double, double> arg_enlarged_rate) {
            SCoordinate tmp;
            tmp.coordinate[0] = coordinate[0] * arg_enlarged_rate.first;
            tmp.coordinate[1] = coordinate[1] * arg_enlarged_rate.second;
            tmp.coordinate[2] = coordinate[2] * arg_enlarged_rate.first;
            tmp.coordinate[3] = coordinate[3] * arg_enlarged_rate.second;
            return tmp;
        }
        static int diagonalchange(int temp[4]) {

            int vector[2] = { temp[2] - temp[0], temp[3] - temp[1] };

            if (vector[0] < 0) {
                temp[0] += vector[0];
                temp[2] -= vector[0];
            }
            if (vector[1] < 0) {
                temp[1] += vector[1];
                temp[3] -= vector[1];
            }

            return 0;
        }
        static int diagonalchange(std::array<int, 4>& temp) {

            int vector[2] = { temp[2] - temp[0], temp[3] - temp[1] };

            if (vector[0] < 0) {
                temp[0] += vector[0];
                temp[2] -= vector[0];
            }
            if (vector[1] < 0) {
                temp[1] += vector[1];
                temp[3] -= vector[1];
            }

            return 0;
        }
        static SCoordinate reverseX(const SCoordinate &a_) {
            SCoordinate ret;
            ret.coordinate = a_.coordinate;
            ret.coordinate[0] *= -1;
            ret.coordinate[2] *= -1;
            diagonalchange(ret.coordinate);
            return ret;
        }
    };

    struct SClsn {
    private:
        std::vector<SCoordinate> Clsn;
    public:

        SClsn operator*(double arg_enlarge_rate) {
            if (arg_enlarge_rate == 1)
                return *this;

            SClsn tmp;
            for (auto it = Clsn.begin(); it != Clsn.end(); it++) {
                tmp.push_back((*it)*arg_enlarge_rate);
            }
            return tmp;
        }
        SClsn operator*(std::pair<double, double> arg_enlarged_rate) {
            SClsn tmp_clsn;
            for (auto it = Clsn.begin(); it != Clsn.end(); it++) {
                tmp_clsn.push_back((*it)*arg_enlarged_rate);
            }
            return tmp_clsn;
        }
        SCoordinate operator[](unsigned int arg_index) {
            if (arg_index > Clsn.size() - 1)
                return SCoordinate();
            return Clsn[arg_index];
        }
        SClsn operator+(std::pair<int, int> add) {
            SClsn output;
            for (auto it = Clsn.cbegin(); it != Clsn.cend(); it++) {
                auto& elem = it->coordinate;
                output.push_back(SCoordinate{ elem[0] + add.first,elem[1] + add.second,elem[2] + add.first,elem[3] + add.second });
            }
            return output;
        }
        SClsn& operator=(std::vector<SCoordinate> arg) {
            Clsn = arg;
            return *this;
        }
        void push_back(SCoordinate arg) {
            Clsn.push_back(arg);
            return;
        }

        unsigned int size() {
            return Clsn.size();
        }



        const std::vector<SCoordinate>& GetClsn()const {
            return Clsn;
        }

    };
}

struct SFrame{

    Coordinate::SClsn Clsn1;
    Coordinate::SClsn Clsn2;

    int Image;
    int Group;
    int delay;

    int reverse;//0�Œʏ�A�P�ō��E�A�Q�ŏ㉺,�R�ŗ���
    int layer;//0�Œʏ�A�P�œ��߁A2�Ō��Z
    int AS = 255;
    int D = 255;
    SFrame(){
        Image = -1;
        Group = -1;
        delay = -1;
        reverse = 0;
        layer = 0;
    }

};

struct SAnim{
private:
    std::vector<SFrame> Anim;//�f�[�^�̍Ō�̖ڈ�ɁASframe�̃R���X�g���N�^���������̃f�[�^�������Ă܂�
public:
    int loop = -1;//0���琔����loop�Ԗڂ̉摜���烋�[�v������B	-1�Ȃ烋�[�v���Ȃ�
    int Dec(FILE*& fp);
    SAnim(){
    }

    SFrame& operator[](unsigned int argindex){
        if (argindex >= Anim.size() || argindex < 0){
            if (Anim.size() == 0){
                SFrame tmp{};
                Anim.push_back(tmp);
            }
            return Anim[0];
        }
        else
            return Anim[argindex];
    }
    unsigned int size(){ return Anim.size(); }


    int GetMaxAirFrame() {
        int ret = 0;
        for (int i = 0, e = Anim.size(); i < e; i++) {
            ret += Anim[i].delay;
        }
        return ret;
    }

    int GetAirFrameIndexByMotionTime(int motiontime_) {
        int frame = 1;
        int airinddexOfTheFrame = 0;
        for (int airindexOfTheFrame, e = Anim.size(); airinddexOfTheFrame < e; airinddexOfTheFrame++) {
            frame += Anim[airinddexOfTheFrame].delay;
            if (motiontime_ <= frame) {
                break;
            }
        }

        return airinddexOfTheFrame;
    }

    SFrame& GetAirFrameByMotionTime(int motiontime_) {
        int frame = 1;
        int airinddexOfTheFrame = 0;
        for (int airindexOfTheFrame, e = Anim.size(); airinddexOfTheFrame < e; airinddexOfTheFrame++) {
            frame += Anim[airinddexOfTheFrame].delay;
            if (motiontime_ <= frame) {
                break;
            }
        }

        return Anim[airinddexOfTheFrame];
    }

};
//Decair�̕Ԃ��l
typedef int Actionnum;
struct SAction{
    std::map<Actionnum,SAnim> Action;
    int Decair(const char* filename);

};

inline int getdata(std::vector<std::string>& temp, const std::string& tempbuf, unsigned int first = 0) {//,��؂�̃f�[�^�����I�[�����s�ł���Ƃ����O��
    unsigned int index1 = first;
    unsigned int length = tempbuf.length();
    for (int i = 0;; i++) {

        unsigned int index2 = tempbuf.find_first_of(",\n", index1);
        std::string data("");
        for (unsigned int j = index1; j < index2; j++) {
            if ((tempbuf[j] == ' ') || (tempbuf[j] == '\t'))
                continue;
            data += tempbuf[j];
        }
        temp.push_back(data);
        index1 = index2 + 1;
        if (index1 >= length)
            break;
    }

    return 0;
}

inline int SAnim::Dec(FILE*& fp) {


    char digit[] = "-0123456789";
    bool defaultclsnflag = false;//true�Ȃ��,clsn��default���ɓ����B����Default�ɉ�Ό���defaultclsn��clsn1,clsn2��clsnnum�Ŏw�肳�ꂽ��������������,true�ɂȂ�@default�̒�`�łȂ��Ƃ�false�ɂȂ�
    bool usedefaultflag1 = true;//true�Ȃ�΁Aframe������Ƃ�default���g���BDefault�łȂ�Clsn�̒�`�ɉ��false����āAdefault�łȂ�Clsn������frame�������Ƃ�true�����
    bool usedefaultflag2 = true;
    int clsnnum = 0;
    bool nextloop = false;//true�Ȃ玟�̃t���[����loop��true�ɂ���

    SFrame nowframe;
    SFrame defaultclsn;
    while (1) {
        std::string buf("");
        {
            char temp;
            do {
                temp = fgetc(fp);
                buf += temp;
            } while ((temp != '\n') && (temp != EOF));
        }



        if (buf.find_first_of(";") != std::string::npos)
            buf.erase(buf.find_first_of(";"));


        buf.erase(buf.find_last_not_of(" \t\n;") + 1);//�I�[�������s��
        buf += '\n';

        if ((buf.find("LoopStart") != std::string::npos) || (buf.find("Loopstart") != std::string::npos)
            || (buf.find("loopStart") != std::string::npos) || (buf.find("loopstart") != std::string::npos)) {//loopstart
            nextloop = true;
            defaultclsnflag = false;
            usedefaultflag1 = true;
            usedefaultflag2 = true;
            continue;
        }

        if (buf.find_first_of(digit) == std::string::npos) {//�������S�������ĂȂ����
            break;
        }

        unsigned int it = 0;
        if (it = (buf.find("Clsn", 0) != std::string::npos)) {//Clsn�̕���������ꍇ
            if (buf.find(":") != std::string::npos) {//clsn�̒�`
                unsigned int index1 = buf.find_first_of(digit);
                clsnnum = buf[index1] - '0';//Clsn?:

                if (buf.find("Default") != std::string::npos) {//Default�Ȃ��
                    if (clsnnum == 1)
                        defaultclsn.Clsn1 = std::vector<Coordinate::SCoordinate>();
                    else if (clsnnum == 2)
                        defaultclsn.Clsn2 = std::vector<Coordinate::SCoordinate>();
                    defaultclsnflag = true;
                }
                else
                {
                    if (clsnnum == 1) {
                        usedefaultflag1 = false;
                    }
                    if (clsnnum == 2) {
                        usedefaultflag2 = false;
                    }
                    defaultclsnflag = false;
                }
                continue;
            }
            else {//���ۂ�clsn���W
                Coordinate::SCoordinate tempCoordinate;
                std::vector<std::string>data;
                unsigned int first = buf.find("=") + 1;
                getdata(data, buf, first);
                for (int i = 0; i < 4; i++)
                    tempCoordinate.coordinate[i] = atoi(data[i].c_str());

                Coordinate::SCoordinate::diagonalchange(tempCoordinate.coordinate);

                if (clsnnum == 1) {
                    if (defaultclsnflag == true) {
                        defaultclsn.Clsn1.push_back(tempCoordinate);
                    }
                    else nowframe.Clsn1.push_back(tempCoordinate);
                }
                else if (clsnnum == 2) {
                    if (defaultclsnflag == true) {
                        defaultclsn.Clsn2.push_back(tempCoordinate);
                    }
                    else nowframe.Clsn2.push_back(tempCoordinate);
                }
                else return -2;//***********************************************
                continue;
            }
        }
        //frame����
        SFrame data;

        if (usedefaultflag1 == true) {
            data.Clsn1 = defaultclsn.Clsn1;
        }
        else {
            data.Clsn1 = nowframe.Clsn1;
            usedefaultflag1 = true;
            nowframe.Clsn1 = std::vector<Coordinate::SCoordinate>();
        }
        if (usedefaultflag2 == true) {
            data.Clsn2 = defaultclsn.Clsn2;
        }
        else {
            data.Clsn2 = nowframe.Clsn2;
            usedefaultflag2 = true;
            nowframe.Clsn2 = std::vector<Coordinate::SCoordinate>();
        }
        std::vector<std::string> framedata;
        getdata(framedata, buf);

        switch (framedata.size()) {
        case 7: {
            std::size_t it;
            if ((it = framedata[6].find("AS")) != std::string::npos) {
                data.AS = std::stoi(std::string{ framedata[6].begin() + it + 2,framedata[6].end() });
                data.layer = 1;
            }
            else if (framedata[6].find("A") != std::string::npos) {
                data.layer = 1;
            }
            else if (framedata[6].find("S") != std::string::npos) {
                data.layer = 2;
            }
            if ((it = framedata[6].find("D")) != std::string::npos) {
                data.D = std::stoi(std::string{ framedata[6].begin() + it + 1,framedata[6].end() });
                data.layer = 1;
            }
        }
        case 6: {
            if (framedata[5].find("H") != std::string::npos)
                data.reverse += 1;
            if (framedata[5].find("V") != std::string::npos)
                data.reverse += 2;
        }
        case 5: {
            data.Group = atoi(framedata[0].c_str());
            data.Image = atoi(framedata[1].c_str());
            data.delay = atoi(framedata[4].c_str()); break;
        }
        default: return -3;//*******************************************
        }

        if (nextloop == true) {
            loop = Anim.size();//���̎��_�ō��������悤�Ƃ���SFrame��,Anim�̂O���琔����vector�C���f�b�N�X�Ԗڂ������[�v�̍ŏ��ƂȂ�
            nextloop = false;
        }
        Anim.push_back(data);
    }

    return 0;

}

inline int SAction::Decair(const char* filename) {
    Action.clear();
    FILE* fp;
    if ((fp = fopen(filename, "r")) == NULL)
        return -1;

    char digit[] = "-0123456789";
    while (1) {
        std::string buf("");
        {
            char temp;
            do {
                temp = fgetc(fp);
                buf += temp;
            } while ((temp != '\n') && (temp != EOF));
        }



        if (buf.find_first_of(";") != std::string::npos)
            buf.erase(buf.find_first_of(";"));


        buf.erase(buf.find_last_not_of(" \t\n;") + 1);//�I�[�������s��
        buf += '\n';


        if (buf[0] == EOF)
            break;

        int actionnum = -1;
        if (buf.find("[") != std::string::npos) {
            unsigned int index1 = buf.find_first_of(digit);
            unsigned int index2 = buf.find_first_not_of(digit, index1);
            std::string num("");
            for (unsigned int i = index1; i < index2; i++)
                num += buf[i];
            actionnum = atoi(num.c_str());


            SAnim Anim;
            int Decerror = 0;
            if ((Decerror = Anim.Dec(fp)) < 0)
                return Decerror;

            Action.insert(std::pair<Actionnum, SAnim>(actionnum, Anim));
        }
    }
    fclose(fp);
    return 0;
}
/*Loopstart ���@LoopStart�ɓ��ꂵ�Ȃ��ƕs�
�R�����g�ȊO�̂ɗ]�v�ȂƂ���ɗ]�v�Ȃ��̂������ƕs�
�R�����g�͑��v�Ȃ͂�*/
#endif