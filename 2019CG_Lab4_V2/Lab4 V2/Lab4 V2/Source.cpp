#include<gl/GLUT.H>
#include<vector>
#include<set>
#include<iostream>
#include<math.h>
#include<fstream>
#include<string>
#include<regex>
#include<stdlib.h>
#include<algorithm>

#define PI 3.141592

using namespace std;
/*
	cd /d D:\�`��Ƨ�\�p����Ͼǧ@�~\2019CG_Lab4_106502530\Debug	2019CG_Lab4_106502530.exe Lab4A.in
*/

// 2D�y��
class Coordinate {
public:
	int x;
	int y;
	Coordinate() {

	}

	Coordinate(float X, float Y) {
		x = round(X);
		y = round(Y);
	}
};

// 3D�y�ЩΦV�q
class Point3D {
public:
	float x, y, z, w;

	Point3D(float X, float Y, float Z, float W) {
		x = X;
		y = Y;
		z = Z;
		w = W;
	}
};

// �����C��RGB�ƭ�
class Color {
public:
	float r, g, b;

	Color() {

	}

	Color(float R, float G, float B) {
		r = R;
		g = G;
		b = B;
	}
};

// �u�q���A
enum type { keep, reject, clip1, clip2, clipBoth };

// �ΨӰO���u�q��clip���p
class Clipper {
public:
	type actType = keep;	// �u�q���p	
	float clipX = 0, clipY = 0, clipZ = 0, clipW = 1;	// clip�᪺�y��
	float clipX2 = 0, clipY2 = 0, clipZ2 = 0, clipW2 = 1;
};

// �����������ƾ�
class Light {
public:
	float Ipr, Ipg, Ipb;
	Point3D LightPos = Point3D(0, 0, 0, 1);
};

// �ϧΪ���
class Object {
public:
	vector<vector<float>> points;
	vector<vector<int>> faceList;	// �����C�ӭ����զ����I
	vector<vector<Clipper>> clipList; // �����C�ӭ����u�qclip���p
	bool* areaVisible;	// �O�_��ݨ�o�ӭ�
	Color* areaColor;	// �����C�ӭ��bflat shading�U���C��
	int pointNum, areaNum;
	int n1, n2;
	float Kd, Ks, nPow;	// shading�����Ѽ�
	float Or, Og, Ob;

	// ��l��
	Object(int pointNumber, int areaNumber) {
		pointNum = pointNumber;
		areaNum = areaNumber;
		areaVisible = new bool[areaNumber];
		areaColor = new Color[areaNumber];
		points.push_back(vector<float>());
		points.push_back(vector<float>());
		points.push_back(vector<float>());
		points.push_back(vector<float>());
	}

	// �O���I
	void AddPoint(Point3D p) {
		points[0].push_back(p.x);
		points[1].push_back(p.y);
		points[2].push_back(p.z);
		points[3].push_back(p.w);
	}

	// ���mareaVisible array
	void resetVisible() {
		for (int i = 0; i < areaNum; i++)
			areaVisible[i] = false;
	}

	// ��sFaceList
	void ListPush(vector<int>& nums, vector<Clipper>& clips) {
		faceList.push_back(nums);
		clipList.push_back(clips);
	}

	void Print() {
		cout << "List:\n";
		for (int i = 0; i < faceList.size(); i++) {
			cout << i + 1 << ": ";
			for (auto j : faceList[i]) {
				cout << j + 1 << " ";
			}
			cout << endl;
		}
		cout << "Points:\n";
		for (int i = 0; i < pointNum; i++) {
			cout << points[0][i] << " ";
			cout << points[1][i] << " ";
			cout << points[2][i] << " ";
			cout << points[3][i] << "\n";
			cout << endl;
		}
	}
};


// �禡�ŧi
void displayFunc();
void readFile();
void readASCFile();
float stringToFloat(string str);
vector<vector<float>> matrixMult(vector<vector<float>> matrixA, vector<vector<float>> matrixB);
void resetTM();
void scale(float scaleX, float scaleY, float scaleZ);
void rotate(float rotateX, float rotateY, float rotateZ);
void translate(float transX, float transY, float transZ);
void computeEM();
void computePM();
void pipelining();
vector<vector<float>> creatWVMatrix();
Point3D vectorCross(Point3D v1, Point3D v2);
Point3D vectorNormalize(Point3D v1);
void drawObject(vector<vector<float>> points, vector<vector<int>> faceList, bool* areaVisible, vector<vector<Clipper>> clipList, Color* areaColor);
void drawDot(float x, float y);
void drawLine(Coordinate coA, Coordinate coB);
void coloring(vector<Point3D> points, float maxX, float maxY, float minX, float minY);
void drawViewPort();
void storeObject();
void flatShading(Object* object, vector<vector<float>> points);
void backFaceCheck(Object* object, vector<vector<float>> points);
void clipping(Object* object, vector<vector<float>> points);
vector<vector<Clipper>> clipWVM(vector<vector<float>> WVM, vector<vector<Clipper>> clipList);
vector<float> checkClip(float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2);
bool isInsidePoly(float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, float t, float k);
bool isInsidePoly2D(float x, float y, vector<Point3D> points);
float findAngle(Point3D v1, Point3D v2);
float vectorDot(Point3D v1, Point3D v2);
float illuminate(int rgbType, Object* object, vector<vector<float>> points, Point3D normalV, int targetPoint);

// �ܼƫŧi
int winWidth, winHeight;
fstream file, ascFile;
string fileName, previousName = "";		// in�ɦW�P�W��ASC�ɦW
string str, ascStr;
regex tokenRegex("[^\\s]+");	// regular expression
sregex_iterator iter, ascIter;	// iterator for regex
vector<Object> objects;
Object object = Object(0, 0);			// ����
vector<vector<float>> TM(4, vector<float>(4));		// transform matrix
vector<vector<float>> EM(4, vector<float>(4));		// eye matrix
vector<vector<float>> PM(4, vector<float>(4, 0));		// project matrix
vector<Light> lights;		// store lights
float zBuffer[1000][1000];
Color cBuffer[1000][1000];
Light light;
float eyeX, eyeY, eyeZ, coiX, coiY, coiZ, tilt;
float hPM, yPM, degPM;
float vXmin, vXmax, vYmin, vYmax;	// viewport
float wXmin = -1, wXmax = 1, wYmin = -1, wYmax = 1;	// window of camera
float clipXmin, clipXmax, clipYmin, clipYmax;
bool noBackFaces = false;
bool enterViewPort = false;
float KaIar, KaIag, KaIab;		// ambient intensity
float Br, Bg, Bb;			// color of background
float pixelR, pixelG, pixelB;	// color of current pixel
float pA, pB, pC, pD;		// value for plane formula

