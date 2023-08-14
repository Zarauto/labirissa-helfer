/*
 * @Author: Álvaro Becker et al.
*/

#include <bits/stdc++.h>
#include "libs/EdubotLib.hpp"

#define SON_ESQ 0		// Sonar esquerdo
#define SON_DIR 6		// Sonar direito
#define SON_FRENTE 3	// Sonar frontal
#define SON_NE 4		// Sonar nordeste

#define DX 0.125		// Distância para o Edubot se mover a cada turno
#define VEL 1		// Velocidade de movimento do Edubot
#define OBST DX*1.5		// Distância para detecção de obstáculo
#define RANGE_MAX 2 	// Máximo alcance do sonar
#define WAIT 2000		// Tempo de espera para rotação e parada
#define D_THETA 4		// Valor de tolerância para rotação
#define CONT_SON 15		// Número de ticks para validar a leitura do sonar

using namespace std;

// Função que mede o tempo necessário para percorrer o DX na velocidade VEL
// Executa uma vez no início do programa
int medeDT(EdubotLib* edubot){

	// Verifica cada direção para achar uma sem obstáculo e dentro do range de detecção
	int i;
	for(i = 0; i < 4; i++){
		auto d = edubot->getSonar(SON_FRENTE);
		if(d <= 2*DX || d >= RANGE_MAX){
			edubot->rotate(-90);
			edubot->sleepMilliseconds(WAIT);
			continue;
		}
		break;
	}

	// Alterar para considerar caso em que começa no nada

	// Acha distância inicial a obstáculo
	auto d0 = edubot->getSonar(SON_FRENTE);

	// Marca início do período de medição
	auto ini = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());

	edubot->move(VEL);

	// Percorre a distância 2dx
	while(edubot->getSonar(SON_FRENTE) > d0 - DX);

	// Marca fim do período de medição
	auto fim = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
	
	edubot->neutral();
	edubot->sleepMilliseconds(WAIT);

	int dt = (fim-ini).count();

	// Retorna Edubot para posição inicial
	edubot->move(-VEL);
	edubot->sleepMilliseconds(dt);
	edubot->stop();
	edubot->sleepMilliseconds(WAIT);
	
	return dt;
	
}

// Faz o robô dar um passo
void step(EdubotLib *edubot, int dt){
	//if(DX> 
	edubot->move(VEL);
	edubot->sleepMilliseconds(dt);
	edubot->stop();
	edubot->sleepMilliseconds(WAIT);	
	cout << "Dei um passo\n";
}

// Indica se o robô pode andar para a frente sem colidir
bool frenteLivre(EdubotLib *edubot){
	return edubot->getSonar(SON_FRENTE) > OBST;
}

// Garante que há uma parede à direita do robô
bool verificaParede(EdubotLib *edubot){

	for(int i=0; i<CONT_SON; i++){
		if(edubot->getSonar(SON_DIR) > OBST){
			return false;
		}
		edubot->sleepMilliseconds(10);
	}

	return true;	
}

// Verifica se o robô não percebe mais parede à direita
bool verificaVazio(EdubotLib *edubot){

	for(int i=0; i<CONT_SON; i++){
		if(edubot->getSonar(SON_DIR) <= OBST){
			return false;
		}
	}

	return true;	
}

void andaFrente(EdubotLib *edubot, char interrupt = '0', char *motivo = NULL){
	// Se houver um obstáculo, não anda
	if(!frenteLivre(edubot)){
		return;
	}

	// Anda até achar uma parede na frente ou ser interrompido
	edubot->move(VEL);

	while(frenteLivre(edubot)){
		// Eventuais funções que interrompam o movimento

		// Interrompe se achar parede à direita
		if(interrupt == 'd' && verificaParede(edubot)){
			edubot->stop();
			edubot->sleepMilliseconds(WAIT);
			*motivo = 'd';
			return;
		}

		// Interrompe se parar de perceber parede à direita
		if(interrupt == 'v' && verificaVazio(edubot)){
			edubot->stop();
			edubot->sleepMilliseconds(WAIT);
			*motivo = 'v';
			return;			
		}
	}

	edubot->stop();
	edubot->sleepMilliseconds(WAIT);
	*motivo = 'f';
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
		double correcao = ang - da;
		edubot->rotate(correcao);
		edubot->sleepMilliseconds(WAIT);

		a1 = angulo(edubot);
		da = normAngulo(a0 - a1);
	}
}

// Acha uma parede à direita do robô
void achaParede(EdubotLib *edubot){

	char dir;

	if(verificaParede(edubot)){
		return;	
	}

	andaFrente(edubot, 'd', &dir);

	if(dir == 'f'){
		gira(edubot,-90);
	}	
}

