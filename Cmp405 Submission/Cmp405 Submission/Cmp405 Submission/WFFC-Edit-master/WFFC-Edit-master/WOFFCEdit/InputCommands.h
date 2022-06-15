#pragma once

struct InputCommands
{
	bool forward;
	bool back;
	bool up;
	bool down;
	bool rotRight;
	bool rotLeft;
	bool rotUp;
	bool rotDown;

	int mouse_X;
	int mouse_Y;

	bool clicked;
	bool dblclicked;
	bool mouseup;


	bool I; //forwardX
	bool K;	//backX
	bool J;//forwardy
	bool L; // backy
	bool O; //down
	bool U; // up


	bool scaleup;
	bool scaledown;

	bool cam1;
	bool cam2;
	bool cam3;

	bool wireframe;
};