int main(int argc, char* argv[])
{
	system("pause");
	fileName = argv[1];								// ���oInput�ɦW
	file.open(fileName, ios::in);
	if (!file)     //�ˬd�ɮ׬O�_���\�}��
	{
		cerr << "Can't open file!\n";
		exit(1);     //�b�����`���ΤU�A���_�{��������
	}
	getline(file, str);
	iter = sregex_iterator(str.begin(), str.end(), tokenRegex);
	winWidth = stringToFloat((*iter).str());
	winHeight = stringToFloat((*++iter).str());

	glutInit(&argc, argv);							// �ϥ�glut�禡�w�ݭn�i���l��
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);	// �]�w������ܼҦ��A�C��ҫ��M�w�s�A�o�̬ORGB�C��ҫ��M��w�s
	glutInitWindowPosition(100, 100);				// �]�w�X�{��������l��m�A���W�������I
	glutInitWindowSize(winWidth, winHeight);		// �����j�p
	glutCreateWindow("�e�ϵ{��");					// �����W��

	gluOrtho2D(0.0, winWidth, 0.0, winHeight);
	glutDisplayFunc(displayFunc);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glutMainLoop();

	return 0;
}

void displayFunc() {
	readFile();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
}

// Ū��input��
void readFile() {
	string token;
	resetTM();
	// �v��Ū��input�é�Ѧ�token
	while (getline(file, str)) {
		if (str[0] == '#' || str == "") {		// Ū������ѩΪŦ�
			continue;
		}
		iter = sregex_iterator(str.begin(), str.end(), tokenRegex);
		token = (*iter).str();
		if (token == "scale") {
			cout << "scale\n\n";
			float sX = stringToFloat((*++iter).str());
			float sY = stringToFloat((*++iter).str());
			float sZ = stringToFloat((*++iter).str());
			scale(sX, sY, sZ);
		}
		else if (token == "rotate") {
			cout << "rotate\n\n";
			float rX = stringToFloat((*++iter).str());
			float rY = stringToFloat((*++iter).str());
			float rZ = stringToFloat((*++iter).str());
			rotate(rX, rY, rZ);
		}
		else if (token == "translate") {
			cout << "translate\n\n";
			float tX = stringToFloat((*++iter).str());
			float tY = stringToFloat((*++iter).str());
			float tZ = stringToFloat((*++iter).str());
			translate(tX, tY, tZ);
		}
		else if (token == "object") {
			token = (*++iter).str();
			ascFile.open(token, ios::in);
			if (!ascFile)     //�ˬd�ɮ׬O�_���\�}��
			{
				cerr << "Can't open ASC file!\n";
				exit(1);     //�b�����`���ΤU�A���_�{��������
			}
			if (previousName != token) {	// �Y�ɮ׸�W���ɮפ��P�~Ū��
				previousName = token;
				readASCFile();
			}
			ascFile.close();
			object.Or = stringToFloat((*++iter).str());
			object.Og = stringToFloat((*++iter).str());
			object.Ob = stringToFloat((*++iter).str());
			object.Kd = stringToFloat((*++iter).str());
			object.Ks = stringToFloat((*++iter).str());
			object.nPow = stringToFloat((*++iter).str());
			storeObject();
		}
		else if (token == "observer") {
			eyeX = stringToFloat((*++iter).str());
			eyeY = stringToFloat((*++iter).str());
			eyeZ = stringToFloat((*++iter).str());
			coiX = stringToFloat((*++iter).str());
			coiY = stringToFloat((*++iter).str());
			coiZ = stringToFloat((*++iter).str());
			tilt = stringToFloat((*++iter).str());
			hPM = stringToFloat((*++iter).str());
			yPM = stringToFloat((*++iter).str());
			degPM = stringToFloat((*++iter).str());
			computeEM();
			computePM();
		}
		else if (token == "viewport") {
			enterViewPort = true;
			vXmin = stringToFloat((*++iter).str());
			vXmax = stringToFloat((*++iter).str());
			vYmin = stringToFloat((*++iter).str());
			vYmax = stringToFloat((*++iter).str());
			computePM();
		}
		else if (token == "ambient") {
			KaIar = stringToFloat((*++iter).str());
			KaIag = stringToFloat((*++iter).str());
			KaIab = stringToFloat((*++iter).str());
		}
		else if (token == "background") {
			Br = stringToFloat((*++iter).str());
			Bg = stringToFloat((*++iter).str());
			Bb = stringToFloat((*++iter).str());
		}
		else if (token == "light") {
			int index = stringToFloat((*++iter).str());
			float x, y, z;
			light.Ipr = stringToFloat((*++iter).str());
			light.Ipg = stringToFloat((*++iter).str());
			light.Ipb = stringToFloat((*++iter).str());
			x = stringToFloat((*++iter).str());
			y = stringToFloat((*++iter).str());
			z = stringToFloat((*++iter).str());
			light.LightPos = Point3D(x, y, z, 1);
			if (index > lights.size())		// �s������
				lights.push_back(light);
			else							// �ª�����
				lights[index - 1] = light;
		}
		else if (token == "display") {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glFinish();
			drawViewPort();
			pipelining();
			system("pause");
		}
		else if (token == "nobackfaces") {
			noBackFaces = true;
		}
		else if (token == "reset") {
			resetTM();
		}
		else if (token == "end") {
			file.close();
			exit(0);
		}
	}
}

