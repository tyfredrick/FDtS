#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>
#include <stdio.h>
#include <wingdi.h>
#include <string.h>

#define SQUARES 200
#define ENT_COUNT 20
#define MAX_OPTS 20
int quit = 0;
int r = 255;
int g, b = 0;
int paintedFlag = 0;
int playerSpeed = 8;
wchar_t* chSl = L"Select your player character:";
RECT sillySquare;
HBRUSH OccSq;

enum Direction {
	NORTH,
	EAST,
	SOUTH,
	WEST,
	NONE
};

enum GameMode{
	PLAY,
	SELECT
};

enum PlayerCharacter{
	KANAME,
	NENEKO
};

enum GameMode mode = PLAY;

enum Direction playerDir = SOUTH;

enum PlayerCharacter playerCharacter;
struct Proj{
	int x;
	int y;
	enum Direction projDir;
};

struct GameEnt{
	int posX;
	int posY;
	int squareX;
	int squareY;
	int imgWidth;
	int imgHeight;
	int walkseq;
	enum Direction dir;
	HBITMAP image;
	HBITMAP f1;
	HBITMAP f2;
	HBITMAP f3;
	HBITMAP b1;
	HBITMAP b2;
	HBITMAP b3;
	HBITMAP l1;
	HBITMAP l2;
	HBITMAP l3;
	HBITMAP r1;
	HBITMAP r2;
	HBITMAP r3;
	HBITMAP fAtk;
	HBITMAP bAtk;
	HBITMAP lAtk;
	HBITMAP rAtk;
	LPCWSTR desc;
};

HBITMAP SKF1, SKF2, SKF3, SKB1, SKB2, SKB3, SKL1, SKL2, SKL3, SKR1, SKR2, SKR3, SNF1, SNF2, SNF3, SNB1, SNB2, SNB3, SNL1, SNL2, SNL3, SNR1, SNR2, SNR3, SKFA, SKBA, SKLA, SKRA;

struct ListNode{
	int x;
	int y;
	struct ListNode * next;
};

struct Selection{
	int originX;
	int originY;
	int optCount;
	int optBoxWidth;
	int optBoxHeight;
	int margin;
	HBRUSH optColor;
	HBRUSH selectorColor;
	int selectorX;
	int selectorY;
	int columnSize;
	LPCWSTR choiceName;
	LPCWSTR opts[MAX_OPTS];	
	int (*assocAct)();
};

struct GameEnt player;
struct GameEnt rock;
struct GameEnt* facedEnt = NULL;
struct GameEnt* entities[ENT_COUNT];
struct GameEnt EMPTY_ENTITY;
struct ListNode* listHead;
struct Selection* cs;

int lightGrey[3] = {172, 172, 172};
int darkGrey[3] = {96, 96, 96};

struct Proj projs[10];
int projsInd = 0;

struct{
	int width;
	int height;
	uint32_t *pixels;
} frame = {0};

//structures for copying pixel array to window
static BITMAPINFO frame_bitmap_info;
static HBITMAP frame_bitmap = 0;
static HDC frame_device_context = 0;

int getSquarePos(int Pstx, int Psty, RECT* sqr){
	if(Pstx < 0 || Psty < 0){
		return 0;
	}
	sqr->left = Pstx/64;
	sqr->top = Psty/64;
	sqr->right = sqr->left + 1;
	sqr->bottom = sqr->top + 1;
	return 1;
}

int getSquareEnt(struct GameEnt ent, RECT* sqr){
	if(&ent == &EMPTY_ENTITY || ent.posX < 0 || ent.posY < 0){
		return 0;
	}
	sqr->left = ent.posX/64;
	sqr->top = ent.posY/64;
	sqr->right = sqr->left + 1;
	sqr->bottom = sqr->top + 1;
	return 1;
}

