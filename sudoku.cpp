#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
using namespace std;

#define MAX_COL 750
#define MAX_ROW 750

#define SQ_OFFSET 0
#define RW_OFFSET 81
#define CL_OFFSET 162
#define BX_OFFSET 243

struct str_node {
	struct str_node * Header;

	struct str_node * Left;
	struct str_node * Right;
	struct str_node * Up;
	struct str_node * Down;

	char IDName;
	int  IDNum;
};

int nCol;
int nRow;
struct str_node Matrix[MAX_COL][MAX_ROW];
struct str_node Root;
struct str_node *RootNode = &Root;
struct str_node *RowHeader[MAX_ROW];
char Data[MAX_COL][MAX_ROW];
int Result[MAX_ROW];
int nResult = 0;
char Finished;
int alreadyFill;

// --> Initialisation functions
inline int dataLeft(int i) { return i - 1<0 ? nCol - 1 : i - 1; }
inline int dataRight(int i) { return (i + 1) % nCol; }
inline int dataUp(int i) { return i - 1<0 ? nRow - 1 : i - 1; }
inline int dataDown(int i) { return (i + 1) % nRow; }

void CreateMatrix(void) {
	int i, j;
	//Build toroidal linklist matrix according to data bitmap
	for (int c = 0; c < nCol; c++) {
		for (int r = 0; r < nRow; r++) {
			if (Data[c][r] != 0) {
				// Left pointer
				i = c; j = r;
				do { i = dataLeft(i); } while (Data[i][j] == 0);
				Matrix[c][r].Left = &Matrix[i][j];
				// Right pointer
				i = c; j = r;
				do { i = dataRight(i); } while (Data[i][j] == 0);
				Matrix[c][r].Right = &Matrix[i][j];
				// Up pointer
				i = c; j = r;
				do { j = dataUp(j); } while (Data[i][j] == 0);
				Matrix[c][r].Up = &Matrix[i][j];
				// Down pointer
				i = c; j = r;
				do { j = dataDown(j); } while (Data[i][j] == 0);
				Matrix[c][r].Down = &Matrix[i][j];
				// Header pointer
				Matrix[c][r].Header = &Matrix[c][nRow - 1];
				Matrix[c][r].IDNum = r;
				//Row Header
				RowHeader[r] = &Matrix[c][r];
			}
		}
	}
	for (int a = 0; a<nCol; a++) {
		Matrix[a][nRow - 1].IDName = 'C';
		Matrix[a][nRow - 1].IDNum = a;
	}
	//Insert root
	Root.IDName = 'R';
	Root.Left = &Matrix[nCol - 1][nRow - 1];
	Root.Right = &Matrix[0][nRow - 1];
	Matrix[nCol - 1][nRow - 1].Right = &Root;
	Matrix[0][nRow - 1].Left = &Root;
}

// --> DLX Algorithm functions

//링크를 해제하는 함수
void Cover(struct str_node *ColNode) {
	struct str_node *RowNode, *RightNode;
	ColNode->Right->Left = ColNode->Left;
	ColNode->Left->Right = ColNode->Right;
	//밑으로 탐색
	for (RowNode = ColNode->Down; RowNode != ColNode; RowNode = RowNode->Down) {
		//오른쪽 방향으로 탐색
		for (RightNode = RowNode->Right; RightNode != RowNode; RightNode = RightNode->Right) {
			//노드 연결 제거 실행, 이 때 나머지 링크들은 살려놓는다(Uncover 때 필요)
			RightNode->Up->Down = RightNode->Down;
			RightNode->Down->Up = RightNode->Up;
		}
	}
}

//해제했던 링크를 다시 살려 원상태로 복구하는 함수
void UnCover(struct str_node *ColNode) {
	struct str_node *RowNode, *LeftNode;
	//위로 탐색
	for (RowNode = ColNode->Up; RowNode != ColNode; RowNode = RowNode->Up) {
		//왼쪽 방향으로 탐색
		for (LeftNode = RowNode->Left; LeftNode != RowNode; LeftNode = LeftNode->Left) {
			//위아래 방향으로 노드 연결, Cover 때 살려놓았던 링크들로 재연결이 가능하다
			LeftNode->Up->Down = LeftNode;
			LeftNode->Down->Up = LeftNode;
		}
	}
	//양 옆 노드 연결
	ColNode->Right->Left = ColNode;
	ColNode->Left->Right = ColNode;
}

void PrintSolution(void);

void Search(int k) {
	//모든 링크를 다 돌은 경우이거나 주어진 빈칸을 다 채운 경우가 답을 찾은 경우다
	if ((RootNode->Left == RootNode && RootNode->Right == RootNode) || k == (81 - alreadyFill)) {
		cout << "----------- SOLUTION FOUND -----------" << endl;
		PrintSolution();
		Finished = 1;
		return;
	}
	struct str_node *Column = RootNode->Right;
	Cover(Column);

	struct str_node *RowNode;
	struct str_node *RightNode;
	for (RowNode = Column->Down; RowNode != Column && !Finished; RowNode = RowNode->Down) {
		// Cover를 통해 RowNode에 해당되는 링크를 모두 제거한다
		//Result에 값을 넣고 시작
		Result[nResult++] = RowNode->IDNum;
		for (RightNode = RowNode->Right; RightNode != RowNode; RightNode = RightNode->Right) {
			Cover(RightNode->Header);
		}
		Search(k + 1);
		// 만약 안되는 경우이면 이전 상태로 되돌린다(Uncover)
		for (RightNode = RowNode->Right; RightNode != RowNode; RightNode = RightNode->Right) {
			UnCover(RightNode->Header);
		}
		//안되는 경우이므로 Result 값 제거
		Result[--nResult] = 0;
	}
	UnCover(Column);
}