// Ū��ASC�ɪ����
void readASCFile() {
	bool getPointNum = true;
	int pointNum, areaNum;
	float x, y, z;
	while (getline(ascFile, ascStr)) {
		if (ascStr == "") {		// Ū���Ŧ�
			continue;
		}
		ascIter = sregex_iterator(ascStr.begin(), ascStr.end(), tokenRegex);
		if (getPointNum) {		// Ū���I�P�����ƶq
			getPointNum = false;
			pointNum = stringToFloat((*ascIter).str());
			areaNum = stringToFloat((*++ascIter).str());
			object = Object(pointNum, areaNum);
			continue;
		}
		if (pointNum > 0) {		// �x�s�I�y��
			pointNum--;
			x = stringToFloat((*ascIter).str());
			y = stringToFloat((*++ascIter).str());
			z = stringToFloat((*++ascIter).str());
			//cout << x << " " << y << " " << z << endl;
			object.AddPoint(Point3D(x, y, z, 1));
		}
		else if (areaNum > 0) {	// �x�sadjList
			areaNum--;
			vector<int> nums;
			int first = stringToFloat((*++ascIter).str()) - 1;
			for (; ascIter != sregex_iterator(); ascIter++) {
				//cout << stringToFloat((*ascIter).str()) << " ";
				nums.push_back(stringToFloat((*ascIter).str()) - 1);
			}
			//cout << endl;
			nums.push_back(first);
			vector<Clipper> clips(nums.size() - 1);
			object.ListPush(nums, clips);
		}
	}
	//object.Print();
}

// ���mtransfom matrix
void resetTM() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (i == j)
				TM[i][j] = 1;
			else
				TM[i][j] = 0;
		}
	}
}

// ����scale matrix�íp��TM
void scale(float sX, float sY, float sZ) {
	vector<vector<float>> SM(4, vector<float>(4, 0));
	SM[0][0] = sX;
	SM[1][1] = sY;
	SM[2][2] = sZ;
	SM[3][3] = 1;

	TM = matrixMult(SM, TM);
	vector<vector<float>>().swap(SM);
}

// ����rotate matrix�íp��TM
void rotate(float rX, float rY, float rZ) {
	vector<vector<float>> RM1(4, vector<float>(4, 0)), RM2(4, vector<float>(4, 0)), RM3(4, vector<float>(4, 0));
	float radX = rX * PI / 180;		// �ഫ�����׳��
	float radY = rY * PI / 180;
	float radZ = rZ * PI / 180;
	RM1[0][0] = 1, RM1[1][1] = cos(radX), RM1[1][2] = -sin(radX);
	RM1[2][1] = sin(radX), RM1[2][2] = cos(radX), RM1[3][3] = 1;

	RM2[0][0] = cos(radY), RM2[0][2] = sin(radY), RM2[1][1] = 1;
	RM2[2][0] = -sin(radY), RM2[2][2] = cos(radY), RM2[3][3] = 1;

	RM3[0][0] = cos(radZ), RM3[0][1] = -sin(radZ), RM3[1][0] = sin(radZ);
	RM3[1][1] = cos(radZ), RM3[2][2] = 1, RM3[3][3] = 1;

	if (rX != 0)
		TM = matrixMult(RM1, TM);
	if (rY != 0)
		TM = matrixMult(RM2, TM);
	if (rZ != 0)
		TM = matrixMult(RM3, TM);
	cout << "RM1:\n";
	for (int i = 0; i < 4; i++) {
		cout << "[";
		for (int j = 0; j < 4; j++) {
			cout << RM1[i][j] << " ";
		}
		cout << "]\n";
	}
	cout << "RM2:\n";
	for (int i = 0; i < 4; i++) {
		cout << "[";
		for (int j = 0; j < 4; j++) {
			cout << RM2[i][j] << " ";
		}
		cout << "]\n";
	}
	cout << "RM3:\n";
	for (int i = 0; i < 4; i++) {
		cout << "[";
		for (int j = 0; j < 4; j++) {
			cout << RM3[i][j] << " ";
		}
		cout << "]\n";
	}
	vector<vector<float>>().swap(RM1);
	vector<vector<float>>().swap(RM2);
	vector<vector<float>>().swap(RM3);
}

// ����translate matrix�íp��TM
void translate(float tX, float tY, float tZ) {
	vector<vector<float>> TransM(4, vector<float>(4, 0));
	TransM[0][0] = 1;
	TransM[0][3] = tX;
	TransM[1][1] = 1;
	TransM[1][3] = tY;
	TransM[2][2] = 1;
	TransM[2][3] = tZ;
	TransM[3][3] = 1;

	TM = matrixMult(TransM, TM);
	vector<vector<float>>().swap(TransM);
}

// �p��world space to eye space matrix
void computeEM() {
	vector<vector<float>> TM(4, vector<float>(4, 0));
	vector<vector<float>> GRM(4, vector<float>(4, 0));
	vector<vector<float>> MM(4, vector<float>(4, 0));	// mirror matrix
	vector<vector<float>> RM(4, vector<float>(4, 0));	// tilt matrix
	float degree = tilt * PI / 180;
	Point3D vectorX(1, 0, 0, 0), vectorY(0, 1, 0, 0), vectorZ(0, 0, 1, 0);
	// ��l�Ưx�}TM
	TM[0][0] = 1, TM[1][1] = 1, TM[2][2] = 1, TM[3][3] = 1;
	TM[0][3] = -eyeX, TM[1][3] = -eyeY, TM[2][3] = -eyeZ;
	// ��l�Ưx�}MM
	MM[0][0] = -1, MM[1][1] = 1, MM[2][2] = 1, MM[3][3] = 1;
	// ��l�Ưx�}RM
	RM[0][0] = cos(degree), RM[0][1] = sin(degree), RM[2][2] = 1;
	RM[1][0] = -sin(degree), RM[1][1] = cos(degree), RM[3][3] = 1;
	// �p��x�}GRM
	vectorZ.x = coiX - eyeX, vectorZ.y = coiY - eyeY, vectorZ.z = coiZ - eyeZ;
	vectorZ = vectorNormalize(vectorZ);
	vectorX = vectorNormalize(vectorCross(vectorY, vectorZ));
	vectorY = vectorNormalize(vectorCross(vectorZ, vectorX));
	GRM[0][0] = vectorX.x, GRM[0][1] = vectorX.y, GRM[0][2] = vectorX.z;
	GRM[1][0] = vectorY.x, GRM[1][1] = vectorY.y, GRM[1][2] = vectorY.z;
	GRM[2][0] = vectorZ.x, GRM[2][1] = vectorZ.y, GRM[2][2] = vectorZ.z;
	GRM[3][3] = 1;
	cout << "TE:\n";
	for (int i = 0; i < 4; i++) {
		cout << "[";
		for (int j = 0; j < 4; j++) {
			cout << TM[i][j] << " ";
		}
		cout << "]\n";
	}
	cout << "GRM:\n";
	for (int i = 0; i < 4; i++) {
		cout << "[";
		for (int j = 0; j < 4; j++) {
			cout << GRM[i][j] << " ";
		}
		cout << "]\n";
	}
	cout << "Mirror:\n";
	for (int i = 0; i < 4; i++) {
		cout << "[";
		for (int j = 0; j < 4; j++) {
			cout << MM[i][j] << " ";
		}
		cout << "]\n";
	}
	cout << "Tilt:\n";
	for (int i = 0; i < 4; i++) {
		cout << "[";
		for (int j = 0; j < 4; j++) {
			cout << RM[i][j] << " ";
		}
		cout << "]\n";
	}
	// �p��EM
	EM = matrixMult(GRM, TM);
	EM = matrixMult(MM, EM);
	EM = matrixMult(RM, EM);
}