int addNode(int x, int y){
	if(listContains(x, y) == 0){
		struct ListNode* new = (struct ListNode *)malloc(sizeof(struct ListNode));
		new->x = x;
		new->y = y;
		new->next = NULL;
		if(listHead->next == NULL){
			listHead->next = new;
		}else{
			struct ListNode* current = listHead;
			while(current->next != NULL){
				current = current->next;
			}
			current->next = new;
		}
		return 1;
	}
	return 0;
}

int listContains(int x, int y){
	struct ListNode* current = listHead->next;
	while(current != NULL){
		if(current->x == x && current->y == y){
			return 1;
		}
		current = current->next;
	}
	return 0;
}

struct ListNode* removeNode(int x, int y){
	if(listContains(x, y)){
		struct ListNode* current = listHead;
		struct ListNode* temp;
		while(current->next != NULL){
			if(current->next->x == x && current->next->y == y){
				temp = current->next;
				current->next = current->next->next;
				return temp;
			}
			current = current->next;
		}
		return NULL;
	}else{
		return NULL;
	}
}

struct ListNode* getListEnd(){
	struct ListNode* prev = listHead;
	struct ListNode* current = listHead->next;
	while(current != NULL){
		current = current->next;
		prev = prev->next;
	}
	return prev;
}

int getListLength(){
	int i = 0;
	struct ListNode* current = listHead;
	while(current->next != NULL){
		i++;
		current = current->next;
	}
	return i;
}

int freeAll() {
	struct ListNode* current = listHead;
	while (current->next != NULL) {
		struct ListNode* temp = current;
		current = current->next;
		free(temp);
	}
	return 1;
}

struct GameEnt* getFacedEnt(){
	int targetX;
	int targetY;
	if(player.dir == EAST){
		targetX = player.squareX + 1;
		targetY = player.squareY;
	}
	if(player.dir == WEST){
		targetX = player.squareX - 1;
		targetY = player.squareY;
	}
	if(player.dir == NORTH){
		targetX = player.squareX;
		targetY = player.squareY - 1;
	}
	if(player.dir == SOUTH){
		targetX = player.squareX;
		targetY = player.squareY + 1;
	}
	if(listContains(targetX, targetY == 0)){
		return NULL;
	}
	struct GameEnt* target = NULL;
	for(int i = 0; i < ENT_COUNT; i++){
		if(entities[i] != NULL){
			if(entities[i] != &player && entities[i]->squareX == targetX && entities[i]->squareY == targetY){
				target = entities[i];
			}
		}
	}
	return target;
}

