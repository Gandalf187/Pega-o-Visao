#include <iostream>
#include <opencv2/opencv.hpp>
#include "menu.hpp"
#include "debouncer.hpp"

using namespace cv;
using namespace std;

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

// Laranja
Scalar lower_orange(10, 100, 100);
Scalar upper_orange(20, 255, 255);

// Cor de "pele"
Scalar lower_pele(0, 30, 60);
Scalar upper_pele(20, 150, 255);

class Desafio {
    public:
        string direcao;
        Scalar cor;
        string corNome;
        Desafio(string dir, Scalar c, string c_nome){
            direcao = dir;
            cor = c;
            corNome = c_nome;
        }
};

Desafio gerarDesafio() {
    vector<string> direcoes = {"Esquerda", "Direita", "Cima", "Baixo"};
    vector<pair<Scalar, string>> cores = {
        {Scalar(0, 255, 0), "Verde"},    // Verde
        {Scalar(0, 0, 255), "Vermelho"}, // Vermelho
        {Scalar(0, 0, 0), "Preto"}       // Preto
    };

    string dir = direcoes[rand() % direcoes.size()];
    double percent = (double) rand() / RAND_MAX;
    auto cor = (percent < 0.375) ? cores[0] : (percent < 0.75) ? cores[1] : cores[2];

    return Desafio(dir, cor.first, cor.second);
}



