#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(){
    VideoCapture cap(0);
    if (!cap.isOpened()){
        cout << "Erro ao abrir webcam\n";
        return -1;
    }

    Mat frame, hsv, mask;
    while (true) {
        cap >> frame;
        if (frame.empty())
            break;
            
        flip(frame, frame, 1);

        // Converte para HSV
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        // Segmenta tons de pele (ajustável!)
        inRange(hsv, Scalar(0, 30, 60), Scalar(20, 150, 255), mask);

        // Limpeza com operações morfológicas
        morphologyEx(mask, mask, MORPH_OPEN, Mat(), Point(-1,-1), 2);
        morphologyEx(mask, mask, MORPH_DILATE, Mat(), Point(-1,-1), 1);

        // Encontra contornos
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        for (const auto& contour : contours) {
            if (contourArea(contour) < 1000)
                continue; // ignora pequenos

            // Convex hull (contorno mais "fechado")
            vector<int> hullIndices;
            convexHull(contour, hullIndices, false, false);

            // Cria imagem auxiliar preta
            Mat overlay = frame.clone();
            // overlay.setTo(Scalar(0, 0, 0));
            
            // Desenha o contorno principal
            drawContours(frame, vector<vector<Point>>{contour}, -1, Scalar(255,0,0), 2);
            
            // Preenche o contorno com a cor (aqui, azul)
            drawContours(overlay, vector<vector<Point>>{contour}, -1, Scalar(255, 200, 0), FILLED);

            // Combina com transparência (0.5 = 50%)
            addWeighted(overlay, 0.5, frame, 0.5, 0, frame);
        }

        imshow("Deteccao Full Body", frame);
        if (waitKey(30) ==  27 || waitKey(30) == 'q')
            break; // Esc ou Q para sair
    }
    return 0;
}
