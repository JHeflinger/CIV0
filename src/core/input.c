#include "input.h"
#include "raylib.h"

char g_WSQueue[2];
char g_ADQueue[2];
char g_IKQueue[2];
char g_JLQueue[2];

void UpdateSmartInput() {
	if (IsKeyDown(KEY_W)) {
		if (g_WSQueue[0] != 'w' && g_WSQueue[1] != 'w') {	
			g_WSQueue[1] = g_WSQueue[0];
			g_WSQueue[0] = 'w';
		}
	} else {
		if (g_WSQueue[0] == 'w') {
			g_WSQueue[0] = g_WSQueue[1];
			g_WSQueue[1] = '\0';
		} else if (g_WSQueue[1] == 'w') {
			g_WSQueue[1] = '\0';
		}
	}
	if (IsKeyDown(KEY_S)) {
		if (g_WSQueue[0] != 's' && g_WSQueue[1] != 's') {	
			g_WSQueue[1] = g_WSQueue[0];
			g_WSQueue[0] = 's';
		}
	} else {
		if (g_WSQueue[0] == 's') {
			g_WSQueue[0] = g_WSQueue[1];
			g_WSQueue[1] = '\0';
		} else if (g_WSQueue[1] == 's') {
			g_WSQueue[1] = '\0';
		}
	}
	if (IsKeyDown(KEY_A)) {
		if (g_ADQueue[0] != 'a' && g_ADQueue[1] != 'a') {	
			g_ADQueue[1] = g_ADQueue[0];
			g_ADQueue[0] = 'a';
		}
	} else {
		if (g_ADQueue[0] == 'a') {
			g_ADQueue[0] = g_ADQueue[1];
			g_ADQueue[1] = '\0';
		} else if (g_ADQueue[1] == 'a') {
			g_ADQueue[1] = '\0';
		}
	}
	if (IsKeyDown(KEY_D)) { 
		if (g_ADQueue[0] != 'd' && g_ADQueue[1] != 'd') {	
			g_ADQueue[1] = g_ADQueue[0];
			g_ADQueue[0] = 'd';
		}
	} else {
		if (g_ADQueue[0] == 'd') {
			g_ADQueue[0] = g_ADQueue[1];
			g_ADQueue[1] = '\0';
		} else if (g_ADQueue[1] == 'd') {
			g_ADQueue[1] = '\0';
		}
	}
	if (IsKeyDown(KEY_I)) {
		if (g_IKQueue[0] != 'i' && g_IKQueue[1] != 'i') {	
			g_IKQueue[1] = g_IKQueue[0];
			g_IKQueue[0] = 'i';
		}
	} else {
		if (g_IKQueue[0] == 'i') {
			g_IKQueue[0] = g_IKQueue[1];
			g_IKQueue[1] = '\0';
		} else if (g_IKQueue[1] == 'i') {
			g_IKQueue[1] = '\0';
		}
	}
	if (IsKeyDown(KEY_K)) {
		if (g_IKQueue[0] != 'k' && g_IKQueue[1] != 'k') {	
			g_IKQueue[1] = g_IKQueue[0];
			g_IKQueue[0] = 'k';
		}
	} else {
		if (g_IKQueue[0] == 'k') {
			g_IKQueue[0] = g_IKQueue[1];
			g_IKQueue[1] = '\0';
		} else if (g_IKQueue[1] == 'k') {
			g_IKQueue[1] = '\0';
		}
	}
	if (IsKeyDown(KEY_J)) {
		if (g_JLQueue[0] != 'j' && g_JLQueue[1] != 'j') {	
			g_JLQueue[1] = g_JLQueue[0];
			g_JLQueue[0] = 'j';
		}
	} else {
		if (g_JLQueue[0] == 'j') {
			g_JLQueue[0] = g_JLQueue[1];
			g_JLQueue[1] = '\0';
		} else if (g_JLQueue[1] == 'j') {
			g_JLQueue[1] = '\0';
		}
	}
	if (IsKeyDown(KEY_L)) { 
		if (g_JLQueue[0] != 'l' && g_JLQueue[1] != 'l') {	
			g_JLQueue[1] = g_JLQueue[0];
			g_JLQueue[0] = 'l';
		}
	} else {
		if (g_JLQueue[0] == 'l') {
			g_JLQueue[0] = g_JLQueue[1];
			g_JLQueue[1] = '\0';
		} else if (g_JLQueue[1] == 'l') {
			g_JLQueue[1] = '\0';
		}
	}
}

char PeekWS() {
	if (g_WSQueue[0] != 'w' && g_WSQueue[0] != 's') return '\0';
	return g_WSQueue[0];
}

char PeekAD() {
	if (g_ADQueue[0] != 'a' && g_ADQueue[0] != 'd') return '\0';
	return g_ADQueue[0];
}

char PeekIK() {
	if (g_IKQueue[0] != 'i' && g_IKQueue[0] != 'k') return '\0';
	return g_IKQueue[0];
}

char PeekJL() {
	if (g_JLQueue[0] != 'j' && g_JLQueue[0] != 'l') return '\0';
	return g_JLQueue[0];
}