moveSprite(int speed, struct GameEnt *ent){
	int LSIS[8];
	int oldX = ent->posX;
	int oldY = ent->posY;
	LSIS[0] = ent->squareX;
	LSIS[1] = (ent->posY + ent->imgHeight - 16)/64;
	LSIS[2] = ent->squareX;
	LSIS[3] = ent->squareY;
	LSIS[4] = (ent->posX + ent->imgWidth)/64;
	LSIS[5] = (ent->posY + ent->imgHeight - 16)/64;
	LSIS[6] = (ent->posX + ent->imgWidth)/64;
	LSIS[7] = ent->squareY;
	switch(ent->dir){
		case WEST: {
		ent->posX = ent->posX - speed;
		ent->squareX = ent->posX/64;
		oldX += 8;
			if(ent->walkseq  > -1 && ent->walkseq < 5){
				ent->image = ent->l1;
			}
			if(ent->walkseq > 4 && ent->walkseq < 10){
				ent->image = ent->l2;
			}
			if(ent->walkseq > 9 && ent->walkseq < 15){
				ent->image = ent->l1;
			}
			if(ent->walkseq > 14 && ent->walkseq < 20){
				ent->image = ent->l3;
			}
			ent->walkseq++;
			if(ent->walkseq > 19){
				ent->walkseq = 0;
			}
		}break;
		
		case EAST: {
		oldX -= 8;
		ent->posX = ent->posX + speed;
		ent->squareX = ent->posX/64;
			if(ent->walkseq  > -1 && ent->walkseq < 5){
				ent->image = ent->r1;
			}
			if(ent->walkseq > 4 && ent->walkseq < 10){
				ent->image = ent->r2;
			}
			if(ent->walkseq > 9 && ent->walkseq < 15){
				ent->image = ent->r1;
			}
			if(ent->walkseq > 14 && ent->walkseq < 20){
				ent->image = ent->r3;
			}
			ent->walkseq++;
			if(ent->walkseq > 19){
				ent->walkseq = 0;
			}
		}break;
		
		case SOUTH: {
		ent->posY = ent->posY + speed;
		ent->squareY = (ent->posY + ent->imgHeight - 8)/64;
		oldY -= 8;
			if(ent->walkseq  > -1 && ent->walkseq < 5){
				ent->image = ent->f1;
			}
			if(ent->walkseq > 4 && ent->walkseq < 10){
				ent->image = ent->f2;
			}
			if(ent->walkseq > 9 && ent->walkseq < 15){
				ent->image = ent->f1;
			}
			if(ent->walkseq > 14 && ent->walkseq < 20){
				ent->image = ent->f3;
			}
			ent->walkseq++;
			if(ent->walkseq > 19){
				ent->walkseq = 0;
			}
		}break;

		case NORTH: {
		oldY +=8;
		ent->posY = ent->posY - speed;
		ent->squareY = (ent->posY + ent->imgHeight - 8)/64;
			if(ent->walkseq  > -1 && ent->walkseq < 5){
				ent->image = ent->b1;
			}
			if(ent->walkseq > 4 && ent->walkseq < 10){
				ent->image = ent->b2;
			}
			if(ent->walkseq > 9 && ent->walkseq < 15){
				ent->image = ent->b1;
			}
			if(ent->walkseq > 14 && ent->walkseq < 20){
				ent->image = ent->b3;
			}
			ent->walkseq++;
			if(ent->walkseq > 19){
				ent->walkseq = 0;
			}
		}break;
	}
	int NSIS[8];
	NSIS[0] = ent->squareX;
	NSIS[1] = (ent->posY + ent->imgHeight - 16)/64;
	NSIS[2] = ent->squareX;
	NSIS[3] = ent->squareY;
	NSIS[4] = (ent->posX + ent->imgWidth)/64;
	NSIS[5] = (ent->posY + ent->imgHeight - 16)/64;
	NSIS[6] = (ent->posX + ent->imgWidth)/64;
	NSIS[7] = ent->squareY;
	for(int i = 0; i < 8; i+=2){
		removeNode(LSIS[i], LSIS[i + 1]);
	}
	int collisionFlag = 0;
	for(int i = 0; i < 8; i+=2){
		if(listContains(NSIS[i], NSIS[i + 1]) == 1)
		{
			collisionFlag = 1;
		}
	}
	int *revised;
	if(collisionFlag == 1){
		revised = LSIS;
		ent->posX = oldX;
		ent->posY = oldY;
		ent->squareX = LSIS[2];
		ent->squareY = LSIS[3];
	}else{
		revised = NSIS;
	}
	for(int i = 0; i < 8; i+=2){
		addNode(*(revised + i), *(revised+i + 1));
	}
	facedEnt = getFacedEnt();
}