// �p��project matrix
void computePM() {
	if (!enterViewPort)
		return;
	float aspectRatio = (vXmax - vXmin) / (vYmax - vYmin);
	float H = hPM, y = yPM, d = degPM;
	d = tan(d * PI / 180);
	PM[0][0] = 1;
	PM[1][1] = aspectRatio;
	PM[2][2] = y / (y - H) * d;
	PM[2][3] = H * y / (H - y) * d;
	PM[3][2] = d;
	cout << "PM:\n";
	for (int i = 0; i < 4; i++) {
		cout << "[";
		for (int j = 0; j < 4; j++) {
			cout << PM[i][j] << " ";
		}
		cout << "]" << endl;
	}
}

// �r���ഫ���B�I��
float stringToFloat(string str) {
	float num = 0, temp = 1;
	int index = 0;
	int sign = 1;
	if (str[0] == '-') {
		sign = -1;
		index++;
	}
	for (; str[index] != '.' && index < str.length(); index++) {
		num *= 10;
		num += str[index] - '0';
	}
	for (index += 1; index < str.length(); index++) {
		temp /= 10;
		num += (str[index] - '0') * temp;
	}
	return num * sign;
}

// ���I�i���ഫ
void pipelining() {
	for (int k = 0; k < objects.size(); k++) {
		Object object = objects[k];
		vector<vector<float>> points = object.points;
		//object.resetVisible();
		//if (noBackFaces)
			//backFaceCheck(&object, points);		// �`�N! ���ɪ�X�b�y�ЬO�ۤϪ�
		flatShading(&object, points);
		points = matrixMult(EM, points);	// �ഫeye space
		points = matrixMult(PM, points);	// �ഫprojection space
		//clipping(&object, points);
		// perspective divide
		for (int i = 0; i < points[0].size(); i++) {
			float w = points[3][i];
			points[0][i] = points[0][i] / w;
			points[1][i] = points[1][i] / w;
			points[2][i] = points[2][i] / w;
			points[3][i] = points[3][i] / w;
		}
		vector<vector<float>> WVM = creatWVMatrix();
		points = matrixMult(WVM, points);
		//object.clipList = clipWVM(WVM, object.clipList);
		drawObject(points, object.faceList, object.areaVisible, object.clipList, object.areaColor);
	}
	for (int i = (vXmin + 1) * (winWidth / 2); i < (vXmax + 1) * (winWidth / 2); i++) {
		for (int j = (vYmin + 1) * (winHeight / 2); j < (vYmax + 1) * (winHeight / 2); j++) {
			pixelR = cBuffer[i][j].r;
			pixelG = cBuffer[i][j].g;
			pixelB = cBuffer[i][j].b;
			drawDot(i, j);
		}
	}
	glFlush();
}

// �bprojection space�Adivide�e��clipping
void clipping(Object* object, vector<vector<float>> points) {
	vector<vector<int>> faceList = (*object).faceList;		// �P�_�C�ӭ����C�ӽu�q�O�_�nclipping
	vector<float> result;
	float p1, p2, x1, y1, z1, w1, x2, y2, z2, w2;
	for (int i = 0; i < faceList.size(); i++) {
		for (int j = 1; j < faceList[i].size(); j++) {
			p1 = faceList[i][j - 1];
			p2 = faceList[i][j];
			x1 = points[0][p1];
			y1 = points[1][p1];
			z1 = points[2][p1];
			w1 = points[3][p1];
			x2 = points[0][p2];
			y2 = points[1][p2];
			z2 = points[2][p2];
			w2 = points[3][p2];
			result = checkClip(x1, y1, z1, w1, x2, y2, z2, w2);		// �^�Ǫ�result[12]��������I�����A�A�H���P�_�u���B�z�覡
			Clipper clipper;
			float t1 = -1, t2 = -1, temp;
			if (isInsidePoly(x1, y1, z1, w1, 0, 0, 0, 0, 0, 6) && isInsidePoly(x2, y2, z2, w2, 0, 0, 0, 0, 0, 6)) {
				clipper.actType = keep;
			}
			else {
				for (int k = 0; k < 6; k++) {
					if (result[k] >= 0 && result[k + 6] < 0) {
						temp = result[k] / (result[k] - result[k + 6]);
						if (isInsidePoly(x1, y1, z1, w1, x2, y2, z2, w2, temp, k)) {
							t1 = temp;
						}
					}
					else if (result[k] < 0 && result[k + 6] >= 0) {
						temp = result[k + 6] / (result[k + 6] - result[k]);
						if (isInsidePoly(x2, y2, z2, w2, x1, y1, z1, w1, temp, k)) {
							t2 = temp;
						}
					}
				}

				if (t1 == -1 && t2 == -1) {
					clipper.actType = reject;
				}
				else if (t2 == -1) {
					clipper.actType = clip2;
				}
				else if (t1 == -1) {
					clipper.actType = clip1;
				}
				else if (t1 != -1 && t2 != -1) {
					clipper.actType = clipBoth;
				}
			}

			if (clipper.actType == clip2 || clipper.actType == clipBoth) {
				clipper.clipX2 = x1 + t1 * (x2 - x1);
				clipper.clipX2 /= w1 + t1 * (w2 - w1);

				clipper.clipY2 = y1 + t1 * (y2 - y1);
				clipper.clipY2 /= w1 + t1 * (w2 - w1);

				clipper.clipZ2 = z1 + t1 * (z2 - z1);
				clipper.clipZ2 /= w1 + t1 * (w2 - w1);
			}
			if (clipper.actType == clip1 || clipper.actType == clipBoth) {
				clipper.clipX = x2 + t2 * (x1 - x2);
				clipper.clipX /= w2 + t2 * (w1 - w2);

				clipper.clipY = y2 + t2 * (y1 - y2);
				clipper.clipY /= w2 + t2 * (w1 - w2);

				clipper.clipZ = z2 + t2 * (z1 - z2);
				clipper.clipZ /= w2 + t2 * (w1 - w2);
			}
			clipper.clipW = 1;
			clipper.clipW2 = 1;
			(*object).clipList[i][j - 1] = clipper;
		}
	}
}

