#pragma once
#ifndef MIXER_HPP
#define MIXER_HPP

#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_mixer.h>

class Mixer{
    private:
        static const int MIN_MAX_VOLUME = 128;
        Mix_Music *musica;
        Mix_Chunk *soundEffect;

    public:
    
        int setupMixer(){
            if (SDL_Init(SDL_INIT_AUDIO) < 0) {
                std::cout << "Erro ao iniciar SDL: " << SDL_GetError() << std::endl;
                throw 1;
            }
            
            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
                std::cout << "Erro ao iniciar SDL_mixer: " << Mix_GetError() << std::endl;
                throw 2;
            }
        }
        
        Mixer(){
            setupMixer();    
        }

        int loadMusic(char* path){
            musica = Mix_LoadMUS(path);
            if (!musica) {
                std::cout << "Erro ao carregar a música: " << Mix_GetError() << std::endl;
                return -1;
            }
        }
        int loadSoundEffect(char* path){
            soundEffect = Mix_LoadWAV(path);
            if(!soundEffect){
                std::cout << "Erro ao carregar o efeito sonoro\n";
                return -1;
            }
        }

        void playMusic(){
            Mix_PlayMusic(musica, 0); // 1 = toca uma vez (0 = infinito)    
        }
        void playSoundEffect(){
            Mix_PlayChannel(1, soundEffect, 0);
            while(Mix_Playing(1)){
                SDL_Delay(100);
            }
            Mix_FreeChunk(soundEffect); 
        }

        void setVolume(int tipo, int vol){ // 1 = música; 2 = efeito sonoro
            if(tipo == 1){
                Mix_VolumeMusic(MIN_MAX_VOLUME * ((float)vol/100.0));
            }
            else if (tipo == 2){
                Mix_VolumeChunk(soundEffect, MIN_MAX_VOLUME * ((float)vol / 100.0));
            }
        }

        void quitMixer(){
            Mix_FreeMusic(musica);
            Mix_FreeChunk(soundEffect);
            Mix_CloseAudio();
            SDL_Quit();
        }

};

#endif