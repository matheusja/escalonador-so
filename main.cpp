
#include <cstddef> /* std::size_t*/
#include <cstdlib> /* std::atoi */

#include <algorithm> /* std::make_heap */
#include <exception> /* std::terminate */
#include <fstream> /* std::ifstream */
#include <iostream> /* std::c{err, out}*/
#include <queue>  /* std::queue */

struct processo_guardado {
    int chegada;
    long int duracao;
    long int memoria;
    int prioridade_inicial;
};


bool operator<(processo_guardado &left, processo_guardado &right) { /* std::make_heap requer isso */
	return left.chegada < right.chegada; 
}

std::istream &operator>>(std::istream &strm, processo_guardado &reg_processo) {
	char virg;
	strm >> reg_processo.chegada >> virg >> reg_processo.duracao >> virg >> reg_processo.memoria >> virg >> reg_processo.prioridade_inicial;
	return strm;
}

struct processo_na_fila {
    const static int TIME_UNTIL_PROMOTION = 10;
    enum class status {
    	/** O processo ficou TIME_UNTIL_PROMOTION na mesma fila, logo sua prioriade deve mudar(ou, não, se tiver prioridade 0 ou sempre ter prioridade 4)*/
        change_priority, 
		/** O processo encerrou a execução, retire-o da fila*/
        terminate,
        /** O processo ficou o quantum determinado na CPU, mas ainda não encerrou a execução, voltar para a fila*/
        wait
    };
    int prioridade_inicial;
    int prioridade_atual;
    long int duracao;
    long int tempo_restante;
    long int memoria;
    int time_in_queue;
    int lancamento;
    bool ascending;
    int chegada;
    processo_na_fila(){};
    processo_na_fila(const processo_guardado &ref, int time) :
    		prioridade_inicial(ref.prioridade_inicial), prioridade_atual(ref.prioridade_inicial), duracao(ref.duracao), tempo_restante(ref.duracao),
    		memoria(ref.memoria), time_in_queue(0),
    		lancamento(time), ascending(false), chegada(ref.chegada) { };
   	struct processo_na_fila &operator=(const processo_na_fila &ref) {
   		prioridade_inicial = ref.prioridade_inicial;
   		prioridade_atual = ref.prioridade_atual;
   		duracao = ref.duracao;
   		tempo_restante = ref.tempo_restante;
   		memoria = ref.memoria;
   		time_in_queue = ref.time_in_queue;
   		lancamento = ref.lancamento;
   		ascending = ref.ascending;
   		chegada = ref.chegada;
   		return *this;
   	}
    status operator++(int) {
        this->tempo_restante--;
        if (this->tempo_restante == 0) {
            return status::terminate;
        }
        this->time_in_queue++;
        if (this->time_in_queue == TIME_UNTIL_PROMOTION) {
            return status::change_priority;
        }
        return status::wait;
    };
};

std::ostream &operator<<(std::ostream &strm, processo_na_fila &processo) {
	return strm << processo.chegada << "," << processo.lancamento << "," << processo.duracao;
}






class fila_de_processos {
private:
	std::vector<processo_guardado> processos_a_lancar;
	std::queue<processo_na_fila> processos;
	int prioridade;
	bool degradar; /* Se é possível que a prioridade caia */

	void lancar_processos_possiveis(int &mem_disp, int tempo) {
		std::vector<processo_guardado>::iterator it(processos_a_lancar.begin());
		while (it != processos_a_lancar.end() && it->memoria < mem_disp) {
			mem_disp -= it->memoria;
			processos.push(processo_na_fila(*it, tempo));
			processos_a_lancar.erase(it);

			it = processos_a_lancar.begin(); /* std::vector::erase pode invalidar o iterador(e os seus sucessores)*/
		}
		for(std::vector<processo_guardado>::iterator prox = it; it != processos_a_lancar.end(); it = prox) {
			prox++;
			while (prox != processos_a_lancar.end() && prox->memoria < mem_disp) {
				mem_disp -= prox->memoria;
				processos.push(processo_na_fila(*prox, tempo));
				processos_a_lancar.erase(prox); /* erase nao invalida os anteriores*/

				prox = it;
				prox++;
			}
		}
	}

public:
	enum class status {
		/** Nada de especial, um processo foi executado normalmente e não há nada a ser feito */
		wait,
		/** Um processo encerrou sua execução - imprimí-lo*/
		process_terminated,
		/** Um processo ira sair dessa fila e entrará em outra*/
		change_priority,
		/** Não há processos na fila - dar oportunidade a próxima fila*/
		nill
	};
	fila_de_processos(int prioridade, bool degradar) : prioridade(prioridade), degradar(degradar) {};

	status simular1slice(int &mem_disp, int tempo, processo_na_fila &write_process) {
		lancar_processos_possiveis(mem_disp, tempo);
		if (processos_a_lancar.empty()) {
			return status::nill;
		}
		write_process = processos.front();
		switch (write_process++) {
			case processo_na_fila::status::change_priority:
				if (degradar)
					return status::change_priority;
				else /* fila 0 nao muda e isso é tratado aqui*/
					return status::wait;
			case processo_na_fila::status::terminate:
				return status::process_terminated;
			case processo_na_fila::status::wait:
				return status::wait;
		}
	};

	void push_processo(processo_guardado processo) {
		processos_a_lancar.push_back(processo);
	};
	void queue_processo(processo_na_fila processo) {
		processos.push(processo);
	}
	bool vazia() {
		return processos.empty() && processos_a_lancar.empty();
	};
};


class escalonador {
private:
	int cpus;
	int mem;
	std::vector<fila_de_processos> filas;
	std::vector<processo_guardado> processos_a_receber;
	std::vector<processo_na_fila> processos_a_recolocar;
	std::vector<processo_na_fila> processos_a_finalizar;
	std::istream &istrm;
	

	const static int NUMERO_DE_FILAS = 5;
public:
	escalonador(int cpus, int mem, std::istream &istrm) : cpus(cpus), mem(mem), istrm(istrm) {
		for(int i = 0; i < NUMERO_DE_FILAS; i++) {
			filas.push_back(fila_de_processos(i, i != 0));
		}
	};
	void run(std::ostream &ostrm) {
		while (this->istrm) {
			processo_guardado proc;
			this->istrm >> proc;
			if (this->mem < proc.memoria) {
				std::cerr << "Erro: ha um processo que requer mais memoria que o possivel\n";
			}
			this->processos_a_receber.push_back(proc);
		}
		std::make_heap(this->processos_a_receber.rbegin(), this->processos_a_receber.rend());
		
		bool feito = false;
		for(int tempo = 0; !feito; tempo++) {
		
			/* Receber processos(O tempo de chegada do processo foi atingido), parar quando:
				Os processos ainda não chegaram
				Não há mais processos a serem recebidos
			*/
			while(!this->processos_a_receber.empty() && this->processos_a_receber.front().chegada <= tempo) {
				processo_guardado proc = this->processos_a_receber.front();
				!this->processos_a_receber.empty() && proc.chegada <= tempo;
				std::pop_heap(this->processos_a_receber.begin(), this->processos_a_receber.end());
				this->filas[proc.prioridade_inicial].push_processo(proc);
				this->processos_a_receber.pop_back();
			}
			/*
				Executar os processos
			*/
			for (int _ = 0; _ < cpus; cpus++) {
				feito = true;
				processo_na_fila ref;
				for(int i = 0; feito && i < NUMERO_DE_FILAS; i++) {
					fila_de_processos::status st = filas[i].simular1slice(tempo, mem, ref);
					feito = feito && st == fila_de_processos::status::nill;
					switch (st){
						case fila_de_processos::status::process_terminated:
							processos_a_finalizar.push_back(ref);
							break;
						case fila_de_processos::status::change_priority:
							processos_a_recolocar.push_back(ref);
							break;
						case fila_de_processos::status::wait:
						case fila_de_processos::status::nill:
						default:
							break;
					}
				}
			}


			std::vector<fila_de_processos> &filas = this->filas;
			/*
				Realizar as finalizacoes e relocacoes adequadas
			*/
			std::for_each(processos_a_recolocar.begin(), processos_a_recolocar.end(), [&filas] (processo_na_fila &ref) {
				/* processos com prioridade inicial minima nao mudam*/
				if (ref.prioridade_inicial == NUMERO_DE_FILAS - 1) {}
				else if (ref.prioridade_atual == ref.prioridade_inicial) {
					ref.prioridade_atual++;
					ref.ascending = false;
				}
				else if (ref.prioridade_atual == NUMERO_DE_FILAS - 1) {
					ref.prioridade_atual--;
					ref.ascending = true;
				}
				else if (ref.ascending) {
					ref.prioridade_atual--;
				}
				else {
					ref.prioridade_atual++;
				}
				filas[ref.prioridade_atual].queue_processo(ref);
			} );

		}
	};
};

std::ostream &operator<<(std::ostream &strm, escalonador &esc) {
	esc.run(strm);
	return strm;
}


/**
	1º o nome do programa
	2º nº de cpus
	3º quantidade de memoria
	4º nome do arquivo de entrada  
	
	
*/
int main(int argc, char **argv) {
	if (argc != 4) {
		std::cerr << "Erro: argumentos devem ser: numero de cpus, quantidade de memoria, nome do arquivo de entrada(nessa ordem)\n";
		std::terminate();
	}
	std::ifstream file(argv[3]);
	escalonador esc((int)std::atoi(argv[1]), std::atoi(argv[2]), file);
	
	std::cout << esc << "\n";
	
	
    return 0;
}