// �^��clip�������
vector<float> checkClip(float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2) {
	vector<float> result(12, 0);
	result[0] = w1 + x1, result[6] = w2 + x2;
	result[1] = w1 - x1, result[7] = w2 - x2;
	result[2] = w1 + y1, result[8] = w2 + y2;
	result[3] = w1 - y1, result[9] = w2 - y2;
	result[4] = z1, result[10] = z2;
	result[5] = w1 - z1, result[11] = w2 - z2;

	return result;
}

// �ˬd�O�_����I�O�b�i���d�򤺡Ak�N����ӧP�_��
bool isInsidePoly(float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, float t, float k) {
	float w = w1 + t * (w2 - w1);
	float x = (x1 + t * (x2 - x1)) / w;
	float y = (y1 + t * (y2 - y1)) / w;
	float z = (z1 + t * (z2 - z1)) / w;

	if (x >= -1 && x <= 1 && y >= -1 && y <= 1 && z >= 0 && z <= 1 && k == 6)
		return true;
	if (k == 0 && y >= -1 && y <= 1 && z >= 0 && z <= 1)
		return true;
	else if (k == 1 && y >= -1 && y <= 1 && z >= 0 && z <= 1)
		return true;
	else if (k == 2 && x >= -1 && x <= 1 && z >= 0 && z <= 1)
		return true;
	else if (k == 3 && x >= -1 && x <= 1 && z >= 0 && z <= 1)
		return true;
	else if (k == 4 && x >= -1 && x <= 1 && y >= -1 && y <= 1)
		return true;
	else if (k == 5 && x >= -1 && x <= 1 && y >= -1 && y <= 1)
		return true;
	return false;
}

// clipPoint���WWVM
vector<vector<Clipper>> clipWVM(vector<vector<float>> WVM, vector<vector<Clipper>> clipList) {
	Clipper clipper;
	vector<vector<float>> temp(4, vector<float>(2, 0));
	for (int i = 0; i < clipList.size(); i++) {
		for (int j = 0; j < clipList[i].size(); j++) {
			clipper = clipList[i][j];
			temp[0][0] = clipper.clipX;
			temp[1][0] = clipper.clipY;
			temp[2][0] = clipper.clipZ;
			temp[3][0] = clipper.clipW;
			temp[0][1] = clipper.clipX2;
			temp[1][1] = clipper.clipY2;
			temp[2][1] = clipper.clipZ2;
			temp[3][1] = clipper.clipW2;
			temp = matrixMult(WVM, temp);

			clipList[i][j].clipX = temp[0][0];
			clipList[i][j].clipY = temp[1][0];
			clipList[i][j].clipZ = temp[2][0];
			clipList[i][j].clipW = temp[3][0];
			clipList[i][j].clipX2 = temp[0][1];
			clipList[i][j].clipY2 = temp[1][1];
			clipList[i][j].clipZ2 = temp[2][1];
			clipList[i][j].clipW2 = temp[3][1];
		}
	}

	return clipList;
}

// ø�sviewPort
void drawViewPort() {
	clipXmin = (vXmin + 1) * (winWidth / 2);
	clipXmax = (vXmax + 1) * (winWidth / 2);
	clipYmin = (vYmin + 1) * (winHeight / 2);
	clipYmax = (vYmax + 1) * (winHeight / 2);

	for (int i = clipXmin; i <= clipXmax; i++) {
		for (int j = clipYmin; j <= clipYmax; j++) {
			zBuffer[i][j] = 1;
			cBuffer[i][j] = Color(Br, Bg, Bb);
		}
	}
	Coordinate coA(clipXmin, clipYmin);
	Coordinate coB(clipXmin, clipYmax);
	Coordinate coC(clipXmax, clipYmax);
	Coordinate coD(clipXmax, clipYmin);
	//drawLine(coA, coB);
	//drawLine(coB, coC);
	//drawLine(coC, coD);
	//drawLine(coD, coA);
	//glFlush();
}