LRESULT CALLBACK WindowProcessMessage(HWND, UINT, WPARAM, LPARAM);
#if RAND_MAX == 32767
#define Rand32() ((rand() << 16) + (rand() << 1) + (rand() & 1))
#else
#define Rand32() rand()
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
    static WNDCLASS window_class = { 0 };
    const wchar_t window_class_name[] = L"My Window Class";
    window_class.lpszClassName = window_class_name;
    window_class.lpfnWndProc = WindowProcessMessage;
    window_class.hInstance = hInstance;
	window_class.hCursor = LoadCursor (NULL, IDC_ARROW);
    
    RegisterClass(&window_class);

	frame_bitmap_info.bmiHeader.biSize = sizeof(frame_bitmap_info.bmiHeader);
	frame_bitmap_info.bmiHeader.biPlanes = 1;
	frame_bitmap_info.bmiHeader.biBitCount = 32; //8 bits for red, 8 bits for blue, 8 bits for green, and 8 filler bits
	frame_bitmap_info.bmiHeader.biCompression = BI_RGB;
	frame_device_context = CreateCompatibleDC(0);
    
    HWND window_handle = CreateWindow(window_class_name, L"C Game Tests", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    if(window_handle == NULL) { return -1; }
    
    ShowWindow(window_handle, nCmdShow);
	//update paint
	InvalidateRect(window_handle, NULL, FALSE);
	UpdateWindow(window_handle);
	player.image = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKF1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    while(quit != 1) {
        MSG message;
        while(PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        // Do game stuff here
    }
    
    return 0;
}



LRESULT CALLBACK WindowProcessMessage(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam) {

static PAINTSTRUCT paint;
static HDC device_context;
BITMAP bitmap;
HDC hdcMem;
HGDIOBJ oldBitmap;	   
 switch(message) {
        case WM_QUIT:
        case WM_DESTROY: {
			freeAll();
		DeleteObject(player.image);
            quit = 1;
        } break;

	case WM_CREATE: {
		
		playerCharacter = KANAME;
		OccSq = CreateSolidBrush(RGB(r, g, b));
		player.posX = 64;
		player.posY = 64;
		player.squareX = 1;
		player.squareY = 1;
		entities[ENT_COUNT - 1] = &player;
		listHead = (struct ListNode *)malloc(sizeof(struct ListNode));
		listHead->x = -1;
		listHead->y = -1;
		listHead->next = NULL;
		addNode(player.squareX, player.squareY);
	
		for(int i = 0; i < ENT_COUNT - 2; i++)
		{
			entities[i] = NULL;
		}
		player.imgWidth = 36;
		player.imgHeight = 52;
		player.image = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKF1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKF1 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKF1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKF2 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKF2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKF3 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKF3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKB1 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKB1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKB2 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKB2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKB3 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKB3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKL1 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKL1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKL2 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKL2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKL3 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKL3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKR1 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKR1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKR2 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKR2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKR3 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKR3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SNF1 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Neneko\\SNF1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SNF2 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Neneko\\SNF2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SNF3 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Neneko\\SNF3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SNB1 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Neneko\\SNB1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SNB2 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Neneko\\SNB2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SNB3 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Neneko\\SNB3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SNL1 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Neneko\\SNL1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SNL2 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Neneko\\SNL2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SNL3 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Neneko\\SNL3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SNR1 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Neneko\\SNR1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SNR2 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Neneko\\SNR2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SNR3 = (HBITMAP) LoadImageW(NULL, L"Sprites\\Neneko\\SNR3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKFA = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKFA.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKBA = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKBA.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKLA = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKLA.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		SKRA = (HBITMAP) LoadImageW(NULL, L"Sprites\\Kaname\\SKRA.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			if (player.image == NULL || SKRA == NULL){
				MessageBoxW(window_handle, L"Failed to load image", L"Error", MB_OK);
			}
		
		if(playerCharacter == KANAME){
			player.f1 = SKF1;
			player.f2 = SKF2;
			player.f3 = SKF3;
			player.b1 = SKB1;
			player.b2 = SKB2;
			player.b3 = SKB3;
			player.l1 = SKL1;
			player.l2 = SKL2;
			player.l3 = SKL3;
			player.r1 = SKR1;
			player.r2 = SKR2;
			player.r3 = SKR3;
			player.fAtk = SKFA;
			player.rAtk = SKRA;
			player.bAtk = SKBA;
			player.lAtk = SKLA;
		}else{
			if(playerCharacter == NENEKO){
							player.f1 = SNF1;
			player.f2 = SNF2;
			player.f3 = SNF3;
			player.b1 = SNB1;
			player.b2 = SNB2;
			player.b3 = SNB3;
			player.l1 = SNL1;
			player.l2 = SNL2;
			player.l3 = SNL3;
			player.r1 = SNR1;
			player.r2 = SNR2;
			player.r3 = SNR3;
			}
		}


		rock.posX = 512;
		rock.posY = 512;
		rock.squareX = 8;
		rock.squareY = 8;
		rock.image = (HBITMAP) LoadImageW(NULL, L"Sprites\\rock.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			if(rock.image == NULL){
				MessageBoxW(window_handle, L"Failed to load rock image", L"Error", MB_OK);
			}
		rock.imgHeight = 64;
		const wchar_t* rockmsg = L"It's a rock.";
		rock.desc = rockmsg;
		entities[0] = &rock;
		addNode(rock.squareX, rock.squareY);
		}break;
		

	case WM_KEYDOWN: {
		wchar_t input = (wchar_t)wParam;
		switch(input){
			case VK_LEFT: {
				if(mode == PLAY){
					player.dir = WEST;
					moveSprite(playerSpeed, &player);
				}
				if(mode == SELECT){
					retreat();
				}
			}break;
			case VK_RIGHT: {
				if(mode == PLAY){	
					player.dir = EAST;
					moveSprite(playerSpeed, &player);
				}
				if(mode == SELECT){
					advance();
				}
			}break;
			case VK_UP: {
				if(mode == PLAY){
					player.dir = NORTH;
					moveSprite(playerSpeed, &player);
				}
				if(mode == SELECT){
					retreat();
				}
			}break;
			case VK_DOWN: {
				if(mode == PLAY){
					player.dir = SOUTH;
					moveSprite(playerSpeed, &player);
				}
				if(mode==SELECT){
					advance();
				}
			}break;
			case VK_SPACE: {
				if(mode == PLAY){
					attack();
					
					int temp = r;
					r = g;
					g = b;
					b = temp;
					HBRUSH oldBrush = OccSq;
					OccSq = CreateSolidBrush(RGB(r, g, b));
					DeleteObject(oldBrush);
				}
			}break;
			case VK_ESCAPE: {
				if(mode == PLAY){
					buildCharacterSelect();
					mode = SELECT;
				}else{
					mode = PLAY;
				}
			}break;
			case VK_RETURN: {
				if(mode == SELECT){
					cs->assocAct();
					mode = PLAY;
					moveSprite(0, &player);
				}
			}break;
			
			case 0x45: {
				if(mode == PLAY && facedEnt != NULL){
					MessageBox(window_handle, facedEnt->desc , L"Test", MB_OK);	
				}

			}break;
				
		}
		InvalidateRect(window_handle, NULL, FALSE);
		UpdateWindow(window_handle);
	} break;

	case WM_KEYUP: {
		wchar_t input = (wchar_t)wParam;
		switch(input){
			case VK_SPACE: {
				switch(player.dir){
					case WEST: {	
						player.image = player.l1;
					}break;
					case EAST: {
						player.image = player.r1;
					}break;
					case NORTH: {
						player.image = player.b1;
					}break;
					case SOUTH: {
						player.image = player.f1;
					}break;
				}
			}break;
		}
		InvalidateRect(window_handle, NULL, FALSE);
		UpdateWindow(window_handle);
	} break;

	case WM_PAINT: {
		device_context = BeginPaint(window_handle, &paint);
		if(mode == PLAY){
			FillRect(device_context, &paint.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
			while(player.posX < 0){
				player.posX++;
			}
			while(player.posY < 0){
				player.posY++;
			}
			while(player.posY > 640){
				player.posY--;
			}
			while(player.posX > 1216){
				player.posX--;
			}
			TextOutA(device_context, 10, 10, "Use the arrow keys to move and e to interact with objects", 60);
		
			char buffer[50];
			if(listContains(5, 5)){
				TextOutA(device_context, 952, 640, "You found the hidden debugging position!", 40);
			}
		
			struct ListNode* occupied = listHead;
			while(occupied != NULL){
				sillySquare.left = occupied->x*64;
				sillySquare.top = occupied->y*64;
				sillySquare.right = sillySquare.left + 64;
				sillySquare.bottom = sillySquare.top + 64;
				FillRect(device_context, &sillySquare, OccSq);
				char sqrString[20];
				sprintf(sqrString, "Occ: %d, %d\t", occupied->x, occupied->y);
				TextOutA(device_context, occupied->x*64, occupied->y*64, sqrString, 20);
				occupied = occupied->next;
			}

			for(int i = 0; i < ENT_COUNT; i++){
				if(entities[i] != NULL){
					hdcMem = CreateCompatibleDC(device_context);
					oldBitmap = SelectObject(hdcMem, entities[i]->image);
					GetObject(entities[i]->image, sizeof(bitmap), &bitmap);
					if(!BitBlt(device_context, entities[i]->posX, entities[i]->posY + bitmap.bmHeight/2, bitmap.bmWidth, bitmap.bmHeight/2, hdcMem, 0, bitmap.bmHeight/2, SRCCOPY)){
						MessageBox(window_handle, L"BitBlt has failed", L"Failed", MB_OK);
					};
					SelectObject(hdcMem, oldBitmap);
					DeleteDC(hdcMem);
				}
			}
			for(int i = 0; i < ENT_COUNT; i++){
				if(entities[i] != NULL){
					hdcMem = CreateCompatibleDC(device_context);
					oldBitmap = SelectObject(hdcMem, entities[i]->image);
					GetObject(entities[i]->image, sizeof(bitmap), &bitmap);
					if(!BitBlt(device_context, entities[i]->posX, entities[i]->posY, bitmap.bmWidth, bitmap.bmHeight/2, hdcMem, 0, 0, SRCCOPY)){
						MessageBox(window_handle, L"BitBlt has failed", L"Failed", MB_OK);
					};
					SelectObject(hdcMem, oldBitmap);
					DeleteDC(hdcMem);
				}
			}
			EndPaint(window_handle, &paint);
		}else{
			if(mode == SELECT){
				int columnNo = cs->optCount/cs->columnSize;
				if(cs->optCount%cs->columnSize != 0){
					columnNo++;
				}
				int dispX = cs->optBoxWidth + cs->margin;
				int dispY = cs->optBoxHeight + cs->margin;
				RECT menuRect;
					menuRect.left = cs->originX;
					menuRect.top = cs->originY;
					menuRect.right = menuRect.left + dispX *columnNo  + cs->margin;
					menuRect.bottom = menuRect.top + dispY * cs->columnSize + cs->margin;
				FillRect(device_context, &menuRect, (HBRUSH)(COLOR_WINDOW + 1));
				RECT selectRect;
					selectRect.left = cs->originX + cs->selectorX * dispX;
					selectRect.top = cs->originY + cs->selectorY * dispY;
					selectRect.right = selectRect.left + dispX + cs->margin;
					selectRect.bottom = selectRect.top + dispY + cs->margin;
				FillRect(device_context, &selectRect, cs->selectorColor);
				for(int a = 0; a < columnNo; a++){
					for(int i = 0; i < cs->columnSize; i++){
						RECT tempRect;
						tempRect.left = cs->originX + cs->margin * (a + 1) + a * cs->optBoxWidth;
						tempRect.top = cs->originY + cs->margin * (i + 1) + i * cs->optBoxHeight;
						tempRect.right = tempRect.left + cs->optBoxWidth;
						tempRect.bottom = tempRect.top + cs->optBoxHeight;
						FillRect(device_context, &tempRect, cs->optColor);
					}
				}
				for(int i = 0; i < cs->optCount; i++){
					TextOutW(device_context, cs->originX + cs->margin * 2 + dispX * i/cs->columnSize, cs->originY + cs->margin * 2 + dispY * i%cs->columnSize, cs->opts[i], wcslen(cs->opts[i]));
				}
				TextOutW(device_context, cs->originX, cs->originY - 10, cs->choiceName, wcslen(cs->choiceName));
			}
		}
		EndPaint(window_handle, &paint);
	} break;
	
	//resize bitmap to match window
	case WM_SIZE: {
		frame_bitmap_info.bmiHeader.biWidth = LOWORD(lParam);
		frame_bitmap_info.bmiHeader.biHeight = HIWORD(lParam);

		if(frame_bitmap){ DeleteObject(frame_bitmap); }
		frame_bitmap = CreateDIBSection(NULL, &frame_bitmap_info, DIB_RGB_COLORS, (void**)&frame.pixels, 0, 0);
		SelectObject(frame_device_context, frame_bitmap);

		frame.width = LOWORD(lParam);
		frame.height = HIWORD(lParam);
	}break;
        
        default: { // Message not handled; pass on to default message handling function
            return DefWindowProc(window_handle, message, wParam, lParam);
        } break;
    }
    return 0;
}


























/*SECTION I: PLAY MODE*/

int attack(){
	switch(player.dir){
		case WEST: {	
			player.image = player.lAtk;
		}break;
		case EAST: {
			player.image = player.rAtk;
		}break;
		case NORTH: {
			player.image = player.bAtk;
		}break;
		case SOUTH: {
			player.image = player.fAtk;
		}break;
	}
	return 1;
}







/*SECTION II: SELECTION MODE*/

int switchCharacters(){
	if(cs->selectorY == 0){
			player.f1 = SKF1;
			player.f2 = SKF2;
			player.f3 = SKF3;
			player.b1 = SKB1;
			player.b2 = SKB2;
			player.b3 = SKB3;
			player.l1 = SKL1;
			player.l2 = SKL2;
			player.l3 = SKL3;
			player.r1 = SKR1;
			player.r2 = SKR2;
			player.r3 = SKR3;
	}else{
			player.f1 = SNF1;
			player.f2 = SNF2;
			player.f3 = SNF3;
			player.b1 = SNB1;
			player.b2 = SNB2;
			player.b3 = SNB3;
			player.l1 = SNL1;
			player.l2 = SNL2;
			player.l3 = SNL3;
			player.r1 = SNR1;
			player.r2 = SNR2;
			player.r3 = SNR3;
	}
	return 1;
}

int buildCharacterSelect(){
	if(cs != NULL){
		DeleteObject(cs->optColor);
		DeleteObject(cs->selectorColor);
		free(cs);
	}
	cs = (struct Selection*)malloc(sizeof(struct Selection));
	cs->choiceName = chSl;
	cs->originX = player.posX;
	cs->originY = player.posY;
	cs->optCount = 2;
	cs->optBoxWidth = 128;
	cs->optBoxHeight = 64;
	cs->margin = 16;
	cs->optColor = CreateSolidBrush(RGB(172, 172, 172));
	cs->selectorColor = CreateSolidBrush(RGB(96, 96, 96));
	cs->selectorX = 0;
	cs->selectorY = 0;
	cs->columnSize = 2;
	const wchar_t* knm = L"1: Kaname";
	const wchar_t* nnk = L"2: Neneko";
	cs->opts[0] = knm;
	cs->opts[1] = nnk;
	cs->assocAct = &switchCharacters;
	return 1;
}


int advance(){
	cs->selectorY++;
	if(cs->selectorY >= cs->columnSize){
		cs->selectorY = 0;
		if(cs->selectorX < cs->optCount/cs->columnSize - 1){
		cs->selectorX++;
		}
	}
	return 1;
}

int retreat(){
	int maxX = cs->optCount/cs->columnSize - 1;
	int maxY = cs->optCount%cs->columnSize;
	if(--cs->selectorY < 0){
		if(cs->selectorX <= 0){
			cs->selectorX = maxX;
			cs->selectorY = maxY;
		}else{
			cs->selectorX--;
			cs->selectorY = cs->columnSize - 1;
		}
	}
	return 1;
}