// --> Sudoku to Exact Cover conversion

// Functions that extract data from a given 3-digit integer index number in the format [N] [R] [C].
inline int retBox(int n, int r, int c) { return ((r / 3) * 3) + (c / 3); }
inline int retSquare(int n, int r, int c) { return r * 9 + c; }
inline int retRowNum(int n, int r, int c) { return n * 9 + r; }
inline int retColNum(int n, int r, int c) { return n * 9 + c; }
inline int retBoxNum(int n, int r, int c) { return n * 9 + retBox(n, r, c); }
// Function that get 3-digit integer index from given info
inline int makeNum(int Nb, int Rw, int Cl) { return Nb * 81 + Rw * 9 + Cl; }

void PrintSolution(void) {
	int Sudoku[9][9] = {};
	//스도쿠 배열 초기화
	for (int r = 0; r < 9; r++) {
		for (int c = 0; c < 9; c++){
			Sudoku[r][c] = -1;
		}
	}
	//스도쿠 배열 채워넣기
	for (int a = 0; a<nResult; a++) {
		int n = Result[a] / 81;
		int r = (Result[a] / 9) % 9;
		int c = Result[a] % 9;
		Sudoku[r][c] = n;
	}
	//출력
	for (int r = 0; r < 9; r++) {
		for (int c = 0; c<9; c++) {
			//horizontal lines
			if (r > 0 && r % 3 == 0 && c == 0) {
				cout << "------------------" <<endl;
			}
			cout << Sudoku[r][c] + 1 << (c % 3 == 2 ? '|' : ' ');
		}
		cout << endl;
	}
}

void BuildData(void) {
	int num;
	nCol = 324; nRow = 729 + 1;
	for (int r = 0; r<9; r++) {
		for (int c = 0; c<9; c++) {
			for (int n = 0; n<9; n++) {
				num = makeNum(n, r, c);
				Data[SQ_OFFSET + retSquare(n, r, c)][num] = 1; //Constraint 1: Only 1 per square
				Data[RW_OFFSET + retRowNum(n, r, c)][num] = 1; //Constraint 2: Only 1 of per number per Row
				Data[CL_OFFSET + retColNum(n, r, c)][num] = 1; //Constraint 3: Only 1 of per number per Column
				Data[BX_OFFSET + retBoxNum(n, r, c)][num] = 1; //Constraint 4: Only 1 of per number per Box
			}
		}
	}
	for (int c = 0; c < nCol; c++) {
		Data[c][nRow - 1] = 2;
	}

	CreateMatrix();
	for (int c = 0; c < nCol; c++) {
		if (c >= 0 && c < RW_OFFSET){
			Matrix[c][nRow - 1].IDName = 'S';
			Matrix[c][nRow - 1].IDNum = c;
		}
		else if (c >= RW_OFFSET && c < CL_OFFSET){
			Matrix[c][nRow - 1].IDName = 'R';
			Matrix[c][nRow - 1].IDNum = c - RW_OFFSET;
		}
		else if (c >= CL_OFFSET && c < BX_OFFSET){
			Matrix[c][nRow - 1].IDName = 'C';
			Matrix[c][nRow - 1].IDNum = c - CL_OFFSET;
		}
		else if (c >= BX_OFFSET && c < nCol){
			Matrix[c][nRow - 1].IDName = 'B';
			Matrix[c][nRow - 1].IDNum = c - BX_OFFSET;
		}
	}
}

inline void AddNumber(int N, int R, int C) {
	struct str_node *RowNode = RowHeader[makeNum(N, R, C)];

	Cover(RowNode->Header);
	struct str_node *RightNode;
	for (RightNode = RowNode->Right; RightNode != RowNode; RightNode = RightNode->Right) {
		Cover(RightNode->Header);
	}

	//이미 맵에 들어가 있는 경우이므로 alreadyFill 증가, Result값 대입
	alreadyFill++;
	Result[nResult++] = makeNum(N, R, C);
}

void LoadPuzzle(ifstream& file) {
	
	if (file.fail()) { 
		printf("File load fail!\n"); 
		return; 
	}
	char temp;
	for (int r = 0; r < 9; r++) {
		for (int c = 0; c < 9; c++) {
			file >> temp;
			if (temp >= '1' && temp <= '9')
				AddNumber((temp - '0') - 1, r, c);
		}
	}
}

int main(void) {
	ifstream file;
	int numCase;
	file.open("input.txt");
	file >> numCase;

	for (int i = 0; i < numCase; i++){
		alreadyFill = 0, Finished = 0;
		BuildData();
		LoadPuzzle(file);
		Search(0);
	}

	file.close();
	return 0;
}