void livraCaminho(EdubotLib *edubot, int dt){
	gira(edubot, 90);

	float d = edubot->getSonar(SON_NE);

	gira(edubot, -90);

	step(edubot, dt*d/DX);

	gira(edubot,90);	
}

void maoDireita(EdubotLib *edubot, int dt){

	while(true){
	achaParede(edubot);
	cout << "Achei uma parede\n";

	char motivo;

	andaFrente(edubot, 'v', &motivo);
	cout << "Estou andando\n";

	if(motivo == 'f'){
		cout << "Parei porque tinha um obstáculo\n";
		gira(edubot, -90);
		//return;
		continue;
	}
	cout << "Parei porque tava vazio\n";

	livraCaminho(edubot, dt);
	
	//andaFrente(edubot, 'd');
	}
}

/*


// Se a calibração do dt não pôde ser feita, acha outro ponto no mapa para calibrar
void reposiciona(EdubotLib* edubot){

	// Acha uma direção com caminho livre
	for(int i = 0; i < 4; i++){
		auto d = edubot->getSonar(SON_FRENTE);
		if(d <= 3*DX){
			edubot->rotate(-90);
			edubot->sleepMilliseconds(WAIT);
			continue;
		}
		break;
	}

	// Move até estar dentro da distância desejada
	edubot->move(VEL);
	while(edubot->getSonar(SON_FRENTE) > 3*DX);
	edubot->stop();
	edubot->sleepMilliseconds(WAIT);	
}


// Acha uma parede à direita do Edubot para ele seguir
void achaParede(EdubotLib *edubot, bool *seguindoParede, int dt){

	while(frenteLivre(edubot)){

		cout << edubot->getSonar(SON_DIR) << endl;

		
		// Se achar uma parede à direita, retorna
		if(edubot->getSonar(SON_DIR) <= OBST){
			*seguindoParede = true;
			return;		
		}

		step(edubot, dt);

	}

	// Se achar uma parede à frente, torna ela a direita e retorna
	edubot->rotate(-90);
	*seguindoParede = true;
	return;	
	
}


bool continuaParede(EdubotLib *edubot, bool deuVolta, int *voltas, int dt){
	// Se já deu alguma volta seguindo essa parede e voltou à posição inicial,
	// então abandona essa parede
	if(deuVolta and *voltas == 0){
		return false;
	}

	step(edubot, dt);
	edubot->rotate(90);
	edubot->sleepMilliseconds(WAIT);
	*voltas++;

	step(edubot, 2*dt);

	bool continua = edubot->getSonar(SON_DIR) <= OBST;
	step(edubot, dt);
	continua = continua && edubot->getSonar(SON_DIR) <= OBST;
	cout << continua << endl;
	return continua;
	
}
*/

int main(){

	EdubotLib *edubot = new EdubotLib();

	//try to connect on robot
	if(edubot->connect()){

		edubot->sleepMilliseconds(WAIT);

		/*
		//andaPraFrente(edubot);

		int dt = medeDT(edubot);
		cout << dt << endl;

		// Indica se o robô está seguindo uma parede ou procurando uma parede.
		bool seguindoParede = false;

		// Indica quantas vezes o robô girou
		int voltas = 0;

		// Auxiliar para determinar se o robô está seguindo uma parede desconexa
		bool deuVolta = false;

		
		// Loop principal
		while(true){

			// Se o robô não estiver seguindo uma parede,
			// acha uma parede para seguir
			if(!seguindoParede){
				achaParede(edubot, &seguindoParede, dt);
				voltas = 0; // Zera o número de voltas
				deuVolta = false;
			}


			// Se puder andar para a frente, dá um passo para a frente
			if(frenteLivre(edubot)){
				step(edubot,dt);
			}

			// Se estiver seguindo uma parede e for bloqueado, quer dizer que deve dobrar à esquerda
			else{
				edubot->rotate(-90);
				edubot->sleepMilliseconds(WAIT);
				voltas--;
				deuVolta = true;
			}

			// Se não detectar parede à direita
			if(edubot->getSonar(SON_DIR) > OBST){
				seguindoParede = continuaParede(edubot, deuVolta, &voltas, dt);
			}

			//cout << "Voltas: " << voltas << endl;		

			
			
		}*/

		int dt = medeDT(edubot);
		
		while(true){
			maoDireita(edubot, dt);
		}
		edubot->disconnect();
	}
	else{
		cout << "Could not connect on robot!" << std::endl;
	}

	return 0;
}