// ø�s����
void drawObject(vector<vector<float>> points, vector<vector<int>> faceList, bool* areaVisible, vector<vector<Clipper>> clipList, Color* areaColor) {
	Clipper clipper;

	for (int i = 0; i < faceList.size(); i++) {
		float maxX = -1, maxY = -1, minX = 1, minY = 1;	// �����C�ӭ��i��|�e�쪺�i��d��
		vector<Point3D> finalPoints;
		pixelR = areaColor[i].r;
		pixelG = areaColor[i].g;
		pixelB = areaColor[i].b;
		//cout << i << " " << pixelR << " " << pixelG << " " << pixelB << endl;
		/*if (!areaVisible[i] && noBackFaces) {
			continue;
		}*/
		for (int j = 1; j < faceList[i].size(); j++) {
			int p1 = faceList[i][j - 1], p2 = faceList[i][j];
			clipper = clipList[i][j - 1];
			Coordinate coA(0, 0), coB(0, 0);
			Point3D coAA(0, 0, 0, 0), coBB(0, 0, 0, 0);

			if (j == 1) {			// �p�⥭����{��
				int p3 = faceList[i][2];
				Point3D v1(points[0][p2] - points[0][p1], points[1][p2] - points[1][p1], points[2][p2] - points[2][p1], 0);
				Point3D v2(points[0][p3] - points[0][p1], points[1][p3] - points[1][p1], points[2][p3] - points[2][p1], 0);
				Point3D normal = vectorCross(v2, v1);
				float x0 = points[0][p1], y0 = points[1][p1], z0 = points[2][p1];
				pA = normal.x, pB = normal.y, pC = normal.z;
				pD = -(pA * x0 + pB * y0 + pC * z0);
			}

			if (clipper.actType == reject) {
				continue;
			}
			else if (clipper.actType == keep) {
				coA = Coordinate((points[0][p1] + 1) * (winWidth / 2), (points[1][p1] + 1) * (winHeight / 2));
				coAA.x = points[0][p1];
				coAA.y = points[1][p1];
				coB = Coordinate((points[0][p2] + 1) * (winWidth / 2), (points[1][p2] + 1) * (winHeight / 2));
				coBB.x = points[0][p2];
				coBB.y = points[1][p2];
			}
			else {
				if (clipper.actType == clip1) {
					coA = Coordinate((clipper.clipX + 1) * (winWidth / 2), (clipper.clipY + 1) * (winHeight / 2));
					coAA.x = clipper.clipX;
					coAA.y = clipper.clipY;
					coB = Coordinate((points[0][p2] + 1) * (winWidth / 2), (points[1][p2] + 1) * (winHeight / 2));
					coBB.x = points[0][p2];
					coBB.y = points[1][p2];
				}
				else if (clipper.actType == clip2) {
					coA = Coordinate((points[0][p1] + 1) * (winWidth / 2), (points[1][p1] + 1) * (winHeight / 2));
					coAA.x = points[0][p1];
					coAA.y = points[1][p1];
					coB = Coordinate((clipper.clipX2 + 1) * (winWidth / 2), (clipper.clipY2 + 1) * (winHeight / 2));
					coBB.x = clipper.clipX2;
					coBB.y = clipper.clipY2;
				}
				else if (clipper.actType == clipBoth) {
					coA = Coordinate((clipper.clipX + 1) * (winWidth / 2), (clipper.clipY + 1) * (winHeight / 2));
					coAA.x = clipper.clipX;
					coAA.y = clipper.clipY;
					coB = Coordinate((clipper.clipX2 + 1) * (winWidth / 2), (clipper.clipY2 + 1) * (winHeight / 2));
					coBB.x = clipper.clipX2;
					coBB.y = clipper.clipY2;
				}
			}
			//drawLine(coA, coB);
			finalPoints.push_back(coAA);
			finalPoints.push_back(coBB);
			if (coAA.x > maxX || coBB.x > maxX)
				maxX = (coAA.x > coBB.x) ? coAA.x : coBB.x;
			if (coAA.y > maxY || coBB.y > maxY)
				maxY = (coAA.y > coBB.y) ? coAA.y : coBB.y;
			if (coAA.x < minX || coBB.x < minX)
				minX = (coAA.x < coBB.x) ? coAA.x : coBB.x;
			if (coAA.y < minY || coBB.y < minY)
				minY = (coAA.y < coBB.y) ? coAA.y : coBB.y;
		}
		for (vector<Point3D>::iterator it = finalPoints.begin(); it != finalPoints.end(); it++) {
			for (vector<Point3D>::iterator itt = it + 1; itt != finalPoints.end(); ) {
				if (it->x == itt->x && it->y == itt->y) {
					itt = finalPoints.erase(itt);
				}
				else
					itt++;
			}
		}
		finalPoints.push_back(finalPoints[0]);
		coloring(finalPoints, maxX, maxY, minX, minY);
	}
	glFlush();
}

// Flat Shading
void flatShading(Object* object, vector<vector<float>> points) {
	vector<vector<int>> faceList = (*object).faceList;
	int p1, p2, p3;
	for (int i = 0; i < faceList.size(); i++) {
		p1 = faceList[i][0];
		p2 = faceList[i][1];
		p3 = faceList[i][2];
		Point3D v1(points[0][p2] - points[0][p1], points[1][p2] - points[1][p1], points[2][p2] - points[2][p1], 0);
		Point3D v2(points[0][p3] - points[0][p1], points[1][p3] - points[1][p1], points[2][p3] - points[2][p1], 0);
		Point3D normalV = vectorNormalize(vectorCross(v2, v1));

		// �p��illumination for RGB
		float Ir = illuminate(1, object, points, normalV, p1);
		float Ig = illuminate(2, object, points, normalV, p1);
		float Ib = illuminate(3, object, points, normalV, p1);

		//cout << Ir << " " << Ig << " " << Ib << endl;
		// �x�s�o�ӭ���Color
		(*object).areaColor[i] = Color(Ir, Ig, Ib);
	}
}

// �e�I�禡(float��ڤW�|�Q�|�ˤ��J)
void drawDot(float x, float y) {
	glBegin(GL_POINTS);
	glColor3f(pixelR, pixelG, pixelB);
	glVertex2f(x, y);
	glEnd();
}

// �⭱�W��
void coloring(vector<Point3D> points, float maxX, float maxY, float minX, float minY) {
	// ø�s�ѤW��U�A�ѥ���k
	for (float i = maxY; i >= minY; i -= (0.5 / winHeight)) {
		bool first = true;
		for (float j = minX; j <= maxX; j += (0.5 / winWidth)) {
			if (isInsidePoly2D(j, i, points)) {
				float z = -((pA * j + pB * i + pD) / pC);
				//cout << j << maxX << i << minY << "z:" << z << endl;
				int jj = 0, ii = 0;
				ii = (i + 1) * winHeight / 2;
				if (first)
					jj = ceil((j + 1) * winWidth / 2), first = false;
				else
					jj = floor((j + 1) * winWidth / 2);
				if (z < zBuffer[jj][ii] && z >= 0) {
					zBuffer[jj][ii] = z;
					cBuffer[jj][ii] = Color(pixelR, pixelG, pixelB);
				}
				//drawDot(j, i);
			}
		}
	}
}

