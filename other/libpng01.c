#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>
#define MAX_Y 200
#define MAX_X 100

int main(int argc, const char **argv)
{
  int i, j;
  char map1[MAX_Y][MAX_X]; /* 書き込みたいデータ */
  uint8_t pngbuf[MAX_Y*MAX_X]; /* libpngのためのバッファ */
  /* for libpng */
  png_image image;
  uint32_t stride;
  memset(&image, 0, sizeof image);
  /* 画像の大きさを指定 */
  image.height = MAX_Y;
  image.width = MAX_X;
  /* 画像のフォーマットを指定(RGB, GRAYなどなど) */
  image.format = PNG_FORMAT_GRAY;
  /* libpngのバージョンを画像に埋め込む */
  image.version = PNG_IMAGE_VERSION;
  /* PNG_IMAGE_ROW_STRIDEマクロで必要な大きさを計算してくれる
   * (png_image_write_to_fileで使う) */
  stride = PNG_IMAGE_ROW_STRIDE(image);
  /* libpng ここまで */

  /* 実際にいじるのはここだけ 
   * pngにするためのバッファへデータをコピー
   * ただし、0~255 の範囲で黒から白を表すので255倍してる 
   * pngbufは1次元化しているので「横幅×縦方向の現在位置+横方向の現在位置」*/
  memset(map1, 1, sizeof(char)*MAX_Y*MAX_X); /* map1 を char型の 1 で埋め尽くす */
  for (j = 0; j < MAX_Y; j++){
    for (i = 0; i < MAX_X; i++){
      pngbuf[MAX_X*j + i] = 255*(uint8_t)map1[j][i];
    }
  }
  /* for libpng */
  png_image_write_to_file(&image, "test.png", 0/*convert_to_8bit*/, pngbuf, stride, NULL/*colormap*/);
  /* libpng ここまで */
  return 0;
}
