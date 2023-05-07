#include <Windows.h>
#include <cstdio>

HHOOK KeyHook;
HHOOK MouseHook;

LRESULT CALLBACK KeyProc(int nCode, WPARAM wParam, LPARAM lParam) {
	auto* p = reinterpret_cast<PKBDLLHOOKSTRUCT>(lParam);

	// If key is being pressed
	if (wParam == WM_KEYDOWN) {
		switch (p->vkCode) {
			// Invisible keys
		case VK_CAPITAL:	printf("<CAPLOCK>");	break;
		case VK_LSHIFT:		printf("<LSHIFT>");	break;
		case VK_RSHIFT:		printf("<RSHIFT>");	break;
		case VK_LCONTROL:	printf("<LCTRL>");	break;
		case VK_RCONTROL:	printf("<RCTRL>");	break;
		case VK_DELETE:		printf("<DEL>");		break;
		case VK_BACK:		printf("<BK>");		break;

		case VK_LEFT:		printf("<LEFT>");		break;
		case VK_RIGHT:		printf("<RIGHT>");	break;
		case VK_UP:			printf("<UP>");		break;
		case VK_DOWN:		printf("<DOWN>");		break;

		case VK_RETURN:		printf("\n");			break;
			// Visible keys
		default:
			printf("%c", char(p->vkCode));
		}
	}

	return CallNextHookEx(KeyHook, nCode, wParam, lParam);
}

bool CaptureMouse = false;
long prevX, prevY;

void DrawLine(long x, long y) {
	auto* hwnd = GetDesktopWindow();
	auto* hdc = GetWindowDC(hwnd);
	MoveToEx(hdc, prevX, prevY, nullptr);
	auto* pen = CreatePen(PS_SOLID, 5, RGB(0, 200, 255));
	SelectObject(hdc, pen);
	LineTo(hdc, x, y);
	ReleaseDC(hwnd, hdc);
	prevX = x, prevY = y;
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	auto* p = reinterpret_cast<PMSLLHOOKSTRUCT>(lParam);
	auto x = p->pt.x;
	auto y = p->pt.y;

	switch (wParam) {
	case WM_LBUTTONDOWN:
		CaptureMouse = true;
		prevX = x;
		prevY = y;
		break;
	case WM_LBUTTONUP:
		CaptureMouse = false;
		break;
	case WM_MOUSEMOVE:
		if (CaptureMouse) {
			printf("(x, y) == %lu, %lu\n", x, y);
			DrawLine(x, y);
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
	KeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyProc, 0, 0);
	if (!KeyHook) {
		printf("Error creating keyboard hook: 0x%x", GetLastError());
		return 1;
	}

	MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, 0, 0);
	if (!MouseHook) {
		printf("Error creating mouse hook: 0x%x", GetLastError());
		return 1;
	}

	printf("Hooks are set...\n");
	
	MSG msg;
	while (!GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(KeyHook);
	UnhookWindowsHookEx(MouseHook);
	return 0;
}