int main(){
    VideoCapture cap(0);
    if (!cap.isOpened()){
        cout << "Erro ao abrir webcam\n";
        return -1;
    }

    // Variáveis importantes para o jogo
    int pontuacao = 0;
    int dificuldade = 1;
    int delay = 2000; // tempo entre desafios em ms
    bool colisao = false;
    int64 ultimoTempo = getTickCount();
    int64 intervalo = delay * getTickFrequency() / 1000;
    Desafio desafioAtual = gerarDesafio();
    int menu = 0; // 0 - Tela Inicial; 1 - JOGAR; 2 - Tutorial; 3 - Pontuações; 4 - Créditos; 5 - Sair

    
    // Crie debouncers para cada área de interação
    Debouncer debouncerMenu(500); // 500ms para o menu principal
    Debouncer debouncerTop(300), debouncerBottom(300), debouncerLeft(300), debouncerRight(300); // 300ms para as áreas do jogo

    // Processo de captura do vídeo/imagem
    Mat frame, hsv, mask;
    while (true) {
        cap >> frame;
        if (frame.empty())
            break;

        // Configurações da janela inicial
        flip(frame, frame, 1);
        int width = 720, height = 540;
        namedWindow("Deteccao Full Body", WINDOW_NORMAL);
        resizeWindow("Deteccao Full Body", width, height);
        int x = (int)cap.get(CAP_PROP_FRAME_WIDTH); // Tamanho largura
        int y = (int)cap.get(CAP_PROP_FRAME_HEIGHT); // Tamanho altura

        // Converte para HSV
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        // Segmenta cores do objeto
        inRange(hsv, lower_yellow, upper_yellow, mask);
        // inRange(hsv, lower_red2, upper_red2, mask); // Para vermelho
        
        // Limpeza com operações morfológicas
        morphologyEx(mask, mask, MORPH_OPEN, Mat(), Point(-1,-1), 2);
        morphologyEx(mask, mask, MORPH_DILATE, Mat(), Point(-1,-1), 1);

        // Encontra contornos
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // Área dos botões do menu principal
        Rect jogar(Point(0, 0), Point(x * 0.2, y * 0.2));
        Rect tutorial(Point(x * 0.2, 0), Point(x * 0.4, y * 0.2));
        Rect pontuacoes(Point(x * 0.4, 0), Point(x * 0.6, y * 0.2));
        Rect creditos(Point(x * 0.6, 0), Point(x * 0.8, y * 0.2));
        Rect sair(Point(x * 0.8, 0), Point(x, y * 0.2));

        // Área dos botões do menu Jogar
        Rect top(Point(0, 0), Point(x, y * 0.15));
        Rect bottom(Point(0, y * 0.85), Point(x, y));
        Rect left(Point(0, y * 0.15), Point(x * 0.10, y * 0.85));
        Rect right(Point(x * 0.9, y * 0.15), Point(x, y * 0.85));

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
            
            if(menu == 0){
                if(debouncerMenu.debounce(detectedJogar, currentTime)){
                    menu = 1;
                    cout << "Menu Jogar\n";
                }
                else if(debouncerMenu.debounce(detectedTutorial, currentTime)){
                    menu = 2;
                    cout << "Menu Tutorial\n";
                }
                else if(debouncerMenu.debounce(detectedPontuacoes, currentTime)){
                    menu = 3;
                    cout << "Menu Pontuações\n";
                }
                else if(debouncerMenu.debounce(detectedCreditos, currentTime)){
                    menu = 4;
                    cout << "Menu Créditos\n";
                }
                else if(debouncerMenu.debounce(detectedSair, currentTime)){
                    cout << "Saindo...\n";
                    cap.release();
                    destroyAllWindows();
                    return 0;
                }
            }    
            else if(menu == 1){
                if(!colisao){
                    if (debouncerTop.debounce((top & box).area() > 0, currentTime)){
                        if((desafioAtual.direcao == "Cima" && desafioAtual.cor == Scalar(0, 255, 0))
                        || (desafioAtual.direcao == "Baixo" && desafioAtual.cor == Scalar(0, 0, 255)))
                        {
                            pontuacao++;
                            colisao = true;
                        }
                        else{
                            pontuacao--;
                        }
                    }
                    else if (debouncerBottom.debounce((bottom & box).area() > 0, currentTime)){
                        if((desafioAtual.direcao == "Baixo" && desafioAtual.cor == Scalar(0, 255, 0))
                        || (desafioAtual.direcao == "Cima" && desafioAtual.cor == Scalar(0, 0, 255)))
                        {
                            pontuacao++;
                            colisao = true;
                        }
                        else{
                            pontuacao--;
                        }
                    }
                    else if (debouncerLeft.debounce((left & box).area() > 0, currentTime)){
                        if((desafioAtual.direcao == "Esquerda" && desafioAtual.cor == Scalar(0, 255, 0))
                        || (desafioAtual.direcao == "Direita" && desafioAtual.cor == Scalar(0, 0, 255)))
                        {
                            pontuacao++;
                            colisao = true;
                        }
                        else{
                            pontuacao--;
                        }
                    }
                    else if (debouncerRight.debounce((right & box).area() > 0, currentTime)){
                        if((desafioAtual.direcao == "Direita" && desafioAtual.cor == Scalar(0, 255, 0))
                        || (desafioAtual.direcao == "Esquerda" && desafioAtual.cor == Scalar(0, 0, 255)))
                        {
                            pontuacao++;
                            colisao = true;
                        }
                        else{
                            pontuacao--;
                        }
                    }
                    if(((top & box).area() > 0 
                        ||(bottom & box).area() > 0
                        ||  (left & box).area() > 0
                        || (right & box).area() > 0)
                        && desafioAtual.cor == Scalar(0, 0, 0)){
                        pontuacao--;
                        colisao = true;
                    }
                }
                else{

                }
            }
            
            // Desenha o contorno principal
            // drawContours(frame, vector<vector<Point>>{contour}, -1, Scalar(255,0,0), 2);
            
            // Preenche o contorno com a cor 
            // drawContours(overlay, vector<vector<Point>>{contour}, -1, Scalar(255, 200, 0), FILLED);

            // Combina com transparência (0.5 = 50%)
            addWeighted(overlay, 0.5, frame, 0.5, 0, frame);
            addWeighted(color_rect, 0.5, roi, 0.5, 0, roi);
        }

        if(menu == 0){
            // Desenhar botões
            rectangle(frame, jogar, Scalar(0, 0, 0), 2);
            rectangle(frame, tutorial, Scalar(0, 0, 0), 2);
            rectangle(frame, pontuacoes, Scalar(0, 0, 0), 2);
            rectangle(frame, creditos, Scalar(0, 0, 0), 2);
            rectangle(frame, sair, Scalar(0, 0, 0), 2);
        }

        else if(menu == 1){ // JOGAR
            // Desenhar as regiões de contato
            rectangle(frame, top, Scalar(0, 0, 255), 2); // Top
            rectangle(frame, bottom, Scalar(0, 255, 255), 2); // Bottom
            rectangle(frame, left, Scalar(255, 0, 0), 2); // Left
            rectangle(frame, right, Scalar(0, 255, 0), 2); // Right
            
            putText(frame, desafioAtual.direcao, Point(50, 100), FONT_HERSHEY_SIMPLEX, 1, desafioAtual.cor, 4);
            putText(frame, "Pontuacao: " + to_string(pontuacao), Point(50, 150), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
            putText(frame, "Dificuldade: " + to_string(dificuldade), Point(50, 200), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);

            // Troca de desafio após intervalo
            if (getTickCount() - ultimoTempo > intervalo) {
                desafioAtual = gerarDesafio();
                ultimoTempo = getTickCount();
    
                // Aumenta dificuldade
                if (pontuacao > 0 && pontuacao % 5 == 0 && delay > 800) {
                    dificuldade++;
                    delay -= 200;
                    intervalo = delay * getTickFrequency() / 1000;
                }
                colisao = false;
            }
        }
        imshow("Deteccao Full Body", frame);
        if (waitKey(10) ==  27 || waitKey(10) == 'q')
            break; // Esc ou Q para sair
    }
    cap.release();
    destroyAllWindows();
    return 0;
}
