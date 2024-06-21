#include <stdio.h>

#include "raylib.h"

typedef enum EditorMode {
  NORMAL,
  INSERT,
} EditorMode;

void SetTextBuffer(char *BUFF, const int BUFFW, const int BUFFH, const char *text, bool wrapText) {
  int x = 0, y = 0;
  for (int i = 0; text[i] != '\0' && i < 1024; i++) {
    if (y >= BUFFH) break;

    if (text[i] == '\n') {
      y++; x = 0;
      continue;
    }

    if (x < BUFFW)
      BUFF[y*(BUFFW+1)+x] = text[i];

    x++;

    if (wrapText && x >= BUFFW) {
      y++; x = 0;
    }
  }
}

int main(void) {

  SetTraceLogLevel(LOG_ERROR);
  InitWindow(1, 1, ".");

  const int FONTSIZE    = 32;
  const int FONTSPACING = 2;

  // Assumes monospace font
  Font editorFont = LoadFontEx("src/resources/JetBrainsMonoNerdFontMono-Regular.ttf", FONTSIZE, NULL, 0);

  Vector2 FONTCHARSIZE  = MeasureTextEx(editorFont, "0", (float)FONTSIZE, (float)FONTSPACING);
  float   FONTCHARW     = FONTCHARSIZE.x;
  float   FONTCHARH     = FONTCHARSIZE.y;

  const int BUFFW = 32;
  const int BUFFH = 16;

  const int SCREENPADDING = 10;

  int SW = (int)(BUFFW*(FONTCHARW+FONTSPACING))+2*SCREENPADDING;
  int SH = (int)((BUFFH+1)*(FONTCHARH))+2*SCREENPADDING;

  SetWindowSize(SW, SH);
  SetWindowTitle("Editor65");
  SetWindowPosition((int)GetWindowPosition().x-SW/2, (int)GetWindowPosition().y);

  EditorMode editorMode = NORMAL;
  
  int cursorX = 0;
  int cursorY = 0;

  //const char msg[256] = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHI\nJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmn\nopqrstuvwxyz{|}~¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓ\nÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷\nøùúûüýþÿ";
  
  // Init text buffer
  char BUFF[(BUFFW+1)*BUFFH+1];
  for (int i = 0; i < BUFFH; i++) {
    for (int j = 0; j < BUFFW; j++) {
      BUFF[i*(BUFFW+1)+j] = '0';
    }
    BUFF[i*(BUFFW+1)+BUFFW] = '\n';
  }
  BUFF[(BUFFW+1)*BUFFH] = '\0';
  //printf(BUFF);
  
  //SetTextBuffer(BUFF, BUFFW, BUFFH, msg, true);

  SetTextLineSpacing((int)FONTCHARH);

  SetTargetFPS(60);

  SetExitKey(KEY_NULL);

  bool shouldClose = false;
  while (!WindowShouldClose() && !shouldClose) {
    switch (editorMode) {
      case NORMAL:
        {
          if (IsKeyPressed(KEY_Q)) shouldClose = true;
          if (IsKeyPressed(KEY_I)) editorMode = INSERT;

          if (IsKeyPressed(KEY_H) && cursorX > 0) cursorX--;
          if (IsKeyPressed(KEY_L) && cursorX < BUFFW - 1) cursorX++;
          if (IsKeyPressed(KEY_J) && cursorY < BUFFH - 1) cursorY++;
          if (IsKeyPressed(KEY_K) && cursorY > 0) cursorY--;
        } break;
      case INSERT:
        {
          if (IsKeyPressed(KEY_ESCAPE)) editorMode = NORMAL;
        } break;
      default: break;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    DrawRectangle(SCREENPADDING+(FONTCHARW+FONTSPACING)*cursorX-(FONTSPACING>>1), SCREENPADDING+FONTCHARH*cursorY, FONTCHARW+FONTSPACING, FONTCHARH, GRAY);

    DrawTextEx(editorFont, BUFF, (Vector2){(float)SCREENPADDING, (float)SCREENPADDING}, FONTCHARH, FONTSPACING, RAYWHITE);
    if (editorMode == NORMAL) {
      DrawTextEx(editorFont, "NORMAL", (Vector2){(float)SCREENPADDING, (float)(SH-SCREENPADDING-FONTCHARH)}, FONTCHARH, FONTSPACING, GRAY);
    } else if (editorMode == INSERT) {
      DrawTextEx(editorFont, "INSERT", (Vector2){(float)SCREENPADDING, (float)(SH-SCREENPADDING-FONTCHARH)}, FONTCHARH, FONTSPACING, GRAY);
    }
    EndDrawing();
  }

  UnloadFont(editorFont);

  CloseWindow();

  return 0;
}
