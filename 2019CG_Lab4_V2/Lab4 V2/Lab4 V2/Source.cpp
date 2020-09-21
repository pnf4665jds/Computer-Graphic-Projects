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
	cd /d D:\總資料夾\計算機圖學作業\2019CG_Lab4_106502530\Debug	2019CG_Lab4_106502530.exe Lab4A.in
*/

// 2D座標
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

// 3D座標或向量
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

// 紀錄顏色RGB數值
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

// 線段狀態
enum type { keep, reject, clip1, clip2, clipBoth };

// 用來記錄線段的clip狀況
class Clipper {
public:
	type actType = keep;	// 線段狀況	
	float clipX = 0, clipY = 0, clipZ = 0, clipW = 1;	// clip後的座標
	float clipX2 = 0, clipY2 = 0, clipZ2 = 0, clipW2 = 1;
};

// 紀錄光源的數據
class Light {
public:
	float Ipr, Ipg, Ipb;
	Point3D LightPos = Point3D(0, 0, 0, 1);
};

// 圖形物件
class Object {
public:
	vector<vector<float>> points;
	vector<vector<int>> faceList;	// 紀錄每個面的組成頂點
	vector<vector<Clipper>> clipList; // 紀錄每個面的線段clip狀況
	bool* areaVisible;	// 是否能看到這個面
	Color* areaColor;	// 紀錄每個面在flat shading下的顏色
	int pointNum, areaNum;
	int n1, n2;
	float Kd, Ks, nPow;	// shading相關參數
	float Or, Og, Ob;

	// 初始化
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

	// 記錄點
	void AddPoint(Point3D p) {
		points[0].push_back(p.x);
		points[1].push_back(p.y);
		points[2].push_back(p.z);
		points[3].push_back(p.w);
	}

	// 重置areaVisible array
	void resetVisible() {
		for (int i = 0; i < areaNum; i++)
			areaVisible[i] = false;
	}

	// 更新FaceList
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


// 函式宣告
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

// 變數宣告
int winWidth, winHeight;
fstream file, ascFile;
string fileName, previousName = "";		// in檔名與上個ASC檔名
string str, ascStr;
regex tokenRegex("[^\\s]+");	// regular expression
sregex_iterator iter, ascIter;	// iterator for regex
vector<Object> objects;
Object object = Object(0, 0);			// 物件
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
	fileName = argv[1];								// 取得Input檔名
	file.open(fileName, ios::in);
	if (!file)     //檢查檔案是否成功開啟
	{
		cerr << "Can't open file!\n";
		exit(1);     //在不正常情形下，中斷程式的執行
	}
	getline(file, str);
	iter = sregex_iterator(str.begin(), str.end(), tokenRegex);
	winWidth = stringToFloat((*iter).str());
	winHeight = stringToFloat((*++iter).str());

	glutInit(&argc, argv);							// 使用glut函式庫需要進行初始化
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);	// 設定視窗顯示模式，顏色模型和緩存，這裡是RGB顏色模型和單緩存
	glutInitWindowPosition(100, 100);				// 設定出現視窗的初始位置，左上角為原點
	glutInitWindowSize(winWidth, winHeight);		// 視窗大小
	glutCreateWindow("畫圖程式");					// 視窗名稱

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

