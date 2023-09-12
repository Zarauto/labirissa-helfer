/*
 * @Author: Álvaro Becker et al.
*/

#include <bits/stdc++.h>
#include "libs/EdubotLib.hpp"

#define SON_ESQ 0		// Sonar esquerdo
#define SON_DIR 6		// Sonar direito
#define SON_FRENTE 3	// Sonar frontal
#define SON_NE 4		// Sonar nordeste

#define SIMU 1
#define OBST 20/pow(100,SIMU)
#define WAIT 2000		// Tempo de espera para rotação e parada
#define D_THETA 4		// Valor de tolerância para rotação
#define CONT_SON 15		// Número de ticks para validar a leitura do sonar
#define VEL 0.3
#define DT_STEP 2000

using namespace std;

void backStep(EdubotLib* edubot){
	edubot->move(-VEL);
	edubot->sleepMilliseconds(DT_STEP);
	edubot->stop();
	edubot->sleepMilliseconds(WAIT);
}

void step(EdubotLib* edubot){
	edubot->move(VEL);
	edubot->sleepMilliseconds(DT_STEP);
	edubot->stop();
	edubot->sleepMilliseconds(WAIT);
}

void halfStep(EdubotLib* edubot){
	edubot->move(VEL);
	edubot->sleepMilliseconds(DT_STEP/2);
	edubot->stop();
	edubot->sleepMilliseconds(WAIT);
}

// Normaliza um ângulo na faixa de -180 a 180 graus
double normAngulo(double a){
	
	if(a > 180){
		a -= 360;
	}

	if(a < -180){
		a += 360;
	}
	return a;
}

// Retorna o ângulo do robô, na faixa de -180 a 180 graus
double angulo(EdubotLib *edubot){
	return normAngulo(edubot->getTheta());
}

// Rotaciona o robô
void gira(EdubotLib *edubot, double ang){

	// Ângulo inicial
	double a0 = angulo(edubot);

	//cout << "a0: " << a0 << endl;

	edubot->rotate(ang);
	edubot->sleepMilliseconds(WAIT);

	// Ângulo após rotação
	double a1 = angulo(edubot);

	//cout << "a1: " << a1 << endl;

	// Ângulo rotacionado de fato pelo robô
	double da = normAngulo(a0 - a1);

	// Enquanto estiver fora da faixa de tolerância,
	// corrige a rotação
	while(da > ang + D_THETA || da < ang - D_THETA){
		if(edubot->getBumper(0) or edubot->getBumper(1)){
			backStep(edubot);
		}
		if(edubot->getBumper(2) or edubot->getBumper(3)){
			halfStep(edubot);
		}
		double correcao = normAngulo(ang - da);
		edubot->rotate(correcao);
		edubot->sleepMilliseconds(WAIT);

		a1 = angulo(edubot);
		da = normAngulo(a0 - a1);
	}
}

bool paredeNaFrente(EdubotLib *edubot){
	double dist = 0; // Distância na frente do edubot

	// Lê a distância um número de vezes e faz a média
	for(int i=0; i<CONT_SON;i++){
		dist += edubot->getSonar(SON_FRENTE);
	}
	dist /=CONT_SON;

	return dist <= OBST; // Se a média for menor ou igual ao limite de distância para ser considerado obstáculo, retorna verdadeiro
}

bool paredeNaEsquerda(EdubotLib *edubot){
	double dist = 0; // Distância na frente do edubot

	// Lê a distância um número de vezes e faz a média
	for(int i=0; i<CONT_SON;i++){
		dist += edubot->getSonar(SON_ESQ);
	}
	dist /=CONT_SON;

	return dist <= OBST; // Se a média for menor ou igual ao limite de distância para ser considerado obstáculo, retorna verdadeiro
}

bool paredeNaDireita(EdubotLib *edubot){
	double dist = 0; // Distância na frente do edubot

	// Lê a distância um número de vezes e faz a média
	for(int i=0; i<CONT_SON;i++){
		dist += edubot->getSonar(SON_DIR	);
	}
	dist /=CONT_SON;

	return dist <= OBST; // Se a média for menor ou igual ao limite de distância para ser considerado obstáculo, retorna verdadeiro
}

bool colidindoFrente(EdubotLib* edubot){
	return edubot->getBumper(0) or edubot->getBumper(1);
}

int escolheLado(){
	return rand()%2;
}

void viraAleatorio(EdubotLib* edubot){
	gira(edubot, 90*pow(-1,escolheLado()));
}

void escolheCaminho(EdubotLib* edubot){

	bool esq = paredeNaEsquerda(edubot), dir = paredeNaDireita(edubot), frente = paredeNaFrente(edubot);

	// Beco sem saída
	if(frente and esq and dir){
		gira(edubot, 180);
		//edubot->sleepMilliseconds(WAIT);
		return;
	}

	
	// Só frente livre
	if((not frente) and esq and dir){
		return;
	}

	// Só esquerda livre
	if((not esq) and frente and dir){
		gira(edubot, -90);
		return;
	}

	// Só direita livre
	if((not dir) and frente and esq){
		gira(edubot, 90);
		return;
	}

	// Frente ou esquerda
	if((not frente) and (not esq) and (dir)){
		gira(edubot, -90*escolheLado());
		return;
	}

	// Frente ou direita
	if((not frente) and (esq) and (not dir)){
		gira(edubot, 90*escolheLado());
		return;
	}

	// Esquerda ou direita
	if((frente) and (not esq) and (not dir)){
		viraAleatorio(edubot);
		return;
	}

	//Qualquer um
	int lado = rand()%3;

	if(lado == 0){
		return;
	}

	viraAleatorio(edubot);
}

void passoLado(EdubotLib* edubot, int lado){
	gira(edubot, 90*lado);
	edubot->sleepMilliseconds(WAIT);
	halfStep(edubot);
	gira(edubot, -90*lado);
	edubot->sleepMilliseconds(WAIT);
	
}

void checaBumpers(EdubotLib* edubot){
	bool bumpers[4];

	for(int i=0;i<4;i++){
		bumpers[i]=edubot->getBumper(i);
	}

	if(bumpers[0] or bumpers[1]){
		edubot->stop();
		edubot->sleepMilliseconds(WAIT);
		backStep(edubot);

		if(bumpers[0] and bumpers[1]){
			return;
		}

		if(bumpers[0]){
			passoLado(edubot,1);
			return;
		}

		if(bumpers[1]){
			passoLado(edubot,-1);
		}
		
	}

}

int main(){
	srand (time(NULL));

	EdubotLib *edubot = new EdubotLib();

	//try to connect on robot
	if(edubot->connect()){

		edubot->sleepMilliseconds(WAIT);

		while(true){

			escolheCaminho(edubot);
			edubot->sleepMilliseconds(WAIT);
			step(edubot);			
			checaBumpers(edubot);			
	
		}


		edubot->disconnect();
	}
	else{
		cout << "Could not connect on robot!" << std::endl;
	}

	return 0;
}
