#ifndef MENU_HPP
#define MENU_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

class Menu {
protected:
    int estado;
    Size frameSize;
    
public:
    Menu(int e = 0, Size size = Size(0,0)) : estado(e), frameSize(size) {}
    virtual ~Menu() {}
    
    int getEstado() const { return estado; }
    void setEstado(int n) { estado = n; }
    void setFrameSize(Size size) { frameSize = size; }
    
    virtual void exibirMenu(Mat& frame) = 0;
    virtual int verificarColisao(const Rect& objeto) = 0;
};

class MenuPrincipal : public Menu {
private:
    vector<Rect> regioesInterativas;
    vector<int> acoes;
    
public:
    MenuPrincipal(Size size) : Menu(0, size) {
        int x = size.width;
        int y = size.height;
        
        // Definir regiões interativas (equivalente aos botões)
        regioesInterativas = {
            Rect(Point(0, 0), Point(x * 0.2, y * 0.2)),       // Jogar (ação 1)
            Rect(Point(x * 0.2, 0), Point(x * 0.4, y * 0.2)), // Tutorial (ação 2)
            Rect(Point(x * 0.4, 0), Point(x * 0.6, y * 0.2)), // Pontuações (ação 3)
            Rect(Point(x * 0.6, 0), Point(x * 0.8, y * 0.2)), // Créditos (ação 4)
            Rect(Point(x * 0.8, 0), Point(x, y * 0.2))        // Sair (ação 5)
        };
        
        acoes = {1, 2, 3, 4, 5};
    }
    
    void exibirMenu(Mat& frame) override {
        // Desenhar áreas interativas
        vector<string> labels = {"Jogar", "Tutorial", "Pontuacoes", "Creditos", "Sair"};
        for (size_t i = 0; i < regioesInterativas.size(); i++) {
            rectangle(frame, regioesInterativas[i], Scalar(0, 0, 255), 2);
            putText(frame, labels[i], 
                   regioesInterativas[i].tl() + Point(10, 30), 
                   FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
        }
    }
    
    int verificarColisao(const Rect& objeto) override {
        // Verifica colisão do objeto com as regiões
        for (size_t i = 0; i < regioesInterativas.size(); i++) {
            if ((regioesInterativas[i] & objeto).area() > 0) {
                return acoes[i]; // Retorna ação correspondente
            }
        }
        return -1; // Nenhuma colisão
    }
};

class MenuJogo : public Menu {
private:
    struct Desafio {
        string direcao;
        Scalar cor;
        Rect areaAlvo;
        
        Desafio(string dir, Scalar c, Rect area) : 
            direcao(dir), cor(c), areaAlvo(area) {}
    };
    
    int pontuacao;
    int64 ultimoTempo;
    int64 intervalo;
    int delay;
    Desafio desafioAtual;
    vector<Rect> areasInteracao;
    
public:
    MenuJogo(Size size) : Menu(1, size), pontuacao(0), delay(2000),
        desafioAtual("", Scalar(0,0,0), Rect(0,0,0,0)) {
        
        intervalo = delay * getTickFrequency() / 1000;
        ultimoTempo = getTickCount();
        
        // Definir áreas de interação do jogo
        int x = size.width;
        int y = size.height;
        areasInteracao = {
            Rect(Point(0, y * 0.15), Point(x * 0.10, y * 0.85)),   // Esquerda
            Rect(Point(x * 0.9, y * 0.15), Point(x, y * 0.85)),     // Direita
            Rect(Point(0, 0), Point(x, y * 0.15)),                 // Cima
            Rect(Point(0, y * 0.85), Point(x, y))                   // Baixo
        };
        
        gerarNovoDesafio();
    }
    
    void exibirMenu(Mat& frame) override {
        // Desenhar áreas de interação
        vector<Scalar> cores = {
            Scalar(255, 0, 0),   // Esquerda - Azul
            Scalar(0, 255, 0),   // Direita - Verde
            Scalar(0, 0, 255),   // Cima - Vermelho
            Scalar(0, 255, 255)  // Baixo - Amarelo
        };
        
        for (size_t i = 0; i < areasInteracao.size(); i++) {
            rectangle(frame, areasInteracao[i], cores[i], 2);
        }
        
        // Desenhar desafio
        if(!desafioAtual.direcao.empty()) {
            putText(frame, desafioAtual.direcao, 
                   Point(frame.cols/2 - 100, 50), 
                   FONT_HERSHEY_SIMPLEX, 1.5, desafioAtual.cor, 3);
        }
        
        // Desenhar pontuação
        putText(frame, "Pontos: " + to_string(pontuacao), 
               Point(50, frame.rows - 30), 
               FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    }
    
    int verificarColisao(const Rect& objeto) override {
        // Verifica colisão com área do desafio atual
        if ((desafioAtual.areaAlvo & objeto).area() > 0) {
            pontuacao++;
            gerarNovoDesafio();
            return 1; // Acerto
        }
        return 0; // Sem colisão relevante
    }
    
    void atualizarDificuldade() {
        if (getTickCount() - ultimoTempo > intervalo) {
            if (pontuacao > 0 && pontuacao % 5 == 0 && delay > 800) {
                delay -= 200;
                intervalo = delay * getTickFrequency() / 1000;
            }
            ultimoTempo = getTickCount();
        }
    }
    
private:
    void gerarNovoDesafio() {
        vector<string> direcoes = {"ESQUERDA", "DIREITA", "CIMA", "BAIXO"};
        vector<Scalar> cores = {
            Scalar(0, 255, 0),    // Verde
            Scalar(0, 0, 255),    // Vermelho
            Scalar(0, 0, 0)        // Preto
        };
        
        int idx = rand() % direcoes.size();
        desafioAtual = Desafio(
            direcoes[idx],
            cores[rand() % cores.size()],
            areasInteracao[idx]
        );
    }
};

#endif