#include <windows.h>
#include <ctime>
#include <thread>
//#include <Windows.h>
// demo for my 2D-motor
// ESC = clear and draw
int shootWS;     // ID of the object shot by W and S
int shootAD;     // ID of the object shot by A and D
int players[1][5];  // co-op, [id, button1, btn2, btn3, btn4]
int objects[32]; // make lists to allow up to 32 objects in the world
int xp[32];      // x-position
int yp[32];      // y-position
int xlp[32];     // length of x
int ylp[32];     // length of y
int colors[32][4]; // R, G, B, A - A for alpha, not yet implemented, so just RGB for now, with pixSpace.
int pixSpace[32][2]; // space between each pixel, giving a transparent look, x and y
int taken = 0;    // amount of busy objects
int hitboxes[32][4]; // positions to collide [x, x2, y, y2]
int partsID[32][4]; // [originItemID, part1, part2, part3, part4]
int doors[5];      // doors
int gameObjs[12]; // objects part of the clientside game
int rooms;
bool open[5] = { true,false,false,false,false};
bool listens[32];// for more automatic listeners, telling whether added or not
bool fill[32];  // not used at the moment
std::thread thr[12]; // easy list with threads to run things async
HWND win = GetConsoleWindow();
HDC hdc = GetDC(win); // Get more access to the CMD window

int addObject(int t = 0, int x = 0, int y = 0, int xlen = 1, int ylen = 1, int R = 0, int G = 0, int B = 0, int A = 0, bool drawOth = false, int ps1 = 0, int ps2 = 0) { // function to add an object with properties
	if (taken < 32) {
		objects[taken] = t;
		xp[taken] = x;
		yp[taken] = y;
		xlp[taken] = xlen;
		ylp[taken] = ylen;
		colors[taken][0] = R;
		colors[taken][1] = G;
		colors[taken][2] = B;
		colors[taken][3] = A;
		hitboxes[taken][0] = x;
		hitboxes[taken][1] = x+xlen;
		hitboxes[taken][2] = y;
		hitboxes[taken][3] = y+ylen;
		if (ps1 != 0)pixSpace[taken][0] = ps1;
		if (ps2 != 0)pixSpace[taken][1] = ps2;
		if (drawOth)fill[taken] = true;
		taken++;
		return taken - 1; // returns the id, which can later be used when more automated
	}
	return -1;          // all objects taken
}
int* collision(int obj,bool any=false) { // give a dynamically-long list of other objects that obj is within
	int x = xp[obj];
	int y = yp[obj];
	int colls[32]; int a = 0;
	for (int i = 0; i < 32; i++) {
		if (x > hitboxes[i][0] - xlp[obj]/2 && x < hitboxes[i][1] + 3 && y > hitboxes[i][2] - ylp[obj]/2 && y < hitboxes[i][3] + ylp[obj]/2) {
			colls[a]=i; a++;
			if(any)break;
		}
	}
	return colls;
}
void moveObjL(int obj, int x, int y) { // move an object far, just replace it all (of the object)
	for (int i = 0; i < xlp[obj]; i++) { // loop the x-size
		for (int i2 = 0; i2 < ylp[obj]; i2++) { // loop the y-size for every x
			SetPixelV(hdc, xp[obj]+i, yp[obj] + i2, RGB(0, 0, 0)); // didn't make it edit screen cache (ASM) instead of SetPixelV() yet :/
		}
	}
	xp[obj] = x; // set position to new
	yp[obj] = y;
	hitboxes[obj][0] = x; // update hitboxes
	hitboxes[obj][1] = x + xlp[obj];
	hitboxes[obj][2] = y;
	hitboxes[obj][3] = y + ylp[obj];
}
void moveObject(int obj, int x, int y) { // function call to move an object with a smoother render
	int diff = x - xp[obj];              // difference between new and old position on the x-axis
	int diff2 = y - yp[obj];             // difference between new and old position on the y-axis
	for (int i = 0; i < xlp[obj]; i+= (pixSpace[obj][0] != 0 ? pixSpace[obj][0] : 1)) { // loop the x-length to print new pixels
		SetPixelV(hdc, x +  i, y + (diff2 > 0 ? ylp[obj]-1 : diff2+1), RGB(colors[obj][0], colors[obj][1], colors[obj][2])); // print new pixels
		//if(diff2>0)SetPixel(hdc, xp[obj] + i, yp[obj], RGB(0, 0, 0));
		for (int i2 = 0; i2 < ylp[obj]; i2+= (pixSpace[obj][1] != 0 ? pixSpace[obj][1] : 1)) { // loop the y-length to print new pixels for that x-row
			SetPixelV(hdc, x + (diff > 0 ? xlp[obj]-1 : diff+1), y + i2, RGB(colors[obj][0], colors[obj][1], colors[obj][2])); //print new pixels
			SetPixelV(hdc, xp[obj] + (diff > 0 ? 0 : xlp[obj]), yp[obj] + i2, RGB(0, 0, 0)); // print over old pixels
			//if(diff>0)SetPixel(hdc, xp[obj], yp[obj] + i2, RGB(0, 0, 0));
		}
		SetPixelV(hdc, xp[obj] + i, yp[obj] + (diff2 > 0 ? 0 : ylp[obj]), RGB(0, 0, 0)); // position + part of length and other position = black 
	}
	if(diff<0||diff2<0)SetPixelV(hdc,xp[obj]+xlp[obj],yp[obj]+ylp[obj],RGB(0,0,0));
	xp[obj] = x; // set position to new
	yp[obj] = y;
	hitboxes[obj][0] = x; // update hitboxes
	hitboxes[obj][1] = x + xlp[obj];
	hitboxes[obj][2] = y;
	hitboxes[obj][3] = y + ylp[obj];
}
void editObj(int obj, int xlen, int ylen, int R, int G, int B){ // easily change information about an object
	xlp[obj] = xlen;
	ylp[obj] = ylen;
	colors[obj][0] = R;
	colors[obj][1] = G;
	colors[obj][2] = B;
}
void update() { // re-render everything
	for (int i = 0; i < 32; i++) {
		if(xp[i]+xlp[i]>0)if(yp[i]+ylp[i]>0)
		switch (objects[i]) {
		case 0: SetPixelV(hdc, xp[i], yp[i], RGB(colors[i][0], colors[i][1], colors[i][2])); break;
		case 2: // if type is a line
			for (int x = 0; x < xlp[i]; x++) {
				SetPixelV(hdc, xp[i] + x, yp[i], RGB(colors[i][0], colors[i][1], colors[i][2]));
			}
			break;
		case 3: // if type is a rectangle
			for (int x = 0; x < xlp[i]; x++) {     // loop x-length
				for (int y = 0; y < ylp[i]; y++) { // loop y-length per every x
					SetPixelV(hdc, xp[i] + x, yp[i] + y, RGB(colors[i][0], colors[i][1], colors[i][2]));
				}
			}
			break;
		}
	}
}

