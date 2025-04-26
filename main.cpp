#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include "menu.hpp"
#include "debouncer.hpp"

using namespace cv;
using namespace std;
using namespace std::chrono;

// Definições de cores HSV
Scalar lower_blue(100, 150, 50);
Scalar upper_blue(130, 255, 255);

Scalar lower_red1(0, 150, 50);
Scalar upper_red1(10, 255, 255);
Scalar lower_red2(170, 150, 50);
Scalar upper_red2(180, 255, 255);

Scalar lower_green(40, 70, 50);
Scalar upper_green(85, 255, 255);

Scalar lower_yellow(20, 100, 100);
Scalar upper_yellow(30, 255, 255);

Scalar lower_orange(10, 100, 100);
Scalar upper_orange(20, 255, 255);

Scalar lower_pele(0, 30, 60);
Scalar upper_pele(20, 150, 255);

class Desafio {
public:
    string direcao;
    Scalar cor;
    string corNome;
    
    Desafio(string dir, Scalar c, string c_nome) : 
        direcao(dir), cor(c), corNome(c_nome) {}
};

Desafio gerarDesafio() {
    vector<string> direcoes = {"Esquerda", "Direita", "Cima", "Baixo"};
    vector<pair<Scalar, string>> cores = {
        {Scalar(0, 255, 0), "Verde"},
        {Scalar(0, 0, 255), "Vermelho"},
        {Scalar(0, 0, 0), "Preto"}
    };

    string dir = direcoes[rand() % direcoes.size()];
    double percent = (double)rand() / RAND_MAX;
    auto cor = (percent < 0.375) ? cores[0] : 
              (percent < 0.75) ? cores[1] : cores[2];

    return Desafio(dir, cor.first, cor.second);
}

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Erro ao abrir webcam\n";
        return -1;
    }

    // Variáveis do jogo
    int pontuacao = 0;
    int dificuldade = 1;
    int delay = 2000;
    bool colisao = false;
    int64 ultimoTempo = getTickCount();
    int64 intervalo = delay * getTickFrequency() / 1000;
    Desafio desafioAtual = gerarDesafio();
    int menu = 0; // 0-Inicial, 1-Jogar, 2-Tutorial, 3-Pontuações, 4-Créditos

    // Debouncers
    Debouncer debouncerMenu(500);
    Debouncer debouncerTop(300), debouncerBottom(300), debouncerLeft(300), debouncerRight(300);

    Mat frame, hsv, mask;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // Configurações da janela
        flip(frame, frame, 1);
        int width = 720, height = 540;
        namedWindow("Deteccao Full Body", WINDOW_NORMAL);
        resizeWindow("Deteccao Full Body", width, height);
        int x = (int)cap.get(CAP_PROP_FRAME_WIDTH);
        int y = (int)cap.get(CAP_PROP_FRAME_HEIGHT);

        // Processamento de imagem
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        inRange(hsv, lower_yellow, upper_yellow, mask);
        morphologyEx(mask, mask, MORPH_OPEN, Mat(), Point(-1, -1), 2);
        morphologyEx(mask, mask, MORPH_DILATE, Mat(), Point(-1, -1), 1);

        // Detecção de contornos
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // Definição das áreas de menu
        Rect jogar(Point(0, 0), Point(x * 0.2, y * 0.2));
        Rect tutorial(Point(x * 0.2, 0), Point(x * 0.4, y * 0.2));
        Rect pontuacoes(Point(x * 0.4, 0), Point(x * 0.6, y * 0.2));
        Rect creditos(Point(x * 0.6, 0), Point(x * 0.8, y * 0.2));
        Rect sair(Point(x * 0.8, 0), Point(x, y * 0.2));

        // Áreas do jogo
        Rect top(Point(0, 0), Point(x, y * 0.15));
        Rect bottom(Point(0, y * 0.85), Point(x, y));
        Rect left(Point(0, y * 0.15), Point(x * 0.10, y * 0.85));
        Rect right(Point(x * 0.9, y * 0.15), Point(x, y * 0.85));

        // Variáveis de detecção
        bool detectedJogar = false, detectedTutorial = false;
        bool detectedPontuacoes = false, detectedCreditos = false, detectedSair = false;
        bool detectedTop = false, detectedBottom = false, detectedLeft = false, detectedRight = false;

        // Processamento de contornos
        for (auto& contour : contours) {
            if (contourArea(contour) < 1000) continue;

            Rect box = boundingRect(contour);
            rectangle(frame, box, Scalar(0, 255, 0), 2);

            // Verifica colisões
            if (menu == 0) {
                if ((jogar & box).area() > 0) detectedJogar = true;
                else if ((tutorial & box).area() > 0) detectedTutorial = true;
                else if ((pontuacoes & box).area() > 0) detectedPontuacoes = true;
                else if ((creditos & box).area() > 0) detectedCreditos = true;
                else if ((sair & box).area() > 0) detectedSair = true;
            }
            else if (menu == 1) {
                if ((top & box).area() > 0) detectedTop = true;
                if ((bottom & box).area() > 0) detectedBottom = true;
                if ((left & box).area() > 0) detectedLeft = true;
                if ((right & box).area() > 0) detectedRight = true;
            }
        }

        // Processamento com debounce
        if (menu == 0) {
            if (debouncerMenu.debounce(detectedJogar)) {
                menu = 1;
                cout << "Menu Jogar\n";
            }
            else if (debouncerMenu.debounce(detectedTutorial)) {
                menu = 2;
                cout << "Menu Tutorial\n";
            }
            else if (debouncerMenu.debounce(detectedPontuacoes)) {
                menu = 3;
                cout << "Menu Pontuações\n";
            }
            else if (debouncerMenu.debounce(detectedCreditos)) {
                menu = 4;
                cout << "Menu Créditos\n";
            }
            else if (debouncerMenu.debounce(detectedSair)) {
                cout << "Saindo...\n";
                cap.release();
                destroyAllWindows();
                return 0;
            }
        }
        else if (menu == 1) {
            if (!colisao) {
                bool shouldProcess = false;
                
                if (debouncerTop.debounce(detectedTop)) {
                    shouldProcess = true;
                    if ((desafioAtual.direcao == "Cima" && desafioAtual.cor == Scalar(0, 255, 0)) ||
                        (desafioAtual.direcao == "Baixo" && desafioAtual.cor == Scalar(0, 0, 255))) {
                        pontuacao++;
                        colisao = true;
                    }
                    else {
                        pontuacao--;
                    }
                }
                else if (debouncerBottom.debounce(detectedBottom)) {
                    shouldProcess = true;
                    if ((desafioAtual.direcao == "Baixo" && desafioAtual.cor == Scalar(0, 255, 0)) ||
                        (desafioAtual.direcao == "Cima" && desafioAtual.cor == Scalar(0, 0, 255))) {
                        pontuacao++;
                        colisao = true;
                    }
                    else {
                        pontuacao--;
                    }
                }
                else if (debouncerLeft.debounce(detectedLeft)) {
                    shouldProcess = true;
                    if ((desafioAtual.direcao == "Esquerda" && desafioAtual.cor == Scalar(0, 255, 0)) ||
                        (desafioAtual.direcao == "Direita" && desafioAtual.cor == Scalar(0, 0, 255))) {
                        pontuacao++;
                        colisao = true;
                    }
                    else {
                        pontuacao--;
                    }
                }
                else if (debouncerRight.debounce(detectedRight)) {
                    shouldProcess = true;
                    if ((desafioAtual.direcao == "Direita" && desafioAtual.cor == Scalar(0, 255, 0)) ||
                        (desafioAtual.direcao == "Esquerda" && desafioAtual.cor == Scalar(0, 0, 255))) {
                        pontuacao++;
                        colisao = true;
                    }
                    else {
                        pontuacao--;
                    }
                }

                if (shouldProcess && desafioAtual.cor == Scalar(0, 0, 0)) {
                    pontuacao--;
                    colisao = true;
                }
            }
        }

        // Desenho da interface
        if (menu == 0) {
            // Menu principal
            rectangle(frame, jogar, Scalar(0, 0, 0), 2);
            putText(frame, "Jogar", Point(jogar.x + 20, jogar.y + 30), 
                   FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 0), 2);
            
            rectangle(frame, tutorial, Scalar(0, 0, 0), 2);
            putText(frame, "Tutorial", Point(tutorial.x + 20, tutorial.y + 30), 
                   FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 0), 2);
            
            rectangle(frame, pontuacoes, Scalar(0, 0, 0), 2);
            putText(frame, "Pontuacoes", Point(pontuacoes.x + 10, pontuacoes.y + 30), 
                   FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 0), 2);
            
            rectangle(frame, creditos, Scalar(0, 0, 0), 2);
            putText(frame, "Creditos", Point(creditos.x + 20, creditos.y + 30), 
                   FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 0), 2);
            
            rectangle(frame, sair, Scalar(0, 0, 0), 2);
            putText(frame, "Sair", Point(sair.x + 30, sair.y + 30), 
                   FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 0), 2);
        }
        else if (menu == 1) {
            // Jogo
            rectangle(frame, top, Scalar(0, 0, 255), 2);
            rectangle(frame, bottom, Scalar(0, 255, 255), 2);
            rectangle(frame, left, Scalar(255, 0, 0), 2);
            rectangle(frame, right, Scalar(0, 255, 0), 2);
            
            putText(frame, desafioAtual.direcao, Point(50, 100), 
                   FONT_HERSHEY_SIMPLEX, 1, desafioAtual.cor, 4);
            putText(frame, "Pontuacao: " + to_string(pontuacao), Point(50, 150), 
                   FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
            putText(frame, "Dificuldade: " + to_string(dificuldade), Point(50, 200), 
                   FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);

            // Troca de desafio
            if (getTickCount() - ultimoTempo > intervalo) {
                desafioAtual = gerarDesafio();
                ultimoTempo = getTickCount();
                
                if (pontuacao > 0 && pontuacao % 5 == 0 && delay > 800) {
                    dificuldade++;
                    delay -= 200;
                    intervalo = delay * getTickFrequency() / 1000;
                }
                colisao = false;
            }
        }

        imshow("Deteccao Full Body", frame);
        if (waitKey(10) == 27 || waitKey(10) == 'q')
            break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}