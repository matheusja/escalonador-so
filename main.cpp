
#include <cstddef> /* std::size_t*/
#include <cstdlib> /* std::atoi */

#include <algorithm> /* std::sort */
#include <exception> /* std::terminate */
#include <fstream> /* std::ifstream */
#include <iostream> /* std::c{err, out}*/
#include <queue>  /* std::queue */
#include <utility> /* std::as_const*/

struct processo_guardado {
    int chegada;
    int prioridade_inicial;
    long int duracao;
    long int memoria;
};


static bool comparador_maior_que(processo_guardado &left, processo_guardado &right) { /* std::sort requer isso */
	return left.chegada > right.chegada;
}

static std::istream &operator>>(std::istream &strm, processo_guardado &reg_processo) {
	char virg;
	strm >> reg_processo.chegada >> virg >> reg_processo.duracao >> virg >> reg_processo.memoria >> virg >> reg_processo.prioridade_inicial;
	if (reg_processo.memoria % 64 != 0) {
        std::cerr << "Erro: foi encontrado um valor estranho na memoria, esperava um multiplo de 64, encontrado " << reg_processo.memoria << "\n";
	}
	return strm;
}

struct processo_na_fila {
    const static int TIME_UNTIL_PROMOTION = 10;
    enum class status {
		/** O processo encerrou a execução, retire-o da fila*/
        terminate,
        /** O processo ficou o quantum determinado na CPU, mas ainda não encerrou a execução, voltar para a fila*/
        wait
    };
    long int duracao;
    long int tempo_restante;
    long int memoria;
    int prioridade_inicial;
    int prioridade_atual;
    int time_in_queue;
    int lancamento;
    int chegada;
    bool ascending;
    processo_na_fila(){}
    processo_na_fila(const processo_guardado &ref, int time) : duracao(ref.duracao), tempo_restante(ref.duracao), memoria(ref.memoria),
    		prioridade_inicial(ref.prioridade_inicial), prioridade_atual(ref.prioridade_inicial), time_in_queue(0),
    		lancamento(time), chegada(ref.chegada), ascending(false) { }
    processo_na_fila(const processo_na_fila &ref) {
        *this = ref;
    }
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
        return status::wait;
    }
};

static std::ostream &operator<<(std::ostream &strm, processo_na_fila &processo) {
	return strm << processo.chegada << "," << processo.lancamento << "," << processo.duracao;
}






class fila_de_processos {
private:
	std::vector<processo_guardado> processos_a_lancar;
	std::queue<processo_na_fila> processos;

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
		/** Não há processos na fila - dar oportunidade a próxima fila*/
		nill
	};
	const bool degradar; /* Se é possível que a prioridade caia */


	fila_de_processos(bool degradar) : degradar(degradar) {}

	status simular1slice(int &mem_disp, int tempo, processo_na_fila &write_process) {
		lancar_processos_possiveis(mem_disp, tempo);
		if (processos_a_lancar.empty()) {
			return status::nill;
		}
		write_process = processos.front();
		switch (write_process++) {
			case processo_na_fila::status::terminate:
				return status::process_terminated;
			case processo_na_fila::status::wait:
				return status::wait;
		}
		std::cerr << "Erro em " << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << " : temos um erro no switch logo atras!\n";
		std::terminate();
	}

	void push_processo(processo_guardado processo) {
		processos_a_lancar.push_back(processo);
	}
	void queue_processo(processo_na_fila processo) {
		processos.push(processo);
	}
	bool vazia() {
		return processos.empty() && processos_a_lancar.empty();
	}
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
			filas.push_back(fila_de_processos(i != 0));
		}
	}
	void run(std::ostream &ostrm) {
		while (this->istrm) {
			processo_guardado proc;
			this->istrm >> proc;
			if (this->mem < proc.memoria) {
				std::cerr << "Erro: ha um processo que requer mais memoria que o possivel\n";
				std::terminate();
			}
			this->processos_a_receber.push_back(proc);
		}
        /*
            std::sort((range), comparador
            se
            comparador(a1, a2) = true, entao a1 vem antes de a2
            Logo como eu faço a1 > a2, então isso fica em ordem decrescente
        */

		std::sort(this->processos_a_receber.begin(), this->processos_a_receber.end(), comparador_maior_que);

		bool feito = false;
		for(int tempo = 0; !feito; tempo++) {

			/* Receber processos(O tempo de chegada do processo foi atingido), parar quando:
				Os processos ainda não chegaram
				Não há mais processos a serem recebidos
			*/
			while(!this->processos_a_receber.empty() && this->processos_a_receber.back().chegada <= tempo) {
				processo_guardado proc = this->processos_a_receber.back();
				this->filas[std::vector<fila_de_processos>::size_type(proc.prioridade_inicial)].push_processo(proc);
				this->processos_a_receber.pop_back();
			}
			/*
				Executar os processos
			*/
			for (int _ = 0; _ < cpus; cpus++) {
				feito = true;
				processo_na_fila ref;
				for(std::vector<fila_de_processos>::size_type i = 0; feito && i < NUMERO_DE_FILAS; i++) {
					fila_de_processos::status st = filas[i].simular1slice(tempo, mem, ref);
					feito = feito && st == fila_de_processos::status::nill;
					switch (st){
						case fila_de_processos::status::process_terminated:
							processos_a_finalizar.push_back(ref);
							break;
						case fila_de_processos::status::wait:
                            processos_a_recolocar.push_back(ref);
							break;
						case fila_de_processos::status::nill:
							break;
					}
				}
			}


			/*
				Recolocar os processos nas filas adequadas
				(eles sao retirados pois eu nao quero que 2 cpus executem o mesmo processo em um mesmo slice)
			*/
			std::for_each(processos_a_recolocar.begin(), processos_a_recolocar.end(), [&filas = this->filas] (processo_na_fila &process) {
				/* processos com prioridade inicial minima nao mudam*/
				if (process.prioridade_inicial == NUMERO_DE_FILAS - 1 ||
                    /* processos que executam com prioridade maxima(degradar == false) nao mudam*/
                    !filas[std::vector<fila_de_processos>::size_type(process.prioridade_atual)].degradar ||
                    /* processos nao executaram */
                    process.time_in_queue != processo_na_fila::TIME_UNTIL_PROMOTION
				) {
                    /*Nao alterar prioridade*/
				}
				else if (process.prioridade_atual == process.prioridade_inicial) {
					process.prioridade_atual++;
					process.ascending = false;
				}
				else if (process.prioridade_atual == NUMERO_DE_FILAS - 1) {
					process.prioridade_atual--;
					process.ascending = true;
				}
				else if (process.ascending) {
					process.prioridade_atual--;
				}
				else {
					process.prioridade_atual++;
				}
				filas[std::vector<fila_de_processos>::size_type(process.prioridade_atual)].queue_processo(process);
			} );
            std::for_each(processos_a_finalizar.begin(), processos_a_finalizar.end(), [&ostrm, &tempo = const_cast<const int &>(tempo)] (processo_na_fila &processo) {
                processo.duracao = tempo - processo.duracao;
                ostrm << processo;
            });
		}
	}
};

static std::ostream &operator<<(std::ostream &strm, escalonador &esc) {
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

	std::cout << esc;


    return 0;
}