void npcM(int ide=1) { // move NPC
	int g = 1;
	int i = 1;
	while (i>0&&listens[gameObjs[ide]]) {
		i += g;
		if (i > 98)g = -1; if (i < 3)g = 1;
		thr[4] = std::thread(moveObject,gameObjs[ide], xp[gameObjs[ide]], yp[gameObjs[ide]] + g); // didn't work properly, when multiple things (like player) are moving the render messes up
		thr[4].join();
		Sleep(10);
		//t2.join();
	}
}

int* genRoom() {
	srand(time(NULL));
	int r = rand() % 300 + 20;
	for (int i = 0; i < 12; i++) {
		if (!gameObjs[i]) { // if object has not been implemented
			switch (i) {
			case 0:
				gameObjs[i] = addObject(3, 200, 200, 200, 20, 200, 30, 45);
				break;
			case 1:
				if (rand() % 9 == 1) {
					gameObjs[i] = addObject(3, 250, 350, 20, 20, 180, 50, 55);
					listens[gameObjs[i]] = true;
					thr[3] = std::thread(npcM,1);
				}
				break;
			case 2:
				gameObjs[i] = addObject(3, 240, 180, 20, 10, 150, 180, 15);
				break;
			case 3:
				if (rand() % 9 == 1) {
					gameObjs[i] = addObject(3, rand()%250+100, rand()%400+100, 20, 20, 180, 50, 55);
					listens[gameObjs[i]] = true;
					thr[5] = std::thread(npcM, 3);
				}
				break;
			}
		}else{
			switch (i) {
			case 1:
				if (rand() % 5 == 2) {
					//if(thr[3].joinable())thr[3].detach();
					listens[gameObjs[i]] = false;
					if (thr[3].joinable())thr[3].join();
				}
				else if(!listens[gameObjs[i]]){
					listens[gameObjs[i]] = true;
					moveObjL(gameObjs[i], 250, 350);
					thr[3] = std::thread(npcM, 1);
				}
				break;
			case 3:
				if (rand() % 5 == 2) {
					listens[gameObjs[i]] = false;
					if (thr[5].joinable())thr[5].join();
				}
				else if (!listens[gameObjs[i]]) {
					listens[gameObjs[i]] = true;
					thr[5] = std::thread(npcM, 3);
				}
				  break;
			case 2:
				int xy[2] = { rand() % 800 + 10, rand() % 300 + 80 };

				moveObjL(gameObjs[2], xy[0],xy[1]);
				break;
			}
		}
	}
	moveObjL(doors[0], rand() % 20+1, rand() % 300 + 100);
	moveObjL(doors[1], 1050, r);
	int res[2] = { 1050, r };
	return res;
}
void agen(){
	//for (int i = 0; i < 9; i++) {
		moveObject(2, xp[2] + 1, yp[2] + 1);
	//}
	agen();
}
void endAsyncs() {
	for (int i = 0; i < 6; i++)thr[i].join();
}
void launch(int pl,int id,int t, int x,int y) {
	switch (t) {
	case 0: {
		xp[id] = xp[pl] + x; yp[id] = yp[pl] + y - 25; for (int i = 1; i < yp[pl]; i++)moveObject(id, xp[pl] + x, yp[pl] + y - 25 - i);
		break; }
	case 1: {
		xp[id] = x - 25; yp[id] = y + 5; for (int i = 1; i < xp[pl]; i++)moveObject(id, xp[pl] + x - 25 - i, yp[pl] + y + 5);
		break; }
	case 2: {
		xp[id] = xp[pl] + x + 25; yp[id] = yp[pl] + y + 50; for (int i = 1; i < 550 - yp[pl]; i++) {
			int* ids = collision(id);
			for (int d = 0; d < sizeof(ids); d++) {
				for (int it = 0; it < sizeof(gameObjs); it++) {
					if (gameObjs[it] == ids[d]) {
						partsID[it][0] = gameObjs[it];
						int gg = xp[id] - xp[gameObjs[it]];
						if(gg>1)
						partsID[it][1] = addObject(3, xp[gameObjs[it]], yp[gameObjs[it]], gg, ylp[gameObjs[it]], colors[gameObjs[it]][0], colors[gameObjs[it]][1], colors[gameObjs[it]][2]);
						//hitboxes[gameObjs[it]][0] = xp[gameObjs[it]] + xlp[gameObjs[it]] - xp[id];
						xlp[gameObjs[it]] = xlp[gameObjs[it]] - gg - xlp[id];
						xp[gameObjs[it]] = xp[id] + xlp[id];
						hitboxes[gameObjs[it]][0] = xp[gameObjs[it]];
					}
				}
			}
			moveObject(id, xp[pl] + x + 25, yp[pl] + y + 50 + i);
		}
		break; }
	case 3: {
		xp[id] = xp[pl] + 50; yp[id] = yp[pl] + y; for (int i = 1; i < 1200 - xp[pl]; i++) {
			moveObject(id, xp[pl] + x + 50 + i, yp[pl] + y); }
		break; }
	}
}
int main(bool y = false)
{
	//std::cout << addObject(3, 0, 0, 1280, 20, 25, 25, 25) << "\n";
	addObject(3, 0, 0, 1280, 10, 255, 255, 255);
	addObject(3, 0, 525, 1280, 20, 250, 25, 255);
	players[0][0] = addObject(3, 200, 200, 20, 20, 100, 95, 45); // set player 1 to the newly created object
	players[0][1] = 'W';
	players[0][2] = 'A';
	players[0][3] = 'S';
	players[0][4] = 'D';
	doors[0] = addObject(3, 10, 100, 10, 50, 100, 200, 50);
	doors[1] = addObject(3, 1050, 200, 10, 50, 100, 200, 50);
	shootWS = addObject(3, 0, 0, 3, 15, 200, 25, 25);
	shootAD = addObject(3, 0, 0, 15, 3, 200, 25, 25);
	update();
	if(!y)thr[0] = std::thread(main, true);
	if(y)while (y) { // keep listening for key input
		int x = 0;
		int y = 0;
		bool change = false; // shall we update the npc pos or not
		bool shoot = false; // Is the player trying to shoot?
		if (GetAsyncKeyState(VK_ESCAPE) & 0x01) {
			system("cls");
			update();
		}
		if (GetAsyncKeyState(VK_SPACE) & 0x01) {
			shoot = true;
		}
		else { shoot = false; }
		if (GetAsyncKeyState(VK_CONTROL) & 0x01) { // if key CTRL is pressed
			moveObjL(2, 50, 50); xp[2] = 50; yp[2] = 50;
		}
		if(GetAsyncKeyState(players[0][4]) & 0x8000){ // is held down?
			if (xp[2] < 1050) { x++; change = true; }
			if (shoot) { launch(players[0][0], shootAD, 3, x, y); }//xp[shootAD] = xp[2]+50; yp[shootAD] = yp[2]+y; for (int i = 1; i < 1200-xp[2]; i++)moveObject(shootAD, xp[2]+x+50 + i, yp[2]+y); }
		}
		if (GetAsyncKeyState(players[0][2]) & 0x8000) {
			if (xp[2] > 10) { x--; change = true; }
			if (shoot) { launch(players[0][0], shootAD, 1, x, y); }//xp[shootAD] = x-25; yp[shootAD] = y+5; for (int i = 1; i < xp[2]; i++)moveObject(shootAD, xp[2]+x-25-i, yp[2]+y+5); }
		}
		if (GetAsyncKeyState(players[0][1]) & 0x8000) {
			if (yp[2] > 12) { y--; change = true; }
			if (shoot) { launch(players[0][0],shootWS,0,x,y); }//xp[shootWS] = xp[2]+x; yp[shootWS] = yp[2]+y-25; for (int i = 1; i < yp[2]; i++)moveObject(shootWS, xp[2]+x, yp[2]+y -25- i); }
		}
		if (GetAsyncKeyState(players[0][3]) & 0x8000) {
			if (yp[2] < 500) { y++; change = true; }
			if (shoot) { launch(players[0][0], shootWS, 2, x, y); }//xp[shootWS] = xp[2]+x+25; yp[shootWS] = yp[2]+y+50; for (int i = 1; i < 550-yp[2]; i++)moveObject(shootWS, xp[2]+x+25, yp[2]+y+50 + i); }
		}
		for (int i = 3; i < 32; i++) {
			if (xp[2]+x > hitboxes[i][0] - 21 && xp[2]+x < hitboxes[i][1] +1 && yp[2]+y > hitboxes[i][2] - 21 && yp[2]+y < hitboxes[i][3] + 3) { // check if player position is within another objects hitbox position
				if (i == gameObjs[2]) {
					open[0]=true;
					moveObjL(gameObjs[2], -20, -20);
				}
				for (int d = 0; d < 5; d++) {
					if (i == doors[d] && open[d]) {
						open[d] = false;
						int* re = genRoom();
						moveObjL(2, re[0]-50, re[1]+y);
						update();
						break;
					}
				}
				y = 0;
				x = 0;
				/*
				if (yp[2]+y > hitboxes[i][2]-25 && yp[2]+y < hitboxes[i][2]) {
					y--;
				}
				else if (yp[2]+y < hitboxes[i][3]) {
					y++;
				}
				if (hitboxes[i][0] - (xp[2]+x) < hitboxes[i][1] - (xp[2]+x)) { x--; }
				else { x++; }*/
				break;
			}
		}
		if (change && (y != 0 || x != 0)) { moveObject(players[0][0], xp[2] + x, yp[2] + y); } // any changes? change!
	}
}