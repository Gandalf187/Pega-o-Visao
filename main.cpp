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
    
    // Código das cores:
    // Azul
    Scalar lower_blue(100, 150, 50);
    Scalar upper_blue(130, 255, 255);

    // Vermelho (2 espectros)
    Scalar lower_red1(0, 150, 50);
    Scalar upper_red1(10, 255, 255);
    Scalar lower_red2(170, 150, 50);
    Scalar upper_red2(180, 255, 255);

    // Verde
    Scalar lower_green(40, 70, 50);
    Scalar upper_green(85, 255, 255);

    // Amarelo
    Scalar lower_yellow(20, 100, 100);
    Scalar upper_yellow(30, 255, 255);

    // Cor de "pele"
    Scalar lower_pele(0, 30, 60);
    Scalar upper_pele(20, 150, 255);

    // Processo de captura do vídeo/imagem
    Mat frame, hsv, mask;
    while (true) {
        cap >> frame;
        if (frame.empty())
            break;
            
        // Configurações da janela inicial
        flip(frame, frame, 1);
        int width = 960, height = 720;
        namedWindow("Deteccao Full Body", WINDOW_NORMAL);
        resizeWindow("Deteccao Full Body", width, height);
        int x = (int)cap.get(CAP_PROP_FRAME_WIDTH); // Tamanho largura
        int y = (int)cap.get(CAP_PROP_FRAME_HEIGHT); // Tamanho altura

        // Converte para HSV
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        // Segmenta cores
        inRange(hsv, lower_blue, upper_blue, mask);
        // inRange(hsv, lower_red2, upper_red2, mask); // Para vermelho

        // Desenhar as regiões de contato
        rectangle(frame, Point(0, 0), Point(x, y * 0.15), Scalar(0, 0, 255), 2); // Top
        rectangle(frame, Point(0, y * 0.85), Point(x, y), Scalar(0, 255, 255), 2); // Bottom
        rectangle(frame, Point(0, y * 0.15), Point(x * 0.10, y * 0.85), Scalar(255, 0, 0), 2); // Left
        rectangle(frame, Point(x * 0.9, y * 0.15), Point(x * 0.9, y * 0.85), Scalar(0, 255, 0), 2); // Right

        // Limpeza com operações morfológicas
        morphologyEx(mask, mask, MORPH_OPEN, Mat(), Point(-1,-1), 2);
        morphologyEx(mask, mask, MORPH_DILATE, Mat(), Point(-1,-1), 1);

        // Encontra contornos
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        for (auto& contour : contours) {
            if (contourArea(contour) < 1000)
                continue; // ignora pequenos

            // Convex hull (contorno mais "fechado")    
            vector<int> hullIndices;
            convexHull(contour, hullIndices, false, false);

            // Cria imagem auxiliar
            Mat overlay = frame.clone();
            // overlay.setTo(Scalar(0, 0, 0));
            
            // Desenha retângulos ao redor dos contornos
            Rect box = boundingRect(contour);
            rectangle(frame, box, Scalar(0, 255, 0), 2);
            Mat roi = frame(box);
            Mat color_rect(roi.size(), roi.type(), Scalar(255, 200, 0));

            // Desenha o contorno principal
            drawContours(frame, vector<vector<Point>>{contour}, -1, Scalar(255,0,0), 2);
            
            // Preenche o contorno com a cor 
            drawContours(overlay, vector<vector<Point>>{contour}, -1, Scalar(255, 200, 0), FILLED);

            // Combina com transparência (0.5 = 50%)
            addWeighted(overlay, 0.5, frame, 0.5, 0, frame);
            addWeighted(color_rect, 0.5, roi, 0.5, 0, roi);
        }

        imshow("Deteccao Full Body", frame);
        if (waitKey(10) ==  27 || waitKey(10) == 'q')
            break; // Esc ou Q para sair
    }
    return 0;
}