// �O�_�bpolygon��
bool isInsidePoly2D(float x, float y, vector<Point3D> points) {
	float previousCheckValue = 0;
	for (int i = 1; i < points.size(); i++) {
		Point3D coA = points[i - 1];
		Point3D coB = points[i];
		float checkValue = ((x - coA.x) * (coB.y - coA.y) - (coB.x - coA.x) * (y - coA.y));
		if (previousCheckValue * checkValue < 0 && i != 1) {	// �Y�I�S�����b�u������Υk��
			return false;
		}
		previousCheckValue = checkValue;
	}
	return true;
}

// �e�u�禡
void drawLine(Coordinate coA, Coordinate coB) {
	if (coA.x > coB.x) {	// ���u���O�q���V�k�e
		Coordinate temp = coB;
		coB = coA;
		coA = temp;
	}
	if (coA.x == coB.x) {	// �p�G��y�Ъ�x�b�ۦP
		if (coA.y > coB.y) {
			for (int i = coA.y; i >= coB.y; i--) {
				drawDot(coA.x, i);
			}
		}
		else {
			for (int i = coA.y; i <= coB.y; i++) {
				drawDot(coA.x, i);
			}
		}
		return;
	}
	else if (coA.y == coB.y) {	// �p�G��y�Ъ�y�b�ۦP
		for (int i = coA.x; i <= coB.x; i++) {
			drawDot(i, coA.y);
		}
		return;
	}

	// �P�_���Ѽ�: d = ax + by + c
	float a = coB.y - coA.y;
	float b = coA.x - coB.x;
	int c = coA.y * (coB.x - coA.x) - coB.x * (coB.y - coA.y);
	int d = 0, incA = 0, incB = 0;	// incA�MincB�N���P����V��d new�ܤƶq
									// �ײv
	float m = a / -b;

	if (m > 1) {
		d = a + 2 * b;		// ��l��d
		incA = 2 * (a + b);
		incB = 2 * b;

		while (coA.x <= coB.x) {
			drawDot(coA.x, coA.y);
			if (coA.x == coB.x && coA.y == coB.y)
				break;
			if (d <= 0) {
				coA.x++, coA.y++;
				d += incA;
			}
			else {
				coA.y++;
				d += incB;
			}
		}
	}
	else if (m <= 1 && m > 0) {
		d = 2 * a + b;		// ��l��d
		incA = 2 * a;
		incB = 2 * (a + b);

		while (coA.x <= coB.x) {
			drawDot(coA.x, coA.y);
			if (coA.x == coB.x && coA.y == coB.y)
				break;
			if (d <= 0) {
				coA.x++;
				d += incA;
			}
			else {
				coA.x++, coA.y++;
				d += incB;
			}
		}
	}
	else if (m < 0 && m >= -1) {
		d = 2 * a - b;		// ��l��d
		incA = 2 * a;
		incB = 2 * (a - b);

		while (coA.x <= coB.x) {
			drawDot(coA.x, coA.y);
			if (coA.x == coB.x && coA.y == coB.y)
				break;
			if (d >= 0) {
				coA.x++;
				d += incA;
			}
			else {
				coA.x++, coA.y--;
				d += incB;
			}
		}
	}
	else {
		d = a - 2 * b;		// ��l��d
		incA = 2 * (a - b);
		incB = -2 * b;

		while (coA.x <= coB.x) {
			drawDot(coA.x, coA.y);
			if (coA.x == coB.x && coA.y == coB.y)
				break;
			if (d >= 0) {
				coA.x++, coA.y--;
				d += incA;
			}
			else {
				coA.y--;
				d += incB;
			}
		}
	}
}

// ���ͤ@��world space��viewport���x�} 
vector<vector<float>> creatWVMatrix() {
	vector<vector<float>> WVM(4, vector<float>(4, 0));
	vector<vector<float>> SM(4, vector<float>(4, 0));
	vector<vector<float>> TransM(4, vector<float>(4, 0));
	float Sx, Sy;
	// ��l�Ưx�}
	WVM[0][0] = 1;
	WVM[0][3] = -wXmin;
	WVM[1][1] = 1;
	WVM[1][3] = -wYmin;
	WVM[2][2] = 1;
	WVM[3][3] = 1;

	TransM[0][0] = 1;
	TransM[0][3] = vXmin;
	TransM[1][1] = 1;
	TransM[1][3] = vYmin;
	TransM[2][2] = 1;
	TransM[3][3] = 1;

	// �p���Y��x�}
	Sx = (vXmax - vXmin) / (wXmax - wXmin);
	Sy = (vYmax - vYmin) / (wYmax - wYmin);
	SM[0][0] = Sx;
	SM[1][1] = Sy;
	SM[2][2] = 1;
	SM[3][3] = 1;

	WVM = matrixMult(SM, WVM);
	WVM = matrixMult(TransM, WVM);

	return WVM;
}

// �x�}�ۭ�
vector<vector<float>> matrixMult(vector<vector<float>> matrixA, vector<vector<float>> matrixB) {
	int rowA = matrixA.size(), rowB = matrixB.size();
	int colA = matrixA[0].size(), colB = matrixB[0].size();
	if (colA != rowB) {
		cout << "matrix can't multiply!" << endl;
		exit(1);
	}
	vector<vector<float>> result(rowA, vector<float>(colB));
	int index;
	float sum;
	for (int i = 0; i < rowA; i++) {
		for (int j = 0; j < colB; j++) {
			index = 0, sum = 0;
			while (index < colA) {
				sum += matrixA[i][index] * matrixB[index][j];
				index++;
			}
			result[i][j] = sum;
		}
	}
	return result;
}

// �V�q�~�n
Point3D vectorCross(Point3D v1, Point3D v2) {
	Point3D result(0, 0, 0, 0);
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;

	return result;
}

