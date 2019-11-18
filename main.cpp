
#include <cstddef> /* std::size_t*/
#include <cstdlib> /* std::atoi */

#include <algorithm> /* std::push_heap */
#include <exception> /* std::terminate */
#include <fstream> /* std::ifstream */
#include <iostream> /* std::c{err, out}*/
#include <queue>  /* std::queue */

struct processo_na_fila {
    const static int TIME_UNTIL_PROMOTION = 10;
    enum class status {
        change_priority, terminate, wait
    };
    int prioridade_inicial;
    long int tempo_restante;
    long int memoria;
    int time_in_queue;
    int lancamento;
    bool ascending;
    status operator++() {
        this->tempo_restante--;
        if (this->tempo_restante == 0) {
            return status::terminate;
        }
        time_in_queue++;
        if (times_in_queue == TIME_UNTIL_PROMOTION) {
            return status::change_priority;
        }
        return status::wait;
    };
};

struct processo_guardado {
    int chegada;
    long int duracao;
    long int memoria;
    int prioridade_inicial;
   
};

bool operator<(processo_guardado &left, processo_guardado &right) { /* std::push_heap requer isso */
	return left.chegada > right.chegada; /* Trocado pois std::heap usa o maior elemento*/
}




std::istream &operator>>(std::istream &strm, processo_guardado &reg_processo) {
	char virg;
	strm >> reg_processo.chegada >> virg >> reg_processo.duracao >> virg >> reg_processo.memoria >> virg >> reg_processo.prioridade_inicial;
	return strm;
}



class fila_de_processos {
private:
	std::queue<processo_na_fila> processos;
	int prioridade;
	bool degradar; /* Se é possível que a prioridade caia */
public:
	fila_de_processos(int prioridade, bool degradar) : prioridade(prioridade), degradar(degradar) {};
	processo_na_fila next_processo() {
		processo_na_fila processo = processo_na_fila.front();
		processo_na_fila.pop()
		return processo;
	}
	void push_processo(processo_na_fila processo) {
		processos.push(processo);
	}
	bool vazia() {
		return processos.empty();
	}
}


class escalonador {
private:
	int cpus;
	long int memoria;
	std::vector<fila_de_processos> filas;
	std::vector<processo_recebido> processos_esperando_lancamento;
	std::vector<processo_guardado> processos_futuros;
	std::istream &istrm;
	
	bool done() {
		for(fila_de_processos fila : filas) {
			if (!fila.vazia()) {
				return false;
			}
		}
		return true;
	}
public:
	escalonador(int cpus, long int memoria, std::istream &istrm) : cpus(cpus), memoria(memoria), istrm(istrm) {
		for(int i = 0; i < 5; i++) {
			filas.push_back(i, i != 0);
		}
	};
	escalonador() : escalonador(1, 8001);
	void run(std::ofstream &ostrm) {
		while (this->istrm) {
			processo_guardado proc;
			this->istrm >> proc;
			this->processos_futuros.push_back(proc);
			std::push_heap(this->processos_futuros.begin(), this->processos_futuros.end());
		}
		
		for(int tempo = 0; this->done(); tempo++) {
			/* pegar processo da heap */
			for (processo_guardado proc = this->processos_futuros.front(); proc.chegada <= tempo; proc = this->processos_futuros.front()) {
				this->processos_esperando_lancamento.push_processo(proc);
				std::pop_heap(this->processos_esperando_lancamento.begin(), this->processos_esperando_lancamento.end());
				this->processos_esperando_lancamento.pop_back();
			};
				
			/* execucao */
			
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

