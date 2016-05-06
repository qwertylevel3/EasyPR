#include "easypr/core/plate_detect.h"
#include "easypr/util/util.h"

namespace easypr {

CPlateDetect::CPlateDetect() {
  m_plateLocate = new CPlateLocate();

  // 默认EasyPR在一幅图中定位最多3个车

  m_maxPlates = 3;
}

CPlateDetect::~CPlateDetect() { SAFE_RELEASE(m_plateLocate); }

int CPlateDetect::plateDetect(Mat src, std::vector<CPlate> &resultVec,
                              bool showDetectArea, int index) {
  std::vector<CPlate> color_Plates;
  std::vector<CPlate> sobel_Plates;
  std::vector<CPlate> color_result_Plates;
  std::vector<CPlate> sobel_result_Plates;

  std::vector<CPlate> all_result_Plates;

  //如果颜色查找找到n个以上（包含n个）的车牌，就不再进行Sobel查找了。

  const int color_find_max = m_maxPlates;

  m_plateLocate->plateColorLocate(src, color_Plates, index);
  PlateJudge::instance()->plateJudge(color_Plates, color_result_Plates);

  for (size_t i = 0; i < color_result_Plates.size(); i++) {
    CPlate plate = color_result_Plates[i];

    plate.setPlateLocateType(COLOR);
    all_result_Plates.push_back(plate);
  }

  //颜色和边界闭操作同时采用

  {
    m_plateLocate->plateSobelLocate(src, sobel_Plates, index);




    //show the temp plate location
    Mat plateLocationTemp;
    src.copyTo(plateLocationTemp);
    std::cout<<sobel_Plates.size()<<std::endl;
    for (size_t j = 0; j < sobel_Plates.size(); j++) {
        CPlate item = sobel_Plates[j];
        Mat plate = item.getPlateMat();


        RotatedRect minRect = item.getPlatePos();
        Point2f rect_points[4];
        minRect.points(rect_points);

        Scalar lineColor = Scalar(0, 255, 0);

        //if (item.getPlateLocateType() == SOBEL) lineColor = Scalar(255, 0, 0);

        //if (item.getPlateLocateType() == COLOR) lineColor = Scalar(0, 255, 0);

        for (int j = 0; j < 4; j++)
            line(plateLocationTemp, rect_points[j], rect_points[(j + 1) % 4], lineColor, 2,
                            8);
    }
    imwrite("temp_picture.jpg",plateLocationTemp);




    PlateJudge::instance()->plateJudge(sobel_Plates, sobel_result_Plates);

    for (size_t i = 0; i < sobel_result_Plates.size(); i++) {
      CPlate plate = sobel_result_Plates[i];

      plate.bColored = false;
      plate.setPlateLocateType(SOBEL);

      all_result_Plates.push_back(plate);
    }
  }

  for (size_t i = 0; i < all_result_Plates.size(); i++) {

    // 把截取的车牌图像依次放到左上角

    CPlate plate = all_result_Plates[i];
    resultVec.push_back(plate);
  }
  //std::cout<<color_Plates.size()<<std::endl;
  //std::cout<<sobel_Plates.size()<<std::endl;
  //std::cout<<color_result_Plates.size()<<std::endl;
  //std::cout<<sobel_result_Plates.size()<<std::endl;
  //std::cout<<resultVec.size()<<std::endl;

  return 0;
}

int CPlateDetect::showResult(const Mat &result) {
  namedWindow("EasyPR", CV_WINDOW_AUTOSIZE);

  const int RESULTWIDTH = 640;   // 640 930
  const int RESULTHEIGHT = 540;  // 540 710

  Mat img_window;
  img_window.create(RESULTHEIGHT, RESULTWIDTH, CV_8UC3);

  int nRows = result.rows;
  int nCols = result.cols;

  Mat result_resize;
  if (nCols <= img_window.cols && nRows <= img_window.rows) {
    result_resize = result;

  } else if (nCols > img_window.cols && nRows <= img_window.rows) {
    float scale = float(img_window.cols) / float(nCols);
    resize(result, result_resize, Size(), scale, scale, CV_INTER_AREA);

  } else if (nCols <= img_window.cols && nRows > img_window.rows) {
    float scale = float(img_window.rows) / float(nRows);
    resize(result, result_resize, Size(), scale, scale, CV_INTER_AREA);

  } else if (nCols > img_window.cols && nRows > img_window.rows) {
    Mat result_middle;
    float scale = float(img_window.cols) / float(nCols);
    resize(result, result_middle, Size(), scale, scale, CV_INTER_AREA);

    if (result_middle.rows > img_window.rows) {
      float scale = float(img_window.rows) / float(result_middle.rows);
      resize(result_middle, result_resize, Size(), scale, scale, CV_INTER_AREA);

    } else {
      result_resize = result_middle;
    }
  } else {
    result_resize = result;
  }

  Mat imageRoi = img_window(Rect((RESULTWIDTH - result_resize.cols) / 2,
                                 (RESULTHEIGHT - result_resize.rows) / 2,
                                 result_resize.cols, result_resize.rows));
  addWeighted(imageRoi, 0, result_resize, 1, 0, imageRoi);

  imshow("EasyPR", img_window);
  waitKey();

  destroyWindow("EasyPR");

  return 0;
}
}