// �V�q���n
float vectorDot(Point3D v1, Point3D v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// �V�q���W�Ʀ����׬�1
Point3D vectorNormalize(Point3D v1) {
	float length = sqrt(pow(v1.x, 2) + pow(v1.y, 2) + pow(v1.z, 2));
	v1.x = v1.x / length;
	v1.y = v1.y / length;
	v1.z = v1.z / length;
	return v1;
}

// �x�s����
void storeObject() {
	Object temp = object;
	temp.points = matrixMult(TM, temp.points);
	objects.push_back(temp);
}

// �ˬdbackFace�A�T�{�C�ӭ����k�V�q�Peye vector������
void backFaceCheck(Object* object, vector<vector<float>> points) {
	int p1, p2, p3;
	for (int i = 0; i < (*object).faceList.size(); i++) {
		p1 = (*object).faceList[i][0];
		p2 = (*object).faceList[i][1];
		p3 = (*object).faceList[i][2];
		Point3D v1(points[0][p2] - points[0][p1], points[1][p2] - points[1][p1], points[2][p2] - points[2][p1], 0);
		Point3D v2(points[0][p3] - points[0][p1], points[1][p3] - points[1][p1], points[2][p3] - points[2][p1], 0);
		Point3D normalV = vectorCross(v2, v1);
		Point3D eyeV(eyeX - points[0][p1], eyeY - points[1][p1], eyeZ - points[2][p1], 0);

		if (findAngle(normalV, eyeV) <= 90) {
			(*object).areaVisible[i] = true;
		}
	}
}

// �p���V�q�������A�^�ǳ��O����
float findAngle(Point3D v1, Point3D v2) {
	float lenV1 = sqrt(pow(v1.x, 2) + pow(v1.y, 2) + pow(v1.z, 2));
	float lenV2 = sqrt(pow(v2.x, 2) + pow(v2.y, 2) + pow(v2.z, 2));
	float value = (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) / (lenV1 * lenV2);

	return acos(value) * 180 / PI;
}

// �p��target point��illumination�A�ë��w�n�p��RGB������
float illuminate(int rgbType, Object* object, vector<vector<float>> points, Point3D normalV, int targetPoint) {
	float Or = (*object).Or, Og = (*object).Og, Ob = (*object).Ob;
	float Kd = (*object).Kd, Ks = (*object).Ks, nPow = (*object).nPow;
	float ambient = 0, diffuse = 0, specular = 0;
	if (rgbType == 1) {
		ambient = KaIar * Or;

		for (int i = 0; i < lights.size(); i++) {
			// ���olight vector
			Point3D lightV(lights[i].LightPos.x - points[0][targetPoint], lights[i].LightPos.y - points[1][targetPoint], lights[i].LightPos.z - points[2][targetPoint], 0);
			lightV = vectorNormalize(lightV);
			float value = vectorDot(normalV, lightV);
			if (value < 0)
				value = 0;
			diffuse += Kd * lights[i].Ipr * value;

			// ���oeye vector
			Point3D eyeV(eyeX - points[0][targetPoint], eyeY - points[1][targetPoint], eyeZ - points[2][targetPoint], 0);
			eyeV = vectorNormalize(eyeV);
			float con = 2 * vectorDot(normalV, lightV);

			// ���oreflection vector
			Point3D reflectV(con * normalV.x - lightV.x, con * normalV.y - lightV.y, con * normalV.z - lightV.z, 0);
			reflectV = vectorNormalize(reflectV);
			value = vectorDot(reflectV, eyeV);
			if (value < 0)
				value = 0;
			specular += Ks * lights[i].Ipr * pow(value, nPow);
		}
		diffuse *= Or;
	}
	else if (rgbType == 2) {
		ambient = KaIag * Og;

		for (int i = 0; i < lights.size(); i++) {
			// ���olight vector
			Point3D lightV(lights[i].LightPos.x - points[0][targetPoint], lights[i].LightPos.y - points[1][targetPoint], lights[i].LightPos.z - points[2][targetPoint], 0);
			lightV = vectorNormalize(lightV);
			float value = vectorDot(normalV, lightV);
			if (value < 0)
				value = 0;
			diffuse += Kd * lights[i].Ipg * value;

			// ���oeye vector
			Point3D eyeV(eyeX - points[0][targetPoint], eyeY - points[1][targetPoint], eyeZ - points[2][targetPoint], 0);
			eyeV = vectorNormalize(eyeV);
			float con = 2 * vectorDot(normalV, lightV);

			// ���oreflection vector
			Point3D reflectV(con * normalV.x - lightV.x, con * normalV.y - lightV.y, con * normalV.z - lightV.z, 0);
			reflectV = vectorNormalize(reflectV);
			value = vectorDot(reflectV, eyeV);
			if (value < 0)
				value = 0;
			specular += Ks * lights[i].Ipg * pow(value, nPow);
		}
		diffuse *= Og;
	}
	else if (rgbType == 3) {
		ambient = KaIab * Ob;

		for (int i = 0; i < lights.size(); i++) {
			// ���olight vector
			Point3D lightV(lights[i].LightPos.x - points[0][targetPoint], lights[i].LightPos.y - points[1][targetPoint], lights[i].LightPos.z - points[2][targetPoint], 0);
			lightV = vectorNormalize(lightV);
			float value = vectorDot(normalV, lightV);
			if (value < 0)
				value = 0;
			diffuse += Kd * lights[i].Ipb * value;

			// ���oeye vector
			Point3D eyeV(eyeX - points[0][targetPoint], eyeY - points[1][targetPoint], eyeZ - points[2][targetPoint], 0);
			eyeV = vectorNormalize(eyeV);
			float con = 2 * vectorDot(normalV, lightV);

			// ���oreflection vector
			Point3D reflectV(con * normalV.x - lightV.x, con * normalV.y - lightV.y, con * normalV.z - lightV.z, 0);
			reflectV = vectorNormalize(reflectV);
			value = vectorDot(reflectV, eyeV);
			if (value < 0)
				value = 0;
			specular += Ks * lights[i].Ipb * pow(value, nPow);
		}
		diffuse *= Ob;
	}

	return ambient + diffuse + specular;
}