// 讀取input檔
void readFile() {
	string token;
	resetTM();
	// 逐行讀取input並拆解成token
	while (getline(file, str)) {
		if (str[0] == '#' || str == "") {		// 讀取到註解或空行
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
			if (!ascFile)     //檢查檔案是否成功開啟
			{
				cerr << "Can't open ASC file!\n";
				exit(1);     //在不正常情形下，中斷程式的執行
			}
			if (previousName != token) {	// 若檔案跟上個檔案不同才讀取
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
			if (index > lights.size())		// 新的光源
				lights.push_back(light);
			else							// 舊的光源
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

// 讀取ASC檔的資料
void readASCFile() {
	bool getPointNum = true;
	int pointNum, areaNum;
	float x, y, z;
	while (getline(ascFile, ascStr)) {
		if (ascStr == "") {		// 讀取空行
			continue;
		}
		ascIter = sregex_iterator(ascStr.begin(), ascStr.end(), tokenRegex);
		if (getPointNum) {		// 讀取點與面的數量
			getPointNum = false;
			pointNum = stringToFloat((*ascIter).str());
			areaNum = stringToFloat((*++ascIter).str());
			object = Object(pointNum, areaNum);
			continue;
		}
		if (pointNum > 0) {		// 儲存點座標
			pointNum--;
			x = stringToFloat((*ascIter).str());
			y = stringToFloat((*++ascIter).str());
			z = stringToFloat((*++ascIter).str());
			//cout << x << " " << y << " " << z << endl;
			object.AddPoint(Point3D(x, y, z, 1));
		}
		else if (areaNum > 0) {	// 儲存adjList
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

// 重置transfom matrix
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

// 產生scale matrix並計算TM
void scale(float sX, float sY, float sZ) {
	vector<vector<float>> SM(4, vector<float>(4, 0));
	SM[0][0] = sX;
	SM[1][1] = sY;
	SM[2][2] = sZ;
	SM[3][3] = 1;

	TM = matrixMult(SM, TM);
	vector<vector<float>>().swap(SM);
}

// 產生rotate matrix並計算TM
void rotate(float rX, float rY, float rZ) {
	vector<vector<float>> RM1(4, vector<float>(4, 0)), RM2(4, vector<float>(4, 0)), RM3(4, vector<float>(4, 0));
	float radX = rX * PI / 180;		// 轉換成弧度單位
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

// 產生translate matrix並計算TM
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

// 計算world space to eye space matrix
void computeEM() {
	vector<vector<float>> TM(4, vector<float>(4, 0));
	vector<vector<float>> GRM(4, vector<float>(4, 0));
	vector<vector<float>> MM(4, vector<float>(4, 0));	// mirror matrix
	vector<vector<float>> RM(4, vector<float>(4, 0));	// tilt matrix
	float degree = tilt * PI / 180;
	Point3D vectorX(1, 0, 0, 0), vectorY(0, 1, 0, 0), vectorZ(0, 0, 1, 0);
	// 初始化矩陣TM
	TM[0][0] = 1, TM[1][1] = 1, TM[2][2] = 1, TM[3][3] = 1;
	TM[0][3] = -eyeX, TM[1][3] = -eyeY, TM[2][3] = -eyeZ;
	// 初始化矩陣MM
	MM[0][0] = -1, MM[1][1] = 1, MM[2][2] = 1, MM[3][3] = 1;
	// 初始化矩陣RM
	RM[0][0] = cos(degree), RM[0][1] = sin(degree), RM[2][2] = 1;
	RM[1][0] = -sin(degree), RM[1][1] = cos(degree), RM[3][3] = 1;
	// 計算矩陣GRM
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
	// 計算EM
	EM = matrixMult(GRM, TM);
	EM = matrixMult(MM, EM);
	EM = matrixMult(RM, EM);
}

// 計算project matrix
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

// 字串轉換成浮點數
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

// 對點進行轉換
void pipelining() {
	for (int k = 0; k < objects.size(); k++) {
		Object object = objects[k];
		vector<vector<float>> points = object.points;
		//object.resetVisible();
		//if (noBackFaces)
			//backFaceCheck(&object, points);		// 注意! 此時的X軸座標是相反的
		flatShading(&object, points);
		points = matrixMult(EM, points);	// 轉換eye space
		points = matrixMult(PM, points);	// 轉換projection space
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

// 在projection space，divide前做clipping
void clipping(Object* object, vector<vector<float>> points) {
	vector<vector<int>> faceList = (*object).faceList;		// 判斷每個面的每個線段是否要clipping
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
			result = checkClip(x1, y1, z1, w1, x2, y2, z2, w2);		// 回傳的result[12]紀錄兩端點的狀態，以此判斷線的處理方式
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

// 回傳clip相關資料
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

// 檢查是否交錯點是在可視範圍內，k代表哪個判斷式
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

// clipPoint乘上WVM
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

// 繪製viewPort
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

// 繪製物件
void drawObject(vector<vector<float>> points, vector<vector<int>> faceList, bool* areaVisible, vector<vector<Clipper>> clipList, Color* areaColor) {
	Clipper clipper;

	for (int i = 0; i < faceList.size(); i++) {
		float maxX = -1, maxY = -1, minX = 1, minY = 1;	// 紀錄每個面可能會畫到的可能範圍
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

			if (j == 1) {			// 計算平面方程式
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

		// 計算illumination for RGB
		float Ir = illuminate(1, object, points, normalV, p1);
		float Ig = illuminate(2, object, points, normalV, p1);
		float Ib = illuminate(3, object, points, normalV, p1);

		//cout << Ir << " " << Ig << " " << Ib << endl;
		// 儲存這個面的Color
		(*object).areaColor[i] = Color(Ir, Ig, Ib);
	}
}

// 畫點函式(float實際上會被四捨五入)
void drawDot(float x, float y) {
	glBegin(GL_POINTS);
	glColor3f(pixelR, pixelG, pixelB);
	glVertex2f(x, y);
	glEnd();
}

// 把面上色
void coloring(vector<Point3D> points, float maxX, float maxY, float minX, float minY) {
	// 繪製由上到下，由左到右
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

// 是否在polygon內
bool isInsidePoly2D(float x, float y, vector<Point3D> points) {
	float previousCheckValue = 0;
	for (int i = 1; i < points.size(); i++) {
		Point3D coA = points[i - 1];
		Point3D coB = points[i];
		float checkValue = ((x - coA.x) * (coB.y - coA.y) - (coB.x - coA.x) * (y - coA.y));
		if (previousCheckValue * checkValue < 0 && i != 1) {	// 若點沒有都在線的左邊或右邊
			return false;
		}
		previousCheckValue = checkValue;
	}
	return true;
}

// 畫線函式
void drawLine(Coordinate coA, Coordinate coB) {
	if (coA.x > coB.x) {	// 讓線都是從左向右畫
		Coordinate temp = coB;
		coB = coA;
		coA = temp;
	}
	if (coA.x == coB.x) {	// 如果兩座標的x軸相同
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
	else if (coA.y == coB.y) {	// 如果兩座標的y軸相同
		for (int i = coA.x; i <= coB.x; i++) {
			drawDot(i, coA.y);
		}
		return;
	}

	// 判斷式參數: d = ax + by + c
	float a = coB.y - coA.y;
	float b = coA.x - coB.x;
	int c = coA.y * (coB.x - coA.x) - coB.x * (coB.y - coA.y);
	int d = 0, incA = 0, incB = 0;	// incA和incB代表不同的方向之d new變化量
									// 斜率
	float m = a / -b;

	if (m > 1) {
		d = a + 2 * b;		// 初始化d
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
		d = 2 * a + b;		// 初始化d
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
		d = 2 * a - b;		// 初始化d
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
		d = a - 2 * b;		// 初始化d
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

// 產生一個world space轉viewport的矩陣 
vector<vector<float>> creatWVMatrix() {
	vector<vector<float>> WVM(4, vector<float>(4, 0));
	vector<vector<float>> SM(4, vector<float>(4, 0));
	vector<vector<float>> TransM(4, vector<float>(4, 0));
	float Sx, Sy;
	// 初始化矩陣
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

	// 計算縮放矩陣
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

// 矩陣相乘
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

// 向量外積
Point3D vectorCross(Point3D v1, Point3D v2) {
	Point3D result(0, 0, 0, 0);
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;

	return result;
}

// 向量內積
float vectorDot(Point3D v1, Point3D v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// 向量正規化成長度為1
Point3D vectorNormalize(Point3D v1) {
	float length = sqrt(pow(v1.x, 2) + pow(v1.y, 2) + pow(v1.z, 2));
	v1.x = v1.x / length;
	v1.y = v1.y / length;
	v1.z = v1.z / length;
	return v1;
}

// 儲存物件
void storeObject() {
	Object temp = object;
	temp.points = matrixMult(TM, temp.points);
	objects.push_back(temp);
}

// 檢查backFace，確認每個面的法向量與eye vector的夾角
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

// 計算兩向量的夾角，回傳單位是角度
float findAngle(Point3D v1, Point3D v2) {
	float lenV1 = sqrt(pow(v1.x, 2) + pow(v1.y, 2) + pow(v1.z, 2));
	float lenV2 = sqrt(pow(v2.x, 2) + pow(v2.y, 2) + pow(v2.z, 2));
	float value = (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) / (lenV1 * lenV2);

	return acos(value) * 180 / PI;
}

// 計算target point的illumination，並指定要計算RGB的哪個
float illuminate(int rgbType, Object* object, vector<vector<float>> points, Point3D normalV, int targetPoint) {
	float Or = (*object).Or, Og = (*object).Og, Ob = (*object).Ob;
	float Kd = (*object).Kd, Ks = (*object).Ks, nPow = (*object).nPow;
	float ambient = 0, diffuse = 0, specular = 0;
	if (rgbType == 1) {
		ambient = KaIar * Or;

		for (int i = 0; i < lights.size(); i++) {
			// 取得light vector
			Point3D lightV(lights[i].LightPos.x - points[0][targetPoint], lights[i].LightPos.y - points[1][targetPoint], lights[i].LightPos.z - points[2][targetPoint], 0);
			lightV = vectorNormalize(lightV);
			float value = vectorDot(normalV, lightV);
			if (value < 0)
				value = 0;
			diffuse += Kd * lights[i].Ipr * value;

			// 取得eye vector
			Point3D eyeV(eyeX - points[0][targetPoint], eyeY - points[1][targetPoint], eyeZ - points[2][targetPoint], 0);
			eyeV = vectorNormalize(eyeV);
			float con = 2 * vectorDot(normalV, lightV);

			// 取得reflection vector
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
			// 取得light vector
			Point3D lightV(lights[i].LightPos.x - points[0][targetPoint], lights[i].LightPos.y - points[1][targetPoint], lights[i].LightPos.z - points[2][targetPoint], 0);
			lightV = vectorNormalize(lightV);
			float value = vectorDot(normalV, lightV);
			if (value < 0)
				value = 0;
			diffuse += Kd * lights[i].Ipg * value;

			// 取得eye vector
			Point3D eyeV(eyeX - points[0][targetPoint], eyeY - points[1][targetPoint], eyeZ - points[2][targetPoint], 0);
			eyeV = vectorNormalize(eyeV);
			float con = 2 * vectorDot(normalV, lightV);

			// 取得reflection vector
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
			// 取得light vector
			Point3D lightV(lights[i].LightPos.x - points[0][targetPoint], lights[i].LightPos.y - points[1][targetPoint], lights[i].LightPos.z - points[2][targetPoint], 0);
			lightV = vectorNormalize(lightV);
			float value = vectorDot(normalV, lightV);
			if (value < 0)
				value = 0;
			diffuse += Kd * lights[i].Ipb * value;

			// 取得eye vector
			Point3D eyeV(eyeX - points[0][targetPoint], eyeY - points[1][targetPoint], eyeZ - points[2][targetPoint], 0);
			eyeV = vectorNormalize(eyeV);
			float con = 2 * vectorDot(normalV, lightV);

			// 取得reflection vector
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
