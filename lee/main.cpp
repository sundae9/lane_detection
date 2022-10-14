#include <iostream>
#include "../common/timer.cpp"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define width 640
#define height 480
#define proc_size 20

typedef Vec4i Line;

bool angle_filter(Line line, double angle_threshold)
{
    double dx = abs(line[2] - line[0]);
    double dy = abs(line[3] - line[1]);
    double increse_rate = dy / dx;

    int w = width / 2;
    if (angle_threshold <= increse_rate && !(line[2] < (w / 2) && line[0] > (w / 2)) && !(line[2] > (w / 2) && line[0] < (w / 2)))
        return true;

    return false;
}

bool cross_filter(Line line)
{
    int mid = width / 2;
    if ((line[0] > mid && line[2] > mid) || (line[0] < mid && line[2] < mid))
        return true;

    return false;
}

void info_mat(Mat mat)
{
    cout << "width : " << mat.cols << endl;
    cout << "height : " << mat.rows << endl;
    cout << "channels : " << mat.channels() << endl;
}

int main()
{
    VideoCapture cap;
    int w, h;
    Mat frame, resize_frame, gray_frame, mask;
    vector<Point> pts;
    TickMeter tm, tm2;
    Mat binarization, edge, dst, closed_edge;
    vector<Vec4i> lines, filtered_lines;
    double fps;
    string filepath = "src/";
    string filename = "실선 + 음영 + 노면표시.avi";

    Video_info vi;
    Initialize_Video_info(vi);
    int idx = 0;

    cap.open(filepath + filename, CAP_ANY);

    if (!cap.isOpened())
    {
        cerr << " img read failed!" << endl;
        return -1;
    }

    w = 640;
    h = 360;

    fps = cap.get(CAP_PROP_FPS);

    int delay = cvRound(1000 / fps);

    pts.push_back(Point(120, 300));
    pts.push_back(Point(310, 180));
    pts.push_back(Point(330, 180));
    pts.push_back(Point(520, 300));

    mask = Mat::zeros(h, w, CV_8UC1);
    fillPoly(mask, pts, Scalar(255), LINE_AA);
    mask = ~mask;
    double total;
    vi.total_frame = 0;
    for (int i = 0; i < 20; i++)
    {
        vi.time_info.min_time[i] = 100;
    }

    while (true)
    {
        long t1 = getTickCount();

        vi.total_frame += 1;

        cap >> frame;
        if (frame.empty())
            break;
        idx = 0;
        tm.reset();
        tm.start();
        tm2.reset();
        tm2.start();
        // resize(frame, resize_frame, Size(w, h));
        // Time_record(tm, vi, idx++);

        cvtColor(frame, gray_frame, COLOR_BGR2GRAY); // 3채널 -> 1채널
        Time_record(tm, vi, idx++);

        adaptiveThreshold(gray_frame, binarization, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 3, 3); // 이진화
        Time_record(tm, vi, idx++);

        binarization.setTo(Scalar(0), mask); // 마스킹
        Time_record(tm, vi, idx++);

        Canny(binarization, edge, 50, 150, 7); // 에지 검출
        Time_record(tm, vi, idx++);
        // morphologyEx(edge, closed_edge, MORPH_OPEN, Mat());         // 모폴로지 닫기 연산
        HoughLinesP(edge, lines, 1, CV_PI / 180, 30, 10, 5); // 직선 검출
        Time_record(tm, vi, idx++);

        filtered_lines.clear();

        for (Vec4i l : lines)
        {
            if (angle_filter(l, 0.5))
            {
                filtered_lines.push_back(l);
            }
        }
        Time_record(tm, vi, idx++);

        lines = filtered_lines;
        filtered_lines.clear();

        for (Vec4i l : lines)
        {
            if (cross_filter(l))
            {
                filtered_lines.push_back(l);
            }
        }
        Time_record(tm, vi, idx++);

        dst = frame.clone();

        for (Vec4i l : filtered_lines)
        {
            line(dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 2, LINE_AA);
        }

        line(dst, Point(320, 0), Point(320, 360), Scalar(0, 255, 0), 2, LINE_AA);
        Time_record(tm, vi, idx++);

        imshow("original", frame);
        imshow("dst", dst);
        imshow("binarization", binarization);
        // imshow("closed_edge", closed_edge);

        resizeWindow("original", w, h);
        resizeWindow("dst", w, h);
        resizeWindow("binarization", w, h);
        // resizeWindow("closed_edge", w, h);

        moveWindow("dst", w, 0);
        moveWindow("binarization", 0, h);
        // moveWindow("closed_edge", w, h);

        Time_record(tm2, vi, idx++);

        tm.stop();

        if (vi.total_frame >= 20)
            break;

        if (waitKey(delay) > 0)
        {
            imwrite("tmp.png", frame);
            waitKey();
        }
    }

    destroyAllWindows();

    Print_info_all(vi, idx);
    cout << "total frame : " << vi.total_frame << endl;

    return 0;
}