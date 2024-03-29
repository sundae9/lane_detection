#ifndef VER3_0_CONSTANT_HPP
#define VER3_0_CONSTANT_HPP

// 영상 정보
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 360
#define FRAME_ROWS FRAME_HEIGHT
#define FRAME_COLS FRAME_WIDTH

// roi 설정 값
#define DX 15 // 2*DX = 동적 ROi 폭
#define DEFAULT_ROI_LEFT 100 // ROI 좌측 x값
#define DEFAULT_ROI_RIGHT 540 // ROI 우측 x값
#define DEFAULT_ROI_UP 195 // ROI 상단 y값
#define DEFAULT_ROI_DOWN 284 // ROI 하단 y값
#define DEFAULT_ROI_CENTER 320
#define DEFAULT_ROI_HEIGHT (DEFAULT_ROI_DOWN - DEFAULT_ROI_UP) // ROI 높이
#define DEFAULT_ROI_WIDTH (DEFAULT_ROI_RIGHT - DEFAULT_ROI_LEFT)
// 기타 변수
#define GRADIENT_STD 2 // cotanent(30 deg) = 1.73

#define HOUGH_PARAM1 30
#define HOUGH_PARAM2 40
#define HOUGH_PARAM3 40

// MODE
//#define SHOW //imshow  활성화
#define TIME_TEST // 출력 문구
//#define GRAPHIC
//#define VIDEO_SAVE
//#define DETECTION_RATE
//#define THRESH_DEBUG

#endif //VER3_0_CONSTANT_HPP